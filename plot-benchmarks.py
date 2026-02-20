import csv
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import argparse
import math
from scipy.stats import mannwhitneyu

# CLI flags:
#   --csv PATH: benchmark CSV to read (default: tests/benchmark.csv)
#   --out-dir DIR: where to write plots (default: CSV directory)

parser = argparse.ArgumentParser(description="Plot planner benchmarks from a CSV.")
parser.add_argument("--csv", default="tests/benchmark.csv", help="Path to benchmark CSV.")
parser.add_argument("--out-dir", default=None, help="Directory to place plots (defaults to CSV dir).")
args = parser.parse_args()

CSV_PATH = Path(args.csv)
OUT_DIR = Path(args.out_dir) if args.out_dir else CSV_PATH.parent
OUT_DIR.mkdir(parents=True, exist_ok=True)

rows = []
with CSV_PATH.open(newline="") as f:
    for line in f:
        if line.startswith("#") or not line.strip():
            continue
        rows.append(line)

reader = csv.DictReader(rows)

data = []
for r in reader:
    try:
        n = int(r["n"]) if r["n"] else None
        broken = int(r["broken_count"]) if r["broken_count"] else None
        planner_status = int(r["planner_status"])
        planner_time = float(r["planner_time_s"])
        bf_status = int(r["bruteforce_status"])
        bf_time = float(r["bruteforce_time_s"])
        auto_status = int(r["automata_status"]) if r.get("automata_status") else None
        auto_time = float(r["automata_time_s"]) if r.get("automata_time_s") else float("nan")
    except Exception:
        continue
    data.append(
        (
            r["case"],
            n,
            broken,
            planner_status,
            planner_time,
            bf_status,
            bf_time,
            auto_status,
            auto_time,
        )
    )

# Filter to successful runs with known n.
ok = [d for d in data if d[1] is not None]
TIMEOUT_STATUSES = {124, 137}
planner_ok = [d for d in ok if d[3] == 0]
planner_to = [d for d in ok if d[3] in TIMEOUT_STATUSES]
bf_ok = [d for d in ok if d[5] == 0]
bf_to = [d for d in ok if d[5] in TIMEOUT_STATUSES]
auto_ok = [d for d in ok if d[7] == 0]
auto_to = [d for d in ok if d[7] in TIMEOUT_STATUSES]

# ---- Plot 1: time vs n (scatter + mean line) ----
fig, ax = plt.subplots(figsize=(8, 5))

ax.scatter([d[1] for d in planner_ok], [d[4] for d in planner_ok], label="planner", alpha=0.8)
ax.scatter([d[1] for d in bf_ok], [d[6] for d in bf_ok], label="bruteforce", alpha=0.8)
if auto_ok:
    ax.scatter([d[1] for d in auto_ok], [d[8] for d in auto_ok], label="automata", alpha=0.8)
if planner_to:
    ax.scatter([d[1] for d in planner_to], [d[4] for d in planner_to],
               label="planner timeout", marker="x")
if bf_to:
    ax.scatter([d[1] for d in bf_to], [d[6] for d in bf_to],
               label="bruteforce timeout", marker="x")
if auto_to:
    ax.scatter([d[1] for d in auto_to], [d[8] for d in auto_to],
               label="automata timeout", marker="x")


def mean_by_n(points, idx_time):
    buckets = defaultdict(list)
    for d in points:
        buckets[d[1]].append(d[idx_time])
    xs = sorted(buckets)
    ys = [sum(buckets[x]) / len(buckets[x]) for x in xs]
    return xs, ys

px, py = mean_by_n(planner_ok, 4)
bx, by = mean_by_n(bf_ok, 6)
ax_x, ax_y = mean_by_n(auto_ok, 8) if auto_ok else ([], [])
if px:
    ax.plot(px, py, linestyle="--")
if bx:
    ax.plot(bx, by, linestyle="--")
if ax_x:
    ax.plot(ax_x, ax_y, linestyle="--")

ax.set_title("Planner Runtime vs n")
ax.set_xlabel("n (number of intersections)")
ax.set_ylabel("time (seconds)")
ax.legend()
ax.grid(True, alpha=0.3)

out1 = OUT_DIR / "benchmark-time-vs-n.png"
fig.tight_layout()
fig.savefig(out1, dpi=160)
plt.close(fig)

# ---- Plot 2: per-case comparison (log scale if needed) ----
cases = [d[0] for d in ok]
planner_times = [d[4] for d in ok]
bf_times = [d[6] for d in ok]
auto_times = [d[8] for d in ok] if any(d[7] is not None for d in ok) else []

fig, ax = plt.subplots(figsize=(10, 5))
ax.plot(planner_times, marker="o", label="planner")
ax.plot(bf_times, marker="o", label="bruteforce")
if auto_times:
    ax.plot(auto_times, marker="o", label="automata")

ax.set_title("Per-case Runtime Comparison")
ax.set_xlabel("case index")
ax.set_ylabel("time (seconds)")
ax.legend()
ax.grid(True, alpha=0.3)

