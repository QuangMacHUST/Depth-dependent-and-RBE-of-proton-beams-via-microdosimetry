#!/usr/bin/env python3
"""Plot depth dose and LET from a *_depth.csv file and print range metrics.

Example:
    python3 plot_depth.py output/pristine_150MeV_depth.csv --save pdd_150.png
"""

import argparse

import matplotlib.pyplot as plt
import numpy as np

from step1_io import distal_depth, load_depth, proximal_depth


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("depth_csv")
    parser.add_argument("--save", help="save the figure instead of showing it")
    parser.add_argument("--let-cut", type=float, default=0.01,
                        help="hide LET where the dose is below this fraction of the maximum (default 0.01)")
    args = parser.parse_args(argv)

    d = load_depth(args.depth_csv)
    z = d["z_mid_mm"]
    dose = d["dose_Gy"]
    rel = dose / dose.max()

    print(f"depth of maximum dose : {z[np.argmax(dose)]:.2f} mm")
    print(f"R90 / R80 (distal)    : {distal_depth(z, dose, 0.9):.2f} / {distal_depth(z, dose, 0.8):.2f} mm")
    print(f"proximal 90 %         : {proximal_depth(z, dose, 0.9):.2f} mm")
    print(f"max rel. uncertainty in >50 % dose region: {d['dose_rel_unc'][rel > 0.5].max() * 100:.2f} %")

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(z, 100 * rel, color="k", label="Dose")
    ax.set_xlabel("Depth in water (mm)")
    ax.set_ylabel("Relative dose (%)")
    ax.set_ylim(bottom=0)

    ax2 = ax.twinx()
    mask = rel > args.let_cut
    for column, label in [("LETd_charged_all", "LET$_d$ all charged"),
                          ("LETd_proton_all", "LET$_d$ protons"),
                          ("LETt_proton_all", "LET$_t$ protons")]:
        ax2.plot(z[mask], d[column][mask], label=label)
    ax2.set_ylabel("LET (keV/µm)")
    ax2.set_ylim(bottom=0)

    lines = ax.get_legend_handles_labels()
    lines2 = ax2.get_legend_handles_labels()
    ax.legend(lines[0] + lines2[0], lines[1] + lines2[1], loc="upper left")
    fig.tight_layout()
    if args.save:
        fig.savefig(args.save, dpi=150)
    else:
        plt.show()


if __name__ == "__main__":
    main()
