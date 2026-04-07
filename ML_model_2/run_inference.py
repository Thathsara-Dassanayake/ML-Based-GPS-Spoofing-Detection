import os
import json
import pickle
import argparse
from pathlib import Path

import numpy as np
import pywt
import torch
import torch.nn as nn
import torch.nn.functional as F


# ============================================================
# Config from training code
# ============================================================
SEGMENT_LEN = 512
N_SCALES = 64
EMBED_DIM = 256


# ============================================================
# Model definitions (must match training exactly)
# ============================================================
class OnlineEncoder(nn.Module):
    def __init__(self, d=EMBED_DIM):
        super().__init__()
        self.net = nn.Sequential(
            nn.Conv2d(1, 32, 3, stride=2, padding=1),
            nn.BatchNorm2d(32), nn.GELU(),
            nn.Conv2d(32, 64, 3, stride=2, padding=1),
            nn.BatchNorm2d(64), nn.GELU(),
            nn.Conv2d(64, 128, 3, stride=2, padding=1),
            nn.BatchNorm2d(128), nn.GELU(),
            nn.Conv2d(128, 256, 3, stride=2, padding=1),
            nn.BatchNorm2d(256), nn.GELU(),
        )
        self.pool = nn.AdaptiveAvgPool2d((4, 4))
        self.proj = nn.Sequential(
            nn.Flatten(),
            nn.Linear(256 * 16, 512), nn.GELU(),
            nn.Linear(512, d)
        )

    def forward(self, x):
        return self.proj(self.pool(self.net(x)))


class TargetEncoder(nn.Module):
    def __init__(self, online):
        super().__init__()
        self.encoder = online
        for p in self.encoder.parameters():
            p.requires_grad = False

    def forward(self, x):
        with torch.no_grad():
            return self.encoder(x)


class Predictor(nn.Module):
    def __init__(self, d=EMBED_DIM, md=128):
        super().__init__()
        self.mask_enc = nn.Sequential(
            nn.Linear(md, 128), nn.GELU(), nn.Linear(128, d)
        )
        self.net = nn.Sequential(
            nn.Linear(d * 2, 512), nn.GELU(),
            nn.Linear(512, 512), nn.GELU(),
            nn.Linear(512, d)
        )

    def forward(self, q, z):
        return self.net(torch.cat([q, self.mask_enc(z)], dim=1))


class NEMESIS(nn.Module):
    def __init__(self, d=EMBED_DIM, tau=0.996):
        super().__init__()
        self.online = OnlineEncoder(d)
        self.target = TargetEncoder(self.online)
        self.pred = Predictor(d)
        self.tau = tau


class MLPProbe(nn.Module):
    def __init__(self, d=EMBED_DIM):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(d, 256), nn.LayerNorm(256),
            nn.GELU(), nn.Dropout(0.3),
            nn.Linear(256, 128), nn.LayerNorm(128),
            nn.GELU(), nn.Dropout(0.2),
            nn.Linear(128, 2)
        )

    def forward(self, x):
        return self.net(x)


# ============================================================
# Data processing
# ============================================================
def load_iq_file(filepath, max_samples=None):
    raw = np.fromfile(filepath, dtype=np.int16)

    if raw.size == 0:
        raise ValueError(f"File is empty or unreadable: {filepath}")

    # remove leading zeros like training code
    nz = np.nonzero(raw)[0]
    if len(nz) and nz[0] > 0:
        skip = nz[0] if nz[0] % 2 == 0 else nz[0] + 1
        raw = raw[skip:]

    # ensure even number of int16 values
    if len(raw) % 2:
        raw = raw[:-1]

    if len(raw) < 2:
        raise ValueError("Not enough IQ data after trimming.")

    I = raw[0::2].astype(np.float32)
    Q = raw[1::2].astype(np.float32)

    if max_samples is not None:
        I = I[:max_samples]
        Q = Q[:max_samples]

    scale = np.sqrt(np.mean(I**2 + Q**2)) + 1e-8
    sig = ((I / scale) + 1j * (Q / scale)).astype(np.complex64)

    return sig


def compute_cwt_scalogram(sig, n_scales=N_SCALES):
    mag = np.abs(sig).astype(np.float64)
    coeffs, _ = pywt.cwt(mag, np.arange(1, n_scales + 1), 'morl')
    s = np.log1p(np.abs(coeffs)).astype(np.float32)
    mn, mx = s.min(), s.max()
    if mx - mn > 1e-8:
        s = (s - mn) / (mx - mn)
    return s


def signal_to_scalograms(signal, segment_len=SEGMENT_LEN, stride=None, max_segments=200):
    if stride is None:
        stride = segment_len // 2

    if len(signal) < segment_len:
        raise ValueError(
            f"Signal too short. Need at least {segment_len} complex samples, got {len(signal)}"
        )

    starts = list(range(0, len(signal) - segment_len + 1, stride))[:max_segments]

    scalograms = []
    for s in starts:
        seg = signal[s:s + segment_len]
        scalograms.append(compute_cwt_scalogram(seg))

    arr = np.array(scalograms, dtype=np.float32)  # (N, 64, 512)
    arr = np.expand_dims(arr, axis=1)             # (N, 1, 64, 512)
    return arr


