// Checks of SiteSampler against analytic microdosimetry (no Geant4 needed).
//
// 1. A straight track of constant LET L, represented by many closely spaced
//    equal transfer points, crossing a sphere of diameter d gives the
//    mu-randomness chord distribution f(l) = 2 l / d^2, hence
//    yF = L and yD = 9/8 L for y = eps / (2d/3).
// 2. EnergyInSite agrees with a brute-force sum over all points.

#include "SiteSampler.hh"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>

namespace
{
int failures = 0;

void Check(bool ok, const char* what, double got, double expected)
{
  std::printf("%-40s got %.5g expected %.5g  %s\n", what, got, expected, ok ? "ok" : "FAILED");
  if (!ok) ++failures;
}
}  // namespace

int main()
{
  std::mt19937_64 engine(12345);
  std::uniform_real_distribution<double> flat(0., 1.);
  const std::function<double()> uniform = [&] { return flat(engine); };

  // --- 1. constant-LET line along z (units: um and keV) ---
  const double d = 1.0, r = 0.5 * d;
  const double let = 2.0;  // keV/um
  const double spacing = 1e-3;
  std::vector<TransferPoint> line;
  for (double z = -2.; z < 12.; z += spacing) line.push_back({0., 0., z, let * spacing});

  SiteSampler sampler(r, 0., 10.);
  sampler.SetPoints(line);
  const double meanChord = 2. * d / 3.;
  const int trials = 400000;
  const double w = sampler.TotalEnergy() / trials;
  double sumW = 0., sumWy = 0., sumWOverY = 0.;
  int accepted = 0;
  for (int t = 0; t < trials; ++t) {
    const double eps = sampler.Sample(uniform);
    if (eps < 0.) continue;
    ++accepted;
    const double y = eps / meanChord;
    sumW += w;
    sumWy += w * y;
    sumWOverY += w / y;
  }
  const double yF = sumW / sumWOverY;
  const double yD = sumWy / sumW;
  Check(std::abs(yF / let - 1.) < 0.01, "yF (line track) / L", yF / let, 1.);
  Check(std::abs(yD / let - 1.125) < 0.01, "yD (line track) / L", yD / let, 1.125);
  // Accepted fraction: centres within the 10 um zone out of a 14 um track
  // padded by r on each side of each point (ball volume cut by the zone)
  Check(std::abs(accepted / double(trials) - 10. / 14.) < 0.01, "accepted fraction",
        accepted / double(trials), 10. / 14.);

  // --- 2. random cloud: grid lookup vs brute force ---
  std::vector<TransferPoint> cloud;
  for (int i = 0; i < 5000; ++i)
    cloud.push_back({4. * flat(engine) - 2., 4. * flat(engine) - 2., 4. * flat(engine) - 2., flat(engine)});
  SiteSampler cloudSampler(0.37, -10., 10.);
  cloudSampler.SetPoints(cloud);
  double maxDiff = 0.;
  for (int t = 0; t < 2000; ++t) {
    const double x = 4. * flat(engine) - 2., y = 4. * flat(engine) - 2., z = 4. * flat(engine) - 2.;
    double brute = 0.;
    for (const auto& p : cloud) {
      const double dx = p.x - x, dy = p.y - y, dz = p.z - z;
      if (dx * dx + dy * dy + dz * dz < 0.37 * 0.37) brute += p.e;
    }
    maxDiff = std::max(maxDiff, std::abs(brute - cloudSampler.EnergyInSite(x, y, z)));
  }
  Check(maxDiff < 1e-9, "grid vs brute-force max difference", maxDiff, 0.);

  // --- 3. empty track ---
  std::vector<TransferPoint> empty;
  SiteSampler emptySampler(0.5, 0., 1.);
  emptySampler.SetPoints(empty);
  Check(emptySampler.Sample(uniform) < 0., "empty track gives no sample", 0., 0.);

  std::printf(failures ? "%d check(s) FAILED\n" : "all checks passed\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
