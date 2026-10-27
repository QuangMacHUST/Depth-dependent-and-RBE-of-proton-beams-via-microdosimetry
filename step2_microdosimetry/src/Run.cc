#include "Run.hh"

#include "G4SystemOfUnits.hh"
#include "G4Version.hh"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace
{
void AddInto(std::vector<G4double>& target, const std::vector<G4double>& source)
{
  for (std::size_t i = 0; i < target.size(); ++i) target[i] += source[i];
}
}  // namespace

Run::Run(const MicroConfig& config) : fConfig(config)
{
  fMeanChord = 2. * fConfig.siteDiameter / 3. / um;  // sphere: 4V/S = 2d/3
  if (fConfig.yMin <= 0. || fConfig.yMax <= fConfig.yMin)
    G4Exception("Run::Run", "Micro020", FatalException, "require 0 < /micro/score/yMin < yMax");
  fNBins = static_cast<G4int>(std::ceil(fConfig.binsPerDecade * std::log10(fConfig.yMax / fConfig.yMin)));
  fHistD.assign(static_cast<std::size_t>(kNSourceSpecies) * fNBins, 0.);
  fHistF.assign(static_cast<std::size_t>(kNSourceSpecies) * fNBins, 0.);
}

G4double Run::Edge(G4int bin) const
{
  return fConfig.yMin * std::pow(10., static_cast<G4double>(bin) / fConfig.binsPerDecade);
}

G4int Run::Bin(G4double y) const
{
  if (y < fConfig.yMin) return -1;
  const auto bin = static_cast<G4int>(std::floor(fConfig.binsPerDecade * std::log10(y / fConfig.yMin)));
  return bin < fNBins ? bin : -1;
}

void Run::Merge(const G4Run* other)
{
  const auto run = static_cast<const Run*>(other);
  AddInto(fHistD, run->fHistD);
  AddInto(fHistF, run->fHistF);
  for (G4int im = 0; im < kNMoments; ++im) fMoments[im] += run->fMoments[im];
  for (G4int is = 0; is < kNSourceSpecies; ++is) {
    for (G4int k = 0; k < 4; ++k) fSpeciesSums[is][k] += run->fSpeciesSums[is][k];
    fSpeciesEvents[is] += run->fSpeciesEvents[is];
  }
  fOutOfRange += run->fOutOfRange;
  fEventsWithHits += run->fEventsWithHits;
  fAccepted += run->fAccepted;
  G4Run::Merge(other);
}

void Run::BeginEvent()
{
  for (G4double& v : fEvent) v = 0.;
}

void Run::AddSample(G4int species, G4double weight, G4double y)
{
  const G4double y0 = fConfig.y0;
  fEvent[0] += weight * y;
  fEvent[1] += weight;
  fEvent[2] += weight / y;
  fEvent[3] += weight / y * (1. - std::exp(-y * y / (y0 * y0)));
  fAccepted += 1.;

  const G4int bin = Bin(y);
  if (bin < 0) {
    fOutOfRange += weight;
    return;
  }
  fHistD[species * fNBins + bin] += weight;
  fHistF[species * fNBins + bin] += weight / y;
}

void Run::EndEvent(G4int species)
{
  const G4double a = fEvent[0], b = fEvent[1], c = fEvent[2], sv = fEvent[3];
  fMoments[kA] += a;
  fMoments[kB] += b;
  fMoments[kC] += c;
  fMoments[kS] += sv;
  fMoments[kAA] += a * a;
  fMoments[kBB] += b * b;
  fMoments[kCC] += c * c;
  fMoments[kSS] += sv * sv;
  fMoments[kAB] += a * b;
  fMoments[kBC] += b * c;
  fMoments[kSB] += sv * b;
  for (G4int k = 0; k < 4; ++k) fSpeciesSums[species][k] += fEvent[k];
  fSpeciesEvents[species] += 1.;
  if (b > 0.) fEventsWithHits += 1.;
}

// Standard error of R = sum X / sum Y over n events (first-order expansion)
G4double Run::RatioUncertainty(G4int x, G4int y, G4int xx, G4int yy, G4int xy) const
{
  const G4double n = numberOfEvent;
  if (n < 2 || fMoments[y] <= 0.) return 0.;
  const G4double r = fMoments[x] / fMoments[y];
  const G4double var = fMoments[xx] - 2. * r * fMoments[xy] + r * r * fMoments[yy];
  return std::sqrt(std::max(var, 0.) * n / (n - 1.)) / fMoments[y];
}

void Run::Write() const
{
  const G4String& prefix = fConfig.outputPrefix;
  const std::filesystem::path prefixPath(prefix.c_str());
  if (prefixPath.has_parent_path()) std::filesystem::create_directories(prefixPath.parent_path());

  WriteDistribution(prefix + "_ydist.csv");
  WriteSummary(prefix + "_summary.txt");

  const G4double b = fMoments[kB];
  if (b > 0.) {
    G4cout << "\n Run " << runID << ": " << numberOfEvent << " events, site " << fConfig.siteDiameter / um
           << " um\n   yF = " << b / fMoments[kC] << " keV/um, yD = " << fMoments[kA] / b
           << " keV/um, y* = " << fConfig.y0 * fConfig.y0 * fMoments[kS] / b << " keV/um\n"
           << "   output written to " << prefix << "_*\n"
           << G4endl;
  }
}

