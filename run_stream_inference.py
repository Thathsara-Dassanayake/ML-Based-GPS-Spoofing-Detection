import os
import json
import time
import pickle
import argparse
from pathlib import Path
from collections import deque

import numpy as np
import pywt
import torch
import torch.nn as nn
import torch.nn.functional as F


# ============================================================
# Config
# ============================================================
SEGMENT_LEN = 512
N_SCALES = 64
EMBED_DIM = 256


# ============================================================
# Model definitions
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
# Signal utils
# ============================================================
def int16_iq_to_complex(raw_int16):
    if raw_int16.size == 0:
        raise ValueError("Received empty int16 buffer.")

    # trim leading zeros once per chunk if needed
    nz = np.nonzero(raw_int16)[0]
    if len(nz) and nz[0] > 0:
        skip = nz[0] if nz[0] % 2 == 0 else nz[0] + 1
        raw_int16 = raw_int16[skip:]

    if len(raw_int16) % 2 != 0:
        raw_int16 = raw_int16[:-1]

    if len(raw_int16) < 2:
        return np.array([], dtype=np.complex64)

    I = raw_int16[0::2].astype(np.float32)
    Q = raw_int16[1::2].astype(np.float32)

    scale = np.sqrt(np.mean(I**2 + Q**2)) + 1e-8
    sig = ((I / scale) + 1j * (Q / scale)).astype(np.complex64)
    return sig


def complex_to_interleaved_int16(sig_complex):
    """
    Convert normalized complex64 signal back to int16 IQ for saving live_sample.bin.
    This is mainly for GUI/debug/live file visibility.
    """
    I = np.real(sig_complex)
    Q = np.imag(sig_complex)

    max_abs = max(np.max(np.abs(I)), np.max(np.abs(Q)), 1e-8)
    scale = 30000.0 / max_abs

    I16 = np.clip(I * scale, -32768, 32767).astype(np.int16)
    Q16 = np.clip(Q * scale, -32768, 32767).astype(np.int16)

    interleaved = np.empty(I16.size + Q16.size, dtype=np.int16)
    interleaved[0::2] = I16
    interleaved[1::2] = Q16
    return interleaved


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

    arr = np.array(scalograms, dtype=np.float32)   # (N, 64, 512)
    arr = np.expand_dims(arr, axis=1)              # (N, 1, 64, 512)
    return arr


# ============================================================
# Model loading
# ============================================================
def load_models(model_dir, device):
    model_dir = Path(model_dir)

    encoder_path = model_dir / "nemesis_encoder (1).pt"
    probe_path = model_dir / "nemesis_mlp_probe (1).pt"
    scaler_path = model_dir / "nemesis_mlp_scaler (1).pkl"

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
# Inference on in-memory signal
# ============================================================
def predict_signal(signal, model, probe, scaler, device, thr2=None, thr3=None, max_segments=100):
    scalograms = signal_to_scalograms(
        signal,
        segment_len=SEGMENT_LEN,
        stride=SEGMENT_LEN // 2,
        max_segments=max_segments
    )

    x_tensor = torch.tensor(scalograms, dtype=torch.float32)
    embeddings = extract_embeddings(model, x_tensor, device=device, batch_size=64)

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

    return result


# ============================================================
# JSON writers
# ============================================================
def atomic_write_json(path, data):
    path = Path(path)
    tmp_path = path.with_suffix(path.suffix + ".tmp")
    with open(tmp_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2)
    os.replace(tmp_path, path)


def append_jsonl(path, data):
    with open(path, "a", encoding="utf-8") as f:
        f.write(json.dumps(data) + "\n")


