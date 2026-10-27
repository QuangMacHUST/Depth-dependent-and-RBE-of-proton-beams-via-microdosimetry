"""Readers for the CSV files written by protonDepth, plus range metrics."""

from pathlib import Path

import numpy as np


def _split_comments(path):
    lines = Path(path).read_text().splitlines()
    comments = [ln for ln in lines if ln.startswith("#")]
    data = [ln for ln in lines if ln.strip() and not ln.startswith("#")]
    return comments, data


def load_depth(path):
    """Return {column name: numpy array} for a *_depth.csv file."""
    _, data = _split_comments(path)
    names = data[0].split(",")
    values = np.loadtxt(data[1:], delimiter=",", ndmin=2)
    return {name: values[:, i] for i, name in enumerate(names)}


def load_spectrum(path):
    """Return (energy_edges_MeV, z_lo_mm, z_hi_mm, dphi_dE) for a *_spectrum_*.csv file.

    dphi_dE has shape (n_depth, n_energy), in cm^-2 MeV^-1 per primary.
    """
    comments, data = _split_comments(path)
    edge_line = next(ln for ln in comments if ln.startswith("# E_edges_MeV"))
    edges = np.array([float(v) for v in edge_line.split(",")[1:]])
    values = np.loadtxt(data[1:], delimiter=",", ndmin=2)
    return edges, values[:, 0], values[:, 1], values[:, 2:]


def distal_depth(z, dose, level):
    """Depth beyond the maximum where the dose falls to `level` * max (linear interpolation)."""
    imax = int(np.argmax(dose))
    target = level * dose[imax]
    for i in range(imax, len(dose) - 1):
        if dose[i] >= target > dose[i + 1]:
            return z[i] + (dose[i] - target) / (dose[i] - dose[i + 1]) * (z[i + 1] - z[i])
    return float("nan")


def proximal_depth(z, dose, level):
    """Depth before the maximum where the dose first reaches `level` * max."""
    imax = int(np.argmax(dose))
    target = level * dose[imax]
    for i in range(imax, 0, -1):
        if dose[i] >= target > dose[i - 1]:
            return z[i - 1] + (target - dose[i - 1]) / (dose[i] - dose[i - 1]) * (z[i] - z[i - 1])
    return float("nan")
