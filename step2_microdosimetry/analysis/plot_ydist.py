#!/usr/bin/env python3
"""Plot y d(y) on a log y axis for one or more *_ydist.csv files.

Example:
    python3 plot_ydist.py output/proton_*MeV_ydist.csv --save ydy.png
"""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt

from micro_io import histogram_means, load_summary, load_ydist


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("files", nargs="+")
    parser.add_argument("--frequency", action="store_true", help="plot y f(y) instead of y d(y)")
    parser.add_argument("--save", help="save the figure instead of showing it")
    args = parser.parse_args(argv)

    fig, ax = plt.subplots(figsize=(8, 5))
    print(f"{'file':40s} {'yF':>9s} {'yD':>9s} {'y*':>9s}   (keV/um, histogram / summary)")
    for path in args.files:
        d = load_ydist(path)
        column = "yf_y" if args.frequency else "yd_y"
        label = Path(path).name.replace("_ydist.csv", "")
        ax.step(d["y_mid"], d[column], where="mid", label=label)

        y_f, y_d, y_star = histogram_means(d)
        summary_path = Path(path).with_name(Path(path).name.replace("_ydist.csv", "_summary.txt"))
        line = f"{label:40s} {y_f:9.3f} {y_d:9.3f} {y_star:9.3f}"
        if summary_path.exists():
            s = load_summary(summary_path)
            line += "   /" + "".join(f" {s[k][0]:9.3f}" for k in ("yF_keV_per_um", "yD_keV_per_um", "ystar_keV_per_um"))
        print(line)

    ax.set_xscale("log")
    ax.set_xlabel("y (keV/µm)")
    ax.set_ylabel("y f(y)" if args.frequency else "y d(y)")
    ax.legend(fontsize=8)
    fig.tight_layout()
    if args.save:
        fig.savefig(args.save, dpi=150)
    else:
        plt.show()


if __name__ == "__main__":
    main()