# ============================================================
# Streaming loop
# ============================================================
def stream_inference(
    source_bin,
    model_dir,
    live_bin_path,
    latest_json_path,
    history_jsonl_path,
    sample_window_complex=50000,
    read_chunk_complex=10000,
    max_segments=100,
    poll_interval=1.0,
    follow=True,
):
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"[INFO] Using device: {device}")

    model, probe, scaler, thr2, thr3, config = load_models(model_dir, device)
    print("[INFO] Loaded model config:")
    print(json.dumps(config, indent=2))

    source_bin = Path(source_bin)
    live_bin_path = Path(live_bin_path)
    latest_json_path = Path(latest_json_path)
    history_jsonl_path = Path(history_jsonl_path)

    # each complex sample = 2 int16 = 4 bytes
    bytes_per_complex = 4
    read_chunk_bytes = read_chunk_complex * bytes_per_complex

    # rolling buffer of complex64 samples
    rolling_buffer = deque(maxlen=sample_window_complex)

    last_infer_time = None
    iteration = 0

    with open(source_bin, "rb") as f:
        while True:
            chunk = f.read(read_chunk_bytes)

            if not chunk:
                if follow:
                    time.sleep(poll_interval)
                    continue
                else:
                    print("[INFO] End of source file reached.")
                    break

            raw = np.frombuffer(chunk, dtype=np.int16)
            sig_chunk = int16_iq_to_complex(raw)

            if sig_chunk.size == 0:
                continue

            rolling_buffer.extend(sig_chunk.tolist())

            if len(rolling_buffer) < SEGMENT_LEN:
                continue

            signal = np.array(rolling_buffer, dtype=np.complex64)

            # save/update small live bin file
            live_int16 = complex_to_interleaved_int16(signal)
            with open(live_bin_path, "wb") as lf:
                lf.write(live_int16.tobytes())

            # inference
            try:
                result = predict_signal(
                    signal=signal,
                    model=model,
                    probe=probe,
                    scaler=scaler,
                    device=device,
                    thr2=thr2,
                    thr3=thr3,
                    max_segments=max_segments
                )

                iteration += 1
                now_ts = time.time()

                payload = {
                    "timestamp_epoch": now_ts,
                    "timestamp_iso": time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime(now_ts)),
                    "iteration": iteration,
                    "source_file": str(source_bin),
                    "live_bin_file": str(live_bin_path),
                    "window_complex_samples": int(len(signal)),
                    "new_chunk_complex_samples": int(len(sig_chunk)),
                    "status": result["final_prediction"],
                    "result": result,
                }

                atomic_write_json(latest_json_path, payload)
                append_jsonl(history_jsonl_path, payload)

                last_infer_time = now_ts

                print(
                    f"[{payload['timestamp_iso']}] "
                    f"Iteration={iteration} | "
                    f"Window={len(signal):,} | "
                    f"Status={result['final_prediction']} | "
                    f"SpoofProb={result['avg_spoof_probability']:.4f}"
                )

            except Exception as e:
                err_payload = {
                    "timestamp_epoch": time.time(),
                    "timestamp_iso": time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime()),
                    "iteration": iteration,
                    "source_file": str(source_bin),
                    "live_bin_file": str(live_bin_path),
                    "status": "ERROR",
                    "error": str(e),
                }

                atomic_write_json(latest_json_path, err_payload)
                append_jsonl(history_jsonl_path, err_payload)
                print(f"[ERROR] {e}")


# ============================================================
# Main
# ============================================================
def main():
    parser = argparse.ArgumentParser(description="Continuous streaming inference from large IQ .bin file")
    parser.add_argument("--source_bin", required=True, help="Large source interleaved int16 IQ .bin file")
    parser.add_argument(
        "--model_dir",
        default=str(Path(__file__).resolve().parent / "Model"),
        help="Directory containing model files"
    )
    parser.add_argument(
        "--live_bin",
        default="live_sample.bin",
        help="Continuously overwritten small live sample bin"
    )
    parser.add_argument(
        "--latest_json",
        default="latest_status.json",
        help="Latest status JSON for React GUI"
    )
    parser.add_argument(
        "--history_jsonl",
        default="history.jsonl",
        help="Append-only JSONL history for GUI timeline"
    )
    parser.add_argument(
        "--sample_window_complex",
        type=int,
        default=50000,
        help="Rolling live sample size in complex samples"
    )
    parser.add_argument(
        "--read_chunk_complex",
        type=int,
        default=10000,
        help="How many new complex samples to read per loop"
    )
    parser.add_argument(
        "--max_segments",
        type=int,
        default=100,
        help="Maximum number of segments used in inference"
    )
    parser.add_argument(
        "--poll_interval",
        type=float,
        default=1.0,
        help="Polling interval when following a growing source file"
    )
    parser.add_argument(
        "--follow",
        action="store_true",
        help="Keep watching if the source file grows"
    )

    args = parser.parse_args()

    stream_inference(
        source_bin=args.source_bin,
        model_dir=args.model_dir,
        live_bin_path=args.live_bin,
        latest_json_path=args.latest_json,
        history_jsonl_path=args.history_jsonl,
        sample_window_complex=args.sample_window_complex,
        read_chunk_complex=args.read_chunk_complex,
        max_segments=args.max_segments,
        poll_interval=args.poll_interval,
        follow=args.follow,
    )


if __name__ == "__main__":
    main()