"""
Simulasi NILM Tier-1 (KNN Langsung) — sesuai README.md
=======================================================
3 perangkat rumah tangga:
  1. Kipas angin        (motor induksi kecil, Q signifikan, inrush sedang)
  2. Pompa air          (motor induksi besar, inrush 5-7x, Q besar)
  3. Lampu LED 50 W     (driver murah: H3 ~9%, H5 ~21%, inrush kecil)

Pipeline (identik dengan desain di README):
  sinyal v(t), i(t) @ 860 SPS (ADS1115)  -->  ekstraksi fitur
  Phi = [P, Q, Inrush_ratio, H3, H5]     -->  Goertzel @ 50/150/250 Hz
  onboarding 5-shot per perangkat        -->  support set di "Flash"
  inferensi: KNN (K=3, Euclidean, majority vote)

Hanya butuh numpy. Jalankan:  python3 nilm_knn_simulation.py
"""

import numpy as np

# ---------------------------------------------------------------- konstanta
FS = 860.0          # sample rate ADS1115 (SPS)
F0 = 50.0           # frekuensi fundamental grid Indonesia (Hz)
VRMS = 220.0        # tegangan nominal
N_GOERTZEL = 430    # jendela analisis 0.5 s = 25 siklus penuh
                    # k = f*N/FS -> 50 Hz: k=25, 150 Hz: k=75, 250 Hz: k=125
                    # semua bilangan bulat -> tidak ada spectral leakage
K_SHOT = 5          # jumlah contoh onboarding per perangkat (few-shot, K=5)
K_NN = 3            # tetangga terdekat untuk majority vote
RNG = np.random.default_rng(42)


# ============================================================ 1. GOERTZEL
def goertzel(x, f_target, fs):
    """Algoritma Goertzel standar (2nd-order IIR resonator).

    Mengembalikan (magnitudo amplitudo, fase) komponen frekuensi f_target.
    Jauh lebih murah dari FFT penuh bila hanya butuh beberapa bin --
    inilah alasan ia dipakai di ESP32 (README: ekstraksi H3/H5).
    """
    n = len(x)
    k = f_target * n / fs                 # bin (dibuat bulat oleh pilihan N)
    w = 2.0 * np.pi * k / n
    cw, sw = np.cos(w), np.sin(w)
    coeff = 2.0 * cw

    s_prev = s_prev2 = 0.0
    for sample in x:                      # loop per-sampel, persis seperti di MCU
        s = sample + coeff * s_prev - s_prev2
        s_prev2, s_prev = s_prev, s

    # konversi state akhir ke komponen real/imajiner
    real = s_prev - s_prev2 * cw
    imag = s_prev2 * sw
    mag = 2.0 / n * np.hypot(real, imag)  # skala -> amplitudo puncak sinusoid
    phase = np.arctan2(imag, real)
    return mag, phase


# ==================================================== 2. MODEL PERANGKAT
# Parameter "fisika" tiap perangkat: daya, power factor, profil harmonik
# (rasio terhadap fundamental), rasio inrush, dan konstanta waktu transien.
APPLIANCES = {
    "Kipas Angin": dict(
        P=60.0, pf=0.72, lagging=True,     # motor induksi: Q signifikan
        h3=0.045, h5=0.020,                # hampir linear, harmonik kecil
        inrush=2.5, tau=0.12,              # soft start ~2-3x
    ),
    "Pompa Air": dict(
        P=350.0, pf=0.68, lagging=True,    # motor induksi besar
        h3=0.055, h5=0.025,
        inrush=6.0, tau=0.20,              # ciri khas: inrush 5-7x (README)
    ),
    "Lampu LED 50W": dict(
        P=50.0, pf=0.90, lagging=False,    # driver switching, sedikit kapasitif
        h3=0.09, h5=0.21,                  # profil README: H3 9%, H5 21%
        inrush=1.3, tau=0.01,              # praktis tanpa transien motor
    ),
}