# Use log scale when times vary a lot.
all_times = planner_times + bf_times + auto_times
max_time = max([t for t in all_times if t == t] or [0.0])
min_time = min([t for t in all_times if t == t] or [0.0])
if max_time > 0 and min_time > 0 and max_time / min_time > 50:
    ax.set_yscale("log")

out2 = OUT_DIR / "benchmark-time-vs-case.png"
fig.tight_layout()
fig.savefig(out2, dpi=160)
plt.close(fig)

print(f"Wrote {out1}")
print(f"Wrote {out2}")

# ---- Plot 3: locality-only detail view ----
fig, (axl, axr) = plt.subplots(1, 2, figsize=(10, 4))

planner_ms = [1000.0 * d[4] for d in planner_ok]
planner_n = [d[1] for d in planner_ok]
axl.scatter(planner_n, planner_ms, alpha=0.85, label="locality runs")
if px:
    axl.plot(px, [1000.0 * y for y in py], linestyle="--", label="mean by n")
axl.set_title("Locality Runtime vs n")
axl.set_xlabel("n (number of intersections)")
axl.set_ylabel("time (ms)")
axl.grid(True, alpha=0.3)
axl.legend()

planner_ms_all = [1000.0 * t for t in planner_times]
axr.plot(planner_ms_all, marker="o", linewidth=1.2)
axr.set_title("Locality Runtime per Case")
axr.set_xlabel("case index")
axr.set_ylabel("time (ms)")
axr.grid(True, alpha=0.3)

out3 = OUT_DIR / "benchmark-time-locality.png"
fig.tight_layout()
fig.savefig(out3, dpi=160)
plt.close(fig)

print(f"Wrote {out3}")

# ---- Plot 4: median with IQR by n for all methods ----
def quantile(sorted_vals, q):
    if not sorted_vals:
        return float("nan")
    if len(sorted_vals) == 1:
        return sorted_vals[0]
    pos = (len(sorted_vals) - 1) * q
    lo = int(math.floor(pos))
    hi = int(math.ceil(pos))
    if lo == hi:
        return sorted_vals[lo]
    w = pos - lo
    return sorted_vals[lo] * (1.0 - w) + sorted_vals[hi] * w


def series_by_n(points, idx_time):
    buckets = defaultdict(list)
    for d in points:
        if d[1] is None:
            continue
        buckets[d[1]].append(d[idx_time])
    xs = sorted(buckets.keys())
    medians = []
    q25 = []
    q75 = []
    for x in xs:
        vals = sorted(buckets[x])
        medians.append(quantile(vals, 0.5))
        q25.append(quantile(vals, 0.25))
        q75.append(quantile(vals, 0.75))
    return xs, medians, q25, q75


pxs, pmed, pq25, pq75 = series_by_n(ok, 4)
bxs, bmed, bq25, bq75 = series_by_n(ok, 6)
axs, amed, aq25, aq75 = series_by_n(ok, 8)

fig, ax = plt.subplots(figsize=(8, 5))

def draw_median_iqr(xs, median, q25, q75, label, color):
    if not xs:
        return
    ax.plot(xs, median, linewidth=2.2, color=color, label=label)
    ax.plot(xs, q75, linewidth=1.2, linestyle="--", color=color, alpha=0.9)
    ax.plot(xs, q25, linewidth=1.2, linestyle="--", color=color, alpha=0.9)
    ax.fill_between(xs, q25, q75, color=color, alpha=0.12)

draw_median_iqr(pxs, pmed, pq25, pq75, "locality (median, IQR)", "#1f77b4")
draw_median_iqr(bxs, bmed, bq25, bq75, "bruteforce (median, IQR)", "#d62728")
draw_median_iqr(axs, amed, aq25, aq75, "automata (median, IQR)", "#2ca02c")

ax.set_title("Median Runtime by n with IQR Bands")
ax.set_xlabel("n (number of intersections)")
ax.set_ylabel("time (seconds)")
ax.grid(True, alpha=0.3)
ax.legend()

out4 = OUT_DIR / "benchmark-time-median-iqr-vs-n.png"
fig.tight_layout()
fig.savefig(out4, dpi=160)
plt.close(fig)

print(f"Wrote {out4}")

# ---- Mann-Whitney U tests: locality vs baselines ----
planner_all = [d[4] for d in ok if not math.isnan(d[4])]
bf_all = [d[6] for d in ok if not math.isnan(d[6])]
auto_all = [d[8] for d in ok if not math.isnan(d[8])]

u_pb, p_pb = mannwhitneyu(planner_all, bf_all, alternative="less")
u_pa, p_pa = mannwhitneyu(planner_all, auto_all, alternative="less")

stats_path = OUT_DIR / "benchmark-stats.txt"
with stats_path.open("w") as f:
    f.write("Mann-Whitney U tests (alternative: locality time < baseline time)\n")
    f.write(f"locality vs brute-force: U={u_pb:.3f}, p={p_pb:.12g}\n")
    f.write(f"locality vs automata: U={u_pa:.3f}, p={p_pa:.12g}\n")

print(f"Wrote {stats_path}")
