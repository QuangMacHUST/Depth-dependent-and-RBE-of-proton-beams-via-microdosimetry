"""Readers for the files written by protonMicro and histogram-based moments."""

import re
from pathlib import Path

import numpy as np


def load_ydist(path):
    """Return {column name: numpy array} for a *_ydist.csv file."""
    lines = [ln for ln in Path(path).read_text().splitlines() if ln.strip() and not ln.startswith("#")]
    names = lines[0].split(",")
    values = np.loadtxt(lines[1:], delimiter=",", ndmin=2)
    return {name: values[:, i] for i, name in enumerate(names)}


_VALUE = re.compile(r"^([A-Za-z_*]+):\s*([-+0-9.eE]+)(?:\s*\+-\s*([-+0-9.eE]+))?")


def load_summary(path):
    """Return {key: (value, uncertainty or None)} for the numeric lines of a *_summary.txt file."""
    result = {}
    for line in Path(path).read_text().splitlines():
        match = _VALUE.match(line)
        if match:
            unc = float(match.group(3)) if match.group(3) else None
            result[match.group(1)] = (float(match.group(2)), unc)
    return result


def histogram_means(ydist, y0=150.0):
    """yF, yD and y* recomputed from the binned f(y) (bin-centre approximation)."""
    y = ydist["y_mid"]
    width = ydist["y_hi"] - ydist["y_lo"]
    f = ydist["f_y"] * width
    if f.sum() <= 0:
        return float("nan"), float("nan"), float("nan")
    f = f / f.sum()
    y_f = np.sum(y * f)
    y_d = np.sum(y * y * f) / y_f
    y_star = y0 * y0 * np.sum((1.0 - np.exp(-(y / y0) ** 2)) * f) / y_f
    return y_f, y_d, y_star