def synth_event(spec, rng):
    """Bangkitkan pasangan sinyal v(t), i(t) satu event penyalaan @ 860 SPS.

    Variasi antar-event (jitter parameter + noise ADC) meniru kondisi nyata:
    fluktuasi tegangan PLN, suhu motor, toleransi komponen.
    """
    dur = 1.2                                   # 1.2 s: transien + steady state
    t = np.arange(int(dur * FS)) / FS

    # jitter kondisi operasional per event
    P   = spec["P"] * rng.normal(1.0, 0.04)
    pf  = np.clip(spec["pf"] * rng.normal(1.0, 0.02), 0.05, 0.999)
    h3  = spec["h3"] * rng.normal(1.0, 0.10)
    h5  = spec["h5"] * rng.normal(1.0, 0.10)
    irr = spec["inrush"] * rng.normal(1.0, 0.12)
    vamp = np.sqrt(2) * VRMS * rng.normal(1.0, 0.01)   # tegangan PLN +-1%

    phi = np.arccos(pf) * (1 if spec["lagging"] else -1)  # sudut arus vs tegangan
    i1 = np.sqrt(2) * P / (VRMS * pf)                     # amplitudo puncak I1

    w = 2 * np.pi * F0
    v = vamp * np.sin(w * t)
    i_steady = i1 * (np.sin(w * t - phi)
                     + h3 * np.sin(3 * w * t - 3 * phi + 0.5)
                     + h5 * np.sin(5 * w * t - 5 * phi + 1.0))

    # transien inrush: selubung eksponensial menuju steady state
    envelope = 1.0 + (irr - 1.0) * np.exp(-t / spec["tau"])
    i = i_steady * envelope

    # noise pengukuran (sensor Hall + ADC 16-bit) ~0.8% amplitudo steady
    i += rng.normal(0, 0.008 * i1, len(t))
    v += rng.normal(0, 0.002 * vamp, len(t))
    return v, i


# ================================================= 3. EKSTRAKSI FITUR Phi
def extract_features(v, i):
    """Phi = [P, Q, Inrush_ratio, H3, H5] -- semua via Goertzel + aritmetika dasar."""
    # jendela steady-state: N_GOERTZEL sampel terakhir (25 siklus penuh)
    vs, is_ = v[-N_GOERTZEL:], i[-N_GOERTZEL:]

    v1, phv = goertzel(vs, F0, FS)          # fundamental tegangan
    i1, phi1 = goertzel(is_, F0, FS)        # fundamental arus
    i3, _ = goertzel(is_, 3 * F0, FS)       # H3 @ 150 Hz
    i5, _ = goertzel(is_, 5 * F0, FS)       # H5 @ 250 Hz

    # daya dari fasor fundamental (amplitudo puncak -> /2)
    dphi = phv - phi1
    P = 0.5 * v1 * i1 * np.cos(dphi)
    Q = 0.5 * v1 * i1 * np.sin(dphi)

    # inrush: puncak arus 0.3 s pertama vs puncak steady-state
    peak_transient = np.max(np.abs(i[: int(0.3 * FS)]))
    peak_steady = np.sqrt(2) * np.sqrt(np.mean(is_**2))
    inrush_ratio = peak_transient / peak_steady

    return np.array([P, Q, inrush_ratio, 100 * i3 / i1, 100 * i5 / i1])


FEAT_NAMES = ["P (W)", "Q (var)", "Inrush", "H3 (%)", "H5 (%)"]


# ============================================ 4. KNN TIER-1 (README, K=3)
class KNNTier1:
    """Support set langsung di 'Flash', tanpa model ML -- ukuran model 0 KB."""

    def fit(self, X, y):
        # z-score agar P (ratusan watt) tidak menenggelamkan H3/H5 (persen)
        self.mu, self.sd = X.mean(axis=0), X.std(axis=0) + 1e-9
        self.X = (X - self.mu) / self.sd
        self.y = np.asarray(y)

    def predict(self, x):
        z = (x - self.mu) / self.sd
        d = np.linalg.norm(self.X - z, axis=1)          # jarak Euclidean
        nn = np.argsort(d)[:K_NN]                       # 3 tetangga terdekat
        labels, counts = np.unique(self.y[nn], return_counts=True)
        return labels[np.argmax(counts)], d[nn], self.y[nn]


