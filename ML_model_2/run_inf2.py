# ============================================================
# NEMESIS-Shield — Standalone Inference
# Depends only on:
#   nemesis_encoder.pt
#   nemesis_mlp_probe.pt        (if you have it, else encoder-only)
#   nemesis_mlp_scaler.pkl
#   nemesis_legit.bin
#   nemesis_spoofed.bin
# ============================================================

import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import (accuracy_score, roc_auc_score,
                             f1_score, classification_report,
                             ConfusionMatrixDisplay, roc_curve)
from copy import deepcopy
import pickle, os
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# ── Config (must match training) ─────────────────────────────
SEGMENT_LEN = 512
N_SCALES    = 64
EMBED_DIM   = 256
DEVICE      = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# ── File paths — edit if needed ──────────────────────────────
ENCODER_PATH = r".\ML_model_2\Model\nemesis_encoder.pt"
PROBE_PATH   = r".\ML_model_2\Model\nemesis_mlp_probe.pt"
SCALER_PATH  = r".\ML_model_2\Model\nemesis_mlp_scaler.pkl"
LEGIT_BIN    = r".\ML_model_2\test_data\nemesis_legit.bin"
SPOOF_BIN    = r".\ML_model_2\test_data\nemesis_spoofed.bin"

print(f"Device: {DEVICE}")

# ═══════════════════════════════════════════════════════════════
# MODEL DEFINITIONS  (must match training exactly)
# ═══════════════════════════════════════════════════════════════

class OnlineEncoder(nn.Module):
    def __init__(self, d=EMBED_DIM):
        super().__init__()
        self.net = nn.Sequential(
            nn.Conv2d(1,   32, 3, stride=2, padding=1),
            nn.BatchNorm2d(32),  nn.GELU(),
            nn.Conv2d(32,  64, 3, stride=2, padding=1),
            nn.BatchNorm2d(64),  nn.GELU(),
            nn.Conv2d(64, 128, 3, stride=2, padding=1),
            nn.BatchNorm2d(128), nn.GELU(),
            nn.Conv2d(128,256, 3, stride=2, padding=1),
            nn.BatchNorm2d(256), nn.GELU(),
        )
        self.pool = nn.AdaptiveAvgPool2d((4, 4))
        self.proj = nn.Sequential(
            nn.Flatten(),
            nn.Linear(256*16, 512), nn.GELU(),
            nn.Linear(512, d)
        )
    def forward(self, x):
        return self.proj(self.pool(self.net(x)))

class TargetEncoder(nn.Module):
    def __init__(self, online):
        super().__init__()
        self.encoder = deepcopy(online)
        for p in self.encoder.parameters():
            p.requires_grad = False
    def forward(self, x):
        with torch.no_grad():
            return self.encoder(x)

class Predictor(nn.Module):
    def __init__(self, d=EMBED_DIM, md=128):
        super().__init__()
        self.mask_enc = nn.Sequential(
            nn.Linear(md, 128), nn.GELU(), nn.Linear(128, d))
        self.net = nn.Sequential(
            nn.Linear(d*2, 512), nn.GELU(),
            nn.Linear(512, 512), nn.GELU(),
            nn.Linear(512, d))
    def forward(self, q, z):
        return self.net(torch.cat([q, self.mask_enc(z)], dim=1))

class NEMESIS(nn.Module):
    def __init__(self, d=EMBED_DIM):
        super().__init__()
        self.online = OnlineEncoder(d)
        self.target = TargetEncoder(self.online)
        self.pred   = Predictor(d)
    def forward(self, x):
        return self.online(x)   # inference: encoder only

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
    def forward(self, x): return self.net(x)

# ═══════════════════════════════════════════════════════════════
# LOAD MODELS
# ═══════════════════════════════════════════════════════════════

print("\nLoading encoder …")
ckpt    = torch.load(ENCODER_PATH, map_location=DEVICE, weights_only=False)
encoder = NEMESIS().to(DEVICE)
encoder.load_state_dict(ckpt['model_state_dict'])
encoder.eval()
print(f"  Threshold 2σ : {ckpt.get('threshold_2sigma', 'N/A')}")
print(f"  Threshold 3σ : {ckpt.get('threshold_3sigma', 'N/A')}")

print("Loading MLP probe …")
probe = MLPProbe().to(DEVICE)
pckpt = torch.load(PROBE_PATH, map_location=DEVICE, weights_only=False)
probe.load_state_dict(pckpt['model_state_dict'])
probe.eval()

print("Loading scaler …")
with open(SCALER_PATH, 'rb') as f:
    scaler = pickle.load(f)

print("All models loaded.\n")

# ═══════════════════════════════════════════════════════════════
# LOAD BIN FILES
# ═══════════════════════════════════════════════════════════════

print("Loading bin files …")
legit_X = np.fromfile(LEGIT_BIN, dtype=np.float32).reshape(-1, 1, N_SCALES, SEGMENT_LEN)
spoof_X = np.fromfile(SPOOF_BIN, dtype=np.float32).reshape(-1, 1, N_SCALES, SEGMENT_LEN)

n_legit = len(legit_X)
n_spoof = len(spoof_X)
print(f"  Legit  : {n_legit} samples")
print(f"  Spoofed: {n_spoof} samples")

# Balance to 50/50 for fair eval
n_each  = min(n_legit, n_spoof)
rng     = np.random.RandomState(42)
l_idx   = rng.choice(n_legit, n_each, replace=False)
s_idx   = rng.choice(n_spoof, n_each, replace=False)

X = np.concatenate([legit_X[l_idx], spoof_X[s_idx]], axis=0)
y = np.array([0]*n_each + [1]*n_each, dtype=np.int64)

# Shuffle
perm = rng.permutation(len(X))
X, y = X[perm], y[perm]

print(f"  Eval set: {len(X)} samples  (legit={n_each}  spoof={n_each})")

# ═══════════════════════════════════════════════════════════════
# INFERENCE
# ═══════════════════════════════════════════════════════════════

def extract_embeddings(encoder, X_np, bs=64):
    encoder.eval()
    out = []
    X_t = torch.FloatTensor(X_np)
    with torch.no_grad():
        for i in range(0, len(X_t), bs):
            out.append(
                encoder.online(X_t[i:i+bs].to(DEVICE)).cpu().numpy())
    return np.concatenate(out)

print("\nExtracting embeddings …")
emb = extract_embeddings(encoder, X)

print("Running MLP probe …")
probe.eval()
X_sc = torch.FloatTensor(scaler.transform(emb)).to(DEVICE)
with torch.no_grad():
    logits = probe(X_sc)
    probs  = F.softmax(logits, dim=1)[:, 1].cpu().numpy()
    preds  = logits.argmax(1).cpu().numpy()

# ═══════════════════════════════════════════════════════════════
# METRICS
# ═══════════════════════════════════════════════════════════════

acc = accuracy_score(y, preds) * 100
auc = roc_auc_score(y, probs)
f1  = f1_score(y, preds, average='macro') * 100
fpr, tpr, _ = roc_curve(y, probs)

print(f"\n{'='*45}")
print(f"  Accuracy  : {acc:.2f}%")
print(f"  ROC-AUC   : {auc:.4f}")
print(f"  Macro F1  : {f1:.2f}%")
print(f"{'='*45}")
print(classification_report(y, preds,
      target_names=["Legitimate", "Spoofed"], digits=3))