void Run::WriteDistribution(const G4String& path) const
{
  G4double totalD = 0., totalF = 0.;
  std::vector<G4double> d(fNBins, 0.), f(fNBins, 0.);
  for (G4int is = 0; is < kNSourceSpecies; ++is)
    for (G4int i = 0; i < fNBins; ++i) {
      d[i] += fHistD[is * fNBins + i];
      f[i] += fHistF[is * fNBins + i];
    }
  for (G4int i = 0; i < fNBins; ++i) {
    totalD += d[i];
    totalF += f[i];
  }

  std::ofstream out(path);
  out << "# Lineal energy distributions, y in keV/um, site diameter " << fConfig.siteDiameter / um
      << " um\n"
      << "# f_y, d_y: probability densities (per keV/um), normalised over the histogram range;\n"
      << "# yf_y, yd_y: y times the density (area 1 per unit ln y on a log axis);\n"
      << "# d_y_<species>: contribution of each primary species to d_y (they add up to d_y)\n"
      << "y_lo,y_hi,y_mid,f_y,d_y,yf_y,yd_y";
  for (G4int is = 0; is < kNSourceSpecies; ++is) out << ",d_y_" << SourceName(is);
  out << '\n' << std::setprecision(7);

  for (G4int i = 0; i < fNBins; ++i) {
    const G4double lo = Edge(i), hi = Edge(i + 1), mid = std::sqrt(lo * hi), dy = hi - lo;
    const G4double fy = totalF > 0. ? f[i] / totalF / dy : 0.;
    const G4double dyv = totalD > 0. ? d[i] / totalD / dy : 0.;
    out << lo << ',' << hi << ',' << mid << ',' << fy << ',' << dyv << ',' << mid * fy << ','
        << mid * dyv;
    for (G4int is = 0; is < kNSourceSpecies; ++is)
      out << ',' << (totalD > 0. ? fHistD[is * fNBins + i] / totalD / dy : 0.);
    out << '\n';
  }
}

void Run::WriteSummary(const G4String& path) const
{
  const G4double b = fMoments[kB];
  const G4double yF = b > 0. ? b / fMoments[kC] : 0.;
  const G4double yD = b > 0. ? fMoments[kA] / b : 0.;
  const G4double y02 = fConfig.y0 * fConfig.y0;
  const G4double yStar = b > 0. ? y02 * fMoments[kS] / b : 0.;
  const G4double sF = RatioUncertainty(kC, kB, kCC, kBB, kBC) * yF * yF;  // yF = 1 / (C / B)
  const G4double sD = RatioUncertainty(kA, kB, kAA, kBB, kAB);
  const G4double sStar = y02 * RatioUncertainty(kS, kB, kSS, kBB, kSB);

  std::ofstream out(path);
  out << std::setprecision(6);
  out << "geant4_version: " << G4Version << '\n'
      << "physics_list: " << fConfig.physicsList << '\n'
      << "events: " << numberOfEvent << '\n'
      << "events_with_energy_in_zone: " << fEventsWithHits << '\n'
      << "accepted_site_samples: " << fAccepted << '\n'
      << "source_mode: " << fConfig.sourceMode << '\n';
  if (fConfig.sourceMode == "spectrum")
    out << "spectrum_prefix: " << fConfig.spectrumPrefix << '\n'
        << "spectrum_depth_mm: " << fConfig.spectrumDepth / mm << '\n'
        << "other_ion_ZA: " << fConfig.otherIonZ << ' ' << fConfig.otherIonA << " (0 0: skipped)\n";
  out << "site_diameter_um: " << fConfig.siteDiameter / um << '\n'
      << "mean_chord_um: " << fMeanChord << '\n'
      << "segment_um (buildup/zone/exit/halfWidth): " << fConfig.buildupLength / um << " / "
      << fConfig.zoneLength / um << " / " << fConfig.exitLength / um << " / "
      << fConfig.halfWidth / um << '\n'
      << "samples_per_event: " << fConfig.samplesPerEvent << '\n'
      << "point_spacing_nm: " << fConfig.pointSpacing / nm << '\n'
      << "y0_keV_per_um: " << fConfig.y0 << '\n'
      << "dose_fraction_outside_y_axis: " << (b > 0. ? fOutOfRange / b : 0.) << '\n'
      << "yF_keV_per_um: " << yF << " +- " << sF << '\n'
      << "yD_keV_per_um: " << yD << " +- " << sD << '\n'
      << "ystar_keV_per_um: " << yStar << " +- " << sStar << '\n'
      << "by_species (events, dose_fraction, yF, yD, ystar):\n";
  for (G4int is = 0; is < kNSourceSpecies; ++is) {
    const auto& sums = fSpeciesSums[is];
    if (fSpeciesEvents[is] == 0.) continue;
    out << "  " << SourceName(is) << ": " << fSpeciesEvents[is] << ", "
        << (b > 0. ? sums[1] / b : 0.) << ", " << (sums[2] > 0. ? sums[1] / sums[2] : 0.) << ", "
        << (sums[1] > 0. ? sums[0] / sums[1] : 0.) << ", "
        << (sums[1] > 0. ? y02 * sums[3] / sums[1] : 0.) << '\n';
  }
}
