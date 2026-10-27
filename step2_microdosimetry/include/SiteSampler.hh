#ifndef SiteSampler_h
#define SiteSampler_h 1

// Pure C++ (no Geant4 dependency) so that it can be unit-tested on its own.

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

struct TransferPoint
{
  double x, y, z;  // position
  double e;  // energy deposited
};

// Energy imparted to a spherical site placed at random around one track.
//
// A transfer point i is chosen with probability e_i / E and the site centre c
// is placed uniformly in the ball of radius r around it. The density of c is
// then proportional to eps(c), the energy imparted to a site centred at c, so
// the sampled eps follow the dose-weighted distribution d(eps); weighting each
// sample by 1/eps gives the frequency distribution f(eps). Centres outside the
// scoring zone zLo <= z < zHi are rejected, so with N trials every accepted
// sample carries the weight E / N (independent of the track).
class SiteSampler
{
  public:
    SiteSampler(double radius, double zLo, double zHi);

    void SetPoints(const std::vector<TransferPoint>& points);
    double TotalEnergy() const { return fTotalEnergy; }

    // Energy imparted to one randomly placed site, or a negative value if the
    // sampled centre fell outside the scoring zone. `uniform` returns [0, 1).
    double Sample(const std::function<double()>& uniform) const;

    // Energy imparted to a site centred at (x, y, z)
    double EnergyInSite(double x, double y, double z) const;

  private:
    std::int64_t Key(std::int64_t ix, std::int64_t iy, std::int64_t iz) const;
    std::int64_t Cell(double v) const;

    double fRadius;
    double fZLo;
    double fZHi;
    const std::vector<TransferPoint>* fPoints = nullptr;
    std::vector<double> fCumulative;
    double fTotalEnergy = 0.;
    std::unordered_map<std::int64_t, std::vector<int>> fGrid;
};

#endif