# ============================================================
# Model loading
# ============================================================
def load_models(model_dir, device):
    model_dir = Path(model_dir)

    encoder_path = model_dir / "nemesis_encoder.pt"
    probe_path = model_dir / "nemesis_mlp_probe.pt"
    scaler_path = model_dir / "nemesis_mlp_scaler.pkl"

    if not encoder_path.exists():
        raise FileNotFoundError(f"Missing: {encoder_path}")
    if not probe_path.exists():
        raise FileNotFoundError(f"Missing: {probe_path}")
    if not scaler_path.exists():
        raise FileNotFoundError(f"Missing: {scaler_path}")

    encoder_ckpt = torch.load(encoder_path, map_location=device, weights_only=False)
    probe_ckpt = torch.load(probe_path, map_location=device, weights_only=False)

    model = NEMESIS(d=EMBED_DIM).to(device)
    model.load_state_dict(encoder_ckpt["model_state_dict"], strict=False)
    model.eval()

    probe = MLPProbe(d=EMBED_DIM).to(device)
    probe.load_state_dict(probe_ckpt["model_state_dict"])
    probe.eval()

    with open(scaler_path, "rb") as f:
        scaler = pickle.load(f)

    threshold_2sigma = encoder_ckpt.get("threshold_2sigma")
    threshold_3sigma = encoder_ckpt.get("threshold_3sigma")
    config = encoder_ckpt.get("config", {})

    return model, probe, scaler, threshold_2sigma, threshold_3sigma, config


def extract_embeddings(model, x_tensor, device, batch_size=64):
    model.eval()
    out = []
    with torch.no_grad():
        for i in range(0, len(x_tensor), batch_size):
            batch = x_tensor[i:i + batch_size].to(device)
            emb = model.online(batch).cpu().numpy()
            out.append(emb)
    return np.concatenate(out, axis=0)


# ============================================================
# Inference
# ============================================================
def predict_file(iq_path, model_dir, max_samples=None, max_segments=200):
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    print(f"[INFO] Using device: {device}")
    print(f"[INFO] Loading models from: {model_dir}")

    model, probe, scaler, thr2, thr3, config = load_models(model_dir, device)

    print("[INFO] Model config from checkpoint:")
    print(json.dumps(config, indent=2))

    signal = load_iq_file(iq_path, max_samples=max_samples)
    print(f"[INFO] Loaded complex samples: {len(signal):,}")

    scalograms = signal_to_scalograms(
        signal,
        segment_len=SEGMENT_LEN,
        stride=SEGMENT_LEN // 2,
        max_segments=max_segments
    )
    print(f"[INFO] Scalograms shape: {scalograms.shape}")

    x_tensor = torch.tensor(scalograms, dtype=torch.float32)

    embeddings = extract_embeddings(model, x_tensor, device=device, batch_size=64)
    print(f"[INFO] Embeddings shape: {embeddings.shape}")

    x_scaled = scaler.transform(embeddings)
    x_scaled = torch.tensor(x_scaled, dtype=torch.float32).to(device)

    with torch.no_grad():
        logits = probe(x_scaled)
        probs = F.softmax(logits, dim=1).cpu().numpy()
        preds = logits.argmax(dim=1).cpu().numpy()

    spoof_probs = probs[:, 1]
    legit_probs = probs[:, 0]

    avg_spoof_prob = float(np.mean(spoof_probs))
    avg_legit_prob = float(np.mean(legit_probs))
    spoof_ratio = float(np.mean(preds == 1))
    legit_ratio = float(np.mean(preds == 0))

    final_label = "Spoofed" if avg_spoof_prob >= 0.5 else "Legitimate"

    result = {
        "file": str(iq_path),
        "device": str(device),
        "num_complex_samples": int(len(signal)),
        "num_segments_used": int(len(scalograms)),
        "segment_len": SEGMENT_LEN,
        "n_scales": N_SCALES,
        "avg_legitimate_probability": avg_legit_prob,
        "avg_spoof_probability": avg_spoof_prob,
        "segment_legitimate_ratio": legit_ratio,
        "segment_spoof_ratio": spoof_ratio,
        "final_prediction": final_label,
        "threshold_2sigma": None if thr2 is None else float(thr2),
        "threshold_3sigma": None if thr3 is None else float(thr3),
    }

    return result, spoof_probs, preds


def main():
    parser = argparse.ArgumentParser(description="Run NEMESIS inference on an IQ .bin file")
    parser.add_argument("--iq_file", required=True, help="Path to interleaved int16 IQ .bin file")
    parser.add_argument(
        "--model_dir",
        default=str(Path(__file__).resolve().parent / "Model"),
        help="Directory containing nemesis_encoder.pt, nemesis_mlp_probe.pt, nemesis_mlp_scaler.pkl"
    )
    parser.add_argument(
        "--max_samples",
        type=int,
        default=200000,
        help="Optional cap on complex samples to load"
    )
    parser.add_argument(
        "--max_segments",
        type=int,
        default=100,
        help="Maximum number of 512-sample segments to evaluate"
    )

    args = parser.parse_args()

    result, spoof_probs, preds = predict_file(
        iq_path=args.iq_file,
        model_dir=args.model_dir,
        max_samples=args.max_samples,
        max_segments=args.max_segments
    )

    print("\n" + "=" * 60)
    print("FINAL RESULT")
    print("=" * 60)
    print(json.dumps(result, indent=2))

    out_json = Path(args.iq_file).with_suffix(".inference.json")
    with open(out_json, "w", encoding="utf-8") as f:
        json.dump(result, f, indent=2)

    print(f"\n[INFO] Saved result JSON to: {out_json}")


if __name__ == "__main__":
    main()