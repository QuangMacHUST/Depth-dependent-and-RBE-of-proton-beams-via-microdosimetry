#ifndef Axes_h
#define Axes_h 1

#include "globals.hh"

#include <algorithm>
#include <cmath>
#include <utility>

// Uniform binning along depth z.
struct LinearAxis
{
  G4double lo = 0.;
  G4double hi = 1.;
  G4int n = 1;

  G4double Width() const { return (hi - lo) / n; }
  G4double Edge(G4int i) const { return lo + i * Width(); }

  G4int Index(G4double x) const
  {
    if (x < lo || x >= hi) return -1;
    return std::min(static_cast<G4int>((x - lo) / Width()), n - 1);
  }

  // Calls f(bin, fraction) for every bin crossed by the straight segment
  // [z1, z2]; fraction is the share of the segment length inside the bin.
  template <class F>
  void ForEachOverlap(G4double z1, G4double z2, F&& f) const
  {
    if (z2 < z1) std::swap(z1, z2);
    const G4double dz = z2 - z1;
    if (dz <= 0.) {
      const G4int i = Index(z1);
      if (i >= 0) f(i, 1.);
      return;
    }
    const G4double w = Width();
    const G4int first = std::max(0, static_cast<G4int>(std::floor((z1 - lo) / w)));
    const G4int last = std::min(n - 1, static_cast<G4int>(std::floor((z2 - lo) / w)));
    for (G4int i = first; i <= last; ++i) {
      const G4double overlap = std::min(z2, Edge(i + 1)) - std::max(z1, Edge(i));
      if (overlap > 0.) f(i, overlap / dz);
    }
  }
};

// Logarithmic binning in kinetic energy.
struct LogAxis
{
  G4double lo = 1.;
  G4double hi = 10.;
  G4int n = 1;

  G4double Edge(G4int i) const { return lo * std::pow(hi / lo, static_cast<G4double>(i) / n); }

  G4int Index(G4double x) const
  {
    if (x < lo || x >= hi) return -1;
    return std::min(static_cast<G4int>(n * std::log(x / lo) / std::log(hi / lo)), n - 1);
  }
};

#endif
