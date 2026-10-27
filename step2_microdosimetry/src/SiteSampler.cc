#include "SiteSampler.hh"

#include <algorithm>
#include <cmath>

SiteSampler::SiteSampler(double radius, double zLo, double zHi)
  : fRadius(radius), fZLo(zLo), fZHi(zHi)
{}

std::int64_t SiteSampler::Cell(double v) const
{
  return static_cast<std::int64_t>(std::floor(v / fRadius));
}

std::int64_t SiteSampler::Key(std::int64_t ix, std::int64_t iy, std::int64_t iz) const
{
  // 21 bits per axis, offset to keep the indices positive
  const std::int64_t offset = 1 << 20;
  const std::int64_t mask = (1 << 21) - 1;
  return (((ix + offset) & mask) << 42) | (((iy + offset) & mask) << 21) | ((iz + offset) & mask);
}

void SiteSampler::SetPoints(const std::vector<TransferPoint>& points)
{
  fPoints = &points;
  fGrid.clear();
  fCumulative.resize(points.size());
  double sum = 0.;
  for (std::size_t i = 0; i < points.size(); ++i) {
    const TransferPoint& p = points[i];
    sum += p.e;
    fCumulative[i] = sum;
    fGrid[Key(Cell(p.x), Cell(p.y), Cell(p.z))].push_back(static_cast<int>(i));
  }
  fTotalEnergy = sum;
}

double SiteSampler::EnergyInSite(double x, double y, double z) const
{
  // Cells have the size of the radius, so the site overlaps at most the
  // 3 x 3 x 3 cells around its centre
  const double r2 = fRadius * fRadius;
  const std::int64_t cx = Cell(x), cy = Cell(y), cz = Cell(z);
  double energy = 0.;
  for (std::int64_t ix = cx - 1; ix <= cx + 1; ++ix)
    for (std::int64_t iy = cy - 1; iy <= cy + 1; ++iy)
      for (std::int64_t iz = cz - 1; iz <= cz + 1; ++iz) {
        const auto it = fGrid.find(Key(ix, iy, iz));
        if (it == fGrid.end()) continue;
        for (int i : it->second) {
          const TransferPoint& p = (*fPoints)[i];
          const double dx = p.x - x, dy = p.y - y, dz = p.z - z;
          if (dx * dx + dy * dy + dz * dz < r2) energy += p.e;
        }
      }
  return energy;
}

double SiteSampler::Sample(const std::function<double()>& uniform) const
{
  if (fTotalEnergy <= 0.) return -1.;

  const double u = uniform() * fTotalEnergy;
  const std::size_t i = std::min<std::size_t>(
    std::upper_bound(fCumulative.begin(), fCumulative.end(), u) - fCumulative.begin(),
    fCumulative.size() - 1);
  const TransferPoint& p = (*fPoints)[i];

  double dx, dy, dz;
  do {
    dx = 2. * uniform() - 1.;
    dy = 2. * uniform() - 1.;
    dz = 2. * uniform() - 1.;
  } while (dx * dx + dy * dy + dz * dz > 1.);

  const double z = p.z + fRadius * dz;
  if (z < fZLo || z >= fZHi) return -1.;
  return EnergyInSite(p.x + fRadius * dx, p.y + fRadius * dy, z);
}
