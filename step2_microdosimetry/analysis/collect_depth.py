#!/usr/bin/env python3
"""Collect yF, yD and y* versus depth from depth_scan.mac summaries into one CSV
(the input of the MKM step).

Example:
    python3 collect_depth.py output/depth_*mm_summary.txt --output micro_vs_depth.csv
"""

import argparse
import sys

from micro_io import load_summary

KEYS = ("yF_keV_per_um", "yD_keV_per_um", "ystar_keV_per_um")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("summaries", nargs="+")
    parser.add_argument("--output", default="micro_vs_depth.csv")
    args = parser.parse_args(argv)

    rows = []
    for path in args.summaries:
        s = load_summary(path)
        if "spectrum_depth_mm" not in s:
            sys.exit(f"{path}: not a spectrum-mode run (no spectrum_depth_mm)")
        row = [s["spectrum_depth_mm"][0], s["site_diameter_um"][0]]
        for key in KEYS:
            value, unc = s[key]
            row += [value, unc if unc is not None else 0.0]
        rows.append(row)
    rows.sort()

    header = "depth_mm,site_um,yF,yF_unc,yD,yD_unc,ystar,ystar_unc"
    with open(args.output, "w") as out:
        out.write("# lineal energies in keV/um\n" + header + "\n")
        for row in rows:
            out.write(",".join(f"{v:.6g}" for v in row) + "\n")
    print(f"{len(rows)} depths written to {args.output}")


if __name__ == "__main__":
    main()