# ======================================================== 5. EKSPERIMEN
def main():
    names = list(APPLIANCES)

    # --- validasi Goertzel: sinus murni 50 Hz amplitudo 1.0 harus terukur 1.0
    tt = np.arange(N_GOERTZEL) / FS
    mag, _ = goertzel(np.sin(2 * np.pi * F0 * tt), F0, FS)
    print(f"[cek] Goertzel sinus murni 50 Hz, amplitudo 1.0 -> terukur {mag:.4f}\n")

    # --- fase onboarding: 5 shot per perangkat -> support set
    Xs, ys = [], []
    print("=" * 74)
    print(f"FASE ONBOARDING  (few-shot, K={K_SHOT} penyalaan per perangkat)")
    print("=" * 74)
    for name in names:
        print(f"\n  {name}")
        print("    shot |" + "|".join(f"{f:>9}" for f in FEAT_NAMES))
        for k in range(K_SHOT):
            v, i = synth_event(APPLIANCES[name], RNG)
            phi = extract_features(v, i)
            Xs.append(phi)
            ys.append(name)
            print(f"      #{k+1} |" + "|".join(f"{x:9.2f}" for x in phi))

    knn = KNNTier1()
    knn.fit(np.array(Xs), ys)

    # --- ringkasan fingerprint rata-rata
    print("\n" + "=" * 74)
    print("ELECTRICAL FINGERPRINT rata-rata (support set)")
    print("=" * 74)
    print(f"{'Perangkat':<16}|" + "|".join(f"{f:>9}" for f in FEAT_NAMES))
    Xa, ya = np.array(Xs), np.array(ys)
    for name in names:
        m = Xa[ya == name].mean(axis=0)
        print(f"{name:<16}|" + "|".join(f"{x:9.2f}" for x in m))

    # --- fase inferensi: 30 event baru per perangkat
    n_test = 30
    print("\n" + "=" * 74)
    print(f"FASE INFERENSI  ({n_test} event baru per perangkat, KNN K={K_NN})")
    print("=" * 74)
    conf = np.zeros((len(names), len(names)), dtype=int)
    shown = 0
    for a, name in enumerate(names):
        for _ in range(n_test):
            v, i = synth_event(APPLIANCES[name], RNG)
            phi = extract_features(v, i)
            pred, dists, nn_labels = knn.predict(phi)
            conf[a, names.index(pred)] += 1
            if shown < 6:  # tampilkan beberapa contoh keputusan KNN
                ok = "BENAR" if pred == name else "SALAH"
                print(f"  event {name:<14} -> prediksi {pred:<14} [{ok}]  "
                      f"3-NN: {', '.join(l.split()[0] for l in nn_labels)}  "
                      f"d={np.round(dists, 2)}")
                shown += 1

    # --- confusion matrix + akurasi
    print("\n  Confusion Matrix (baris = aktual, kolom = prediksi)")
    header = " " * 18 + "".join(f"{n.split()[0]:>12}" for n in names)
    print(header)
    for a, name in enumerate(names):
        print(f"  {name:<16}" + "".join(f"{c:>12}" for c in conf[a]))

    acc = np.trace(conf) / conf.sum() * 100
    print(f"\n  AKURASI KESELURUHAN : {acc:.1f}%  "
          f"({np.trace(conf)}/{conf.sum()} event dikenali benar)")

    # --- separabilitas antar kelas di ruang fitur ter-normalisasi
    print("\n" + "=" * 74)
    print("SEPARABILITAS (jarak Euclidean antar-centroid, ruang z-score)")
    print("=" * 74)
    Z = (Xa - knn.mu) / knn.sd
    cents = {n: Z[ya == n].mean(axis=0) for n in names}
    for a in range(len(names)):
        for b in range(a + 1, len(names)):
            d = np.linalg.norm(cents[names[a]] - cents[names[b]])
            print(f"  {names[a]:<16} <-> {names[b]:<16}: {d:.2f}")
    intra = np.mean([np.linalg.norm(Z[ya == n] - cents[n], axis=1).mean()
                     for n in names])
    print(f"  sebaran intra-kelas rata-rata           : {intra:.2f}")


if __name__ == "__main__":
    main()
