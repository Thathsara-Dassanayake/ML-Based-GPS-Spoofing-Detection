# ============================================================
# NEMESIS-Shield — Single Bin Inference
# Depends only on:
#   nemesis_encoder.pt
#   nemesis_mlp_probe.pt
#   nemesis_mlp_scaler.pkl
#   <your_bin_file>.bin
# ============================================================

import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from sklearn.metrics import (accuracy_score, roc_auc_score,
                             f1_score, classification_report,
                             ConfusionMatrixDisplay, roc_curve)
from copy import deepcopy
import pickle, os
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# ═══════════════════════════════════════════════════════════════
# CONFIG  — edit these
# ═══════════════════════════════════════════════════════════════

ENCODER_PATH = r".\ML_model_2\Model\nemesis_encoder.pt"
PROBE_PATH   = r".\ML_model_2\Model\nemesis_mlp_probe.pt"
SCALER_PATH  = r".\ML_model_2\Model\nemesis_mlp_scaler.pkl"

INPUT_BIN    = r".\ML_model_2\test_data\nemesis_spoofed.bin"   # ← your bin file
TRUE_LABEL   = 0     # 0 = Legitimate, 1 = Spoofed
                     # set to None to skip metrics (unknown source)

# ── Must match training ───────────────────────────────────────
SEGMENT_LEN = 512
N_SCALES    = 64
EMBED_DIM   = 256
DEVICE      = torch.device("cuda" if torch.cuda.is_available() else "cpu")

print(f"Device : {DEVICE}")

# ═══════════════════════════════════════════════════════════════
# MODEL DEFINITIONS
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
        return self.online(x)

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
# LOAD BIN FILE
# ═══════════════════════════════════════════════════════════════

print(f"Loading {INPUT_BIN} …")
X = np.fromfile(INPUT_BIN, dtype=np.float32).reshape(-1, 1, N_SCALES, SEGMENT_LEN)
print(f"  Samples: {len(X)}")

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

print("Extracting embeddings …")
emb = extract_embeddings(encoder, X)

print("Running MLP probe …")
probe.eval()
X_sc = torch.FloatTensor(scaler.transform(emb)).to(DEVICE)
with torch.no_grad():
    logits = probe(X_sc)
    probs  = F.softmax(logits, dim=1)[:, 1].cpu().numpy()
    preds  = logits.argmax(1).cpu().numpy()

# ═══════════════════════════════════════════════════════════════
# RESULTS
# ═══════════════════════════════════════════════════════════════

n_legit = (preds == 0).sum()
n_spoof = (preds == 1).sum()

print(f"\n{'='*45}")
print(f"  File          : {os.path.basename(INPUT_BIN)}")
print(f"  Total samples : {len(preds)}")
print(f"  → Legitimate  : {n_legit}  ({n_legit/len(preds)*100:.1f}%)")
print(f"  → Spoofed     : {n_spoof}  ({n_spoof/len(preds)*100:.1f}%)")
print(f"{'='*45}")

# Default values so plotting code does not break
y = None
acc = None
auc = None
f1 = None
fpr = None
tpr = None

if TRUE_LABEL is not None:
    y = np.full(len(preds), TRUE_LABEL, dtype=np.int64)
    acc = accuracy_score(y, preds) * 100
    f1  = f1_score(y, preds, average='macro') * 100

    print(f"\n  Accuracy  : {acc:.2f}%")

    unique_classes = np.unique(y)

    # ROC-AUC only makes sense if both classes are present in y
    if len(unique_classes) == 2:
        auc = roc_auc_score(y, probs)
        fpr, tpr, _ = roc_curve(y, probs)
        print(f"  ROC-AUC   : {auc:.4f}")
    else:
        print("  ROC-AUC   : N/A (only one class present in TRUE_LABEL/y_true)")

    print(f"  Macro F1  : {f1:.2f}%")
    print(f"{'='*45}")

    # classification_report must match the labels actually present
    if len(unique_classes) == 2:
        print(classification_report(
            y, preds,
            labels=[0, 1],
            target_names=["Legitimate", "Spoofed"],
            digits=3,
            zero_division=0
        ))
    else:
        present_label = int(unique_classes[0])
        present_name = "Legitimate" if present_label == 0 else "Spoofed"

        print(classification_report(
            y, preds,
            labels=[present_label],
            target_names=[present_name],
            digits=3,
            zero_division=0
        ))

# ═══════════════════════════════════════════════════════════════
# PLOTS
# ═══════════════════════════════════════════════════════════════

plt.rcParams.update({
    'font.family'    : 'serif',
    'font.serif'     : ['Times New Roman', 'Times', 'DejaVu Serif'],
    'font.size'      : 5,   'axes.titlesize' : 5.5,
    'axes.labelsize' : 5,   'xtick.labelsize': 4.5,
    'ytick.labelsize': 4.5, 'legend.fontsize': 4.5,
    'axes.linewidth' : 0.5, 'lines.linewidth': 0.9,
    'axes.grid'      : True,'grid.linestyle' : '--',
    'grid.linewidth' : 0.3, 'grid.alpha'     : 0.45,
    'axes.axisbelow' : True,
    'pdf.fonttype'   : 42,  'ps.fonttype'    : 42,
})

has_full_binary_metrics = (y is not None) and (len(np.unique(y)) == 2)
n_plots = 4 if has_full_binary_metrics else 2
fig, axes = plt.subplots(1, n_plots, figsize=(n_plots * 2.4, 2.4))
fig.subplots_adjust(wspace=0.42)

# (1) Score distribution
ax = axes[0]
ax.hist(probs, bins=40, color='#0D47A1', alpha=0.75, density=True)
ax.axvline(0.5, color='r', lw=0.8, linestyle='--', label='τ=0.5')
ax.set_title(f"Score Distribution")
ax.set_xlabel("Spoof Probability")
ax.set_ylabel("Density")
ax.legend()

# (2) Prediction pie
ax = axes[1]
ax.pie([n_legit, n_spoof],
       labels=[f"Legit\n{n_legit}", f"Spoof\n{n_spoof}"],
       colors=['#2196F3', '#F44336'],
       autopct='%1.1f%%',
       textprops={'fontsize': 5},
       startangle=90)
ax.set_title("Prediction Split")

# (3) & (4) only when ground truth is available
if has_full_binary_metrics:

    # (3) ROC
    ax = axes[2]
    ax.plot(fpr, tpr, color='#0D47A1', lw=1.2,
            label=f"AUC={auc:.3f}")
    ax.plot([0,1],[0,1],'k--', lw=0.5)
    ax.set_title("ROC Curve")
    ax.set_xlabel("FPR"); ax.set_ylabel("TPR")
    ax.legend(loc='lower right')
    ax.xaxis.set_major_locator(ticker.MultipleLocator(0.2))
    ax.yaxis.set_major_locator(ticker.MultipleLocator(0.2))

    # (4) Confusion matrix
    ax = axes[3]
    ConfusionMatrixDisplay.from_predictions(
        y, preds,
        display_labels=["Legit", "Spoof"],
        cmap='Blues', ax=ax, colorbar=False,
        text_kw={"fontsize": 5})
    ax.set_title(f"Confusion Matrix\nAcc={acc:.1f}%")
    ax.set_xlabel("Predicted"); ax.set_ylabel("True")

fname = os.path.splitext(os.path.basename(INPUT_BIN))[0]
plt.savefig(f"nemesis_{fname}.pdf")
plt.savefig(f"nemesis_{fname}.png", dpi=200)
plt.show()
print(f"\nPlots saved → nemesis_{fname}.pdf / .png")