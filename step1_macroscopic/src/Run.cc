#include "Run.hh"

#include "DetectorConstruction.hh"

#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include "G4Version.hh"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace
{
const char* kLetGroupNames[Run::kNLetGroups] = {"proton_primary", "proton_all", "charged_all"};

void AddInto(std::vector<G4double>& target, const std::vector<G4double>& source)
{
  for (std::size_t i = 0; i < target.size(); ++i) target[i] += source[i];
}
}  // namespace

Run::Run(const DetectorConstruction* detector, const ScoringConfig& config)
  : fDetector(detector), fConfig(config)
{
  const G4double zMax = fConfig.depthMax > 0. ? fConfig.depthMax : detector->GetPhantomDepth();
  fDoseAxis = {0., zMax, fConfig.nDepthBins};
  fSpectrumAxis = {0., zMax, fConfig.nSpectrumDepthBins};
  fEnergyAxis = {fConfig.eMin, fConfig.eMax, fConfig.nEnergyBins};

  const std::size_t nz = fDoseAxis.n;
  fEdepSum.assign(nz, 0.);
  fEdepSum2.assign(nz, 0.);
  fSpeciesEdep.assign(nz * kNSpecies, 0.);
  fLet.assign(kNLetGroups * nz * 3, 0.);
  fTrackLength.assign(static_cast<std::size_t>(kNSpectrumSpecies) * fSpectrumAxis.n * fEnergyAxis.n,
                      0.);
}

void Run::Merge(const G4Run* other)
{
  const auto run = static_cast<const Run*>(other);
  AddInto(fEdepSum, run->fEdepSum);
  AddInto(fEdepSum2, run->fEdepSum2);
  AddInto(fSpeciesEdep, run->fSpeciesEdep);
  AddInto(fLet, run->fLet);
  AddInto(fTrackLength, run->fTrackLength);
  G4Run::Merge(other);
}

G4bool Run::InScoringRegion(const G4ThreeVector& point) const
{
  if (fConfig.scoringRadius <= 0.) return true;
  return point.perp2() < fConfig.scoringRadius * fConfig.scoringRadius;
}

void Run::AddSpeciesEdep(G4int bin, G4int species, G4double edep)
{
  fSpeciesEdep[bin * kNSpecies + species] += edep;
}

void Run::AddEventEdep(const std::vector<G4double>& edep, const std::vector<G4int>& bins)
{
  for (G4int iz : bins) {
    fEdepSum[iz] += edep[iz];
    fEdepSum2[iz] += edep[iz] * edep[iz];
  }
}

void Run::AddTrackSegment(G4int species, G4double z1, G4double z2, G4double length, G4double ekin,
                          G4double dedx)
{
  const G4bool inGroup[kNLetGroups] = {species == kPrimaryProton, species <= kSecondaryProton,
                                       species <= kOtherHadron};
  const std::size_t nz = fDoseAxis.n;
  fDoseAxis.ForEachOverlap(z1, z2, [&](G4int iz, G4double fraction) {
    const G4double l = fraction * length;
    for (G4int ig = 0; ig < kNLetGroups; ++ig) {
      if (!inGroup[ig]) continue;
      G4double* mom = &fLet[(ig * nz + iz) * 3];
      mom[0] += l;
      mom[1] += dedx * l;
      mom[2] += dedx * dedx * l;
    }
  });

  if (species >= kNSpectrumSpecies) return;
  const G4int ie = fEnergyAxis.Index(ekin);
  if (ie < 0) return;
  const std::size_t nzs = fSpectrumAxis.n;
  const std::size_t ne = fEnergyAxis.n;
  fSpectrumAxis.ForEachOverlap(z1, z2, [&](G4int iz, G4double fraction) {
    fTrackLength[(species * nzs + iz) * ne + ie] += fraction * length;
  });
}

G4double Run::SlabArea() const
{
  if (fConfig.scoringRadius > 0.) return CLHEP::pi * fConfig.scoringRadius * fConfig.scoringRadius;
  return fDetector->GetPhantomWidth() * fDetector->GetPhantomWidth();
}

void Run::Write() const
{
  const G4String& prefix = fConfig.outputPrefix;
  const std::filesystem::path prefixPath(prefix.c_str());
  if (prefixPath.has_parent_path()) std::filesystem::create_directories(prefixPath.parent_path());

  WriteDepthFile(prefix + "_depth.csv");
  for (G4int is = 0; is < kNSpectrumSpecies; ++is)
    WriteSpectrumFile(prefix + "_spectrum_" + SpeciesName(is) + ".csv", {is});
  WriteSpectrumFile(prefix + "_spectrum_proton_all.csv", {kPrimaryProton, kSecondaryProton});
  WriteSummary(prefix + "_summary.txt");

  G4cout << "\n Run " << runID << ": " << numberOfEvent << " primaries, output written to "
         << prefix << "_*.csv\n"
         << G4endl;
}

void Run::WriteDepthFile(const G4String& path) const
{
  const G4double nEvents = numberOfEvent;
  const G4double mass = fDetector->GetPhantomMaterial()->GetDensity() * SlabArea() * fDoseAxis.Width();
  const G4double toGyPerPrimary = 1. / (mass * nEvents * gray);
  const G4double letUnit = keV / um;
  const std::size_t nz = fDoseAxis.n;

  std::ofstream out(path);
  out << "# Slab-integrated depth dose (Gy per primary) and LET (keV/um)\n"
      << "# LET: unrestricted electronic stopping power; t = track-length averaged, "
         "d = dose averaged\n"
      << "z_lo_mm,z_hi_mm,z_mid_mm,dose_Gy,dose_rel_unc";
  for (G4int is = 0; is < kNSpecies; ++is) out << ",dose_" << SpeciesName(is) << "_Gy";
  for (const char* name : kLetGroupNames) out << ",LETt_" << name << ",LETd_" << name;
  out << '\n' << std::setprecision(7);

  for (std::size_t iz = 0; iz < nz; ++iz) {
    const G4double mean = fEdepSum[iz] / nEvents;
    const G4double variance = fEdepSum2[iz] / nEvents - mean * mean;
    const G4double relUnc =
      (mean > 0. && nEvents > 1) ? std::sqrt(std::max(variance, 0.) / (nEvents - 1)) / mean : 0.;
    const G4double zlo = fDoseAxis.Edge(iz);
    const G4double zhi = fDoseAxis.Edge(iz + 1);

    out << zlo / mm << ',' << zhi / mm << ',' << 0.5 * (zlo + zhi) / mm << ','
        << fEdepSum[iz] * toGyPerPrimary << ',' << relUnc;
    for (G4int is = 0; is < kNSpecies; ++is)
      out << ',' << fSpeciesEdep[iz * kNSpecies + is] * toGyPerPrimary;
    for (G4int ig = 0; ig < kNLetGroups; ++ig) {
      const G4double* mom = &fLet[(ig * nz + iz) * 3];
      const G4double letT = mom[0] > 0. ? mom[1] / mom[0] : 0.;
      const G4double letD = mom[1] > 0. ? mom[2] / mom[1] : 0.;
      out << ',' << letT / letUnit << ',' << letD / letUnit;
    }
    out << '\n';
  }
}

void Run::WriteSpectrumFile(const G4String& path, const std::vector<G4int>& species) const
{
  const std::size_t nzs = fSpectrumAxis.n;
  const std::size_t ne = fEnergyAxis.n;
  // track length / volume = fluence; expressed in cm^-2 per primary
  const G4double norm = cm2 / (SlabArea() * fSpectrumAxis.Width() * numberOfEvent);

  std::ofstream out(path);
  out << "# Track-length fluence spectrum dPhi/dE per primary (cm^-2 MeV^-1), species:";
  for (G4int is : species) out << ' ' << SpeciesName(is);
  out << "\n# rows: depth bins; columns: total kinetic energy bins (log), header = bin centre\n"
      << "# E_edges_MeV";
  out << std::setprecision(7);
  for (std::size_t ie = 0; ie <= ne; ++ie) out << ',' << fEnergyAxis.Edge(ie) / MeV;
  out << "\nz_lo_mm,z_hi_mm";
  for (std::size_t ie = 0; ie < ne; ++ie)
    out << ',' << std::sqrt(fEnergyAxis.Edge(ie) * fEnergyAxis.Edge(ie + 1)) / MeV;
  out << '\n';

  for (std::size_t iz = 0; iz < nzs; ++iz) {
    out << fSpectrumAxis.Edge(iz) / mm << ',' << fSpectrumAxis.Edge(iz + 1) / mm;
    for (std::size_t ie = 0; ie < ne; ++ie) {
      G4double length = 0.;
      for (G4int is : species) length += fTrackLength[(is * nzs + iz) * ne + ie];
      const G4double dE = (fEnergyAxis.Edge(ie + 1) - fEnergyAxis.Edge(ie)) / MeV;
      out << ',' << length * norm / dE;
    }
    out << '\n';
  }
}

void Run::WriteSummary(const G4String& path) const
{
  G4double totalEdep = 0.;
  std::vector<G4double> speciesEdep(kNSpecies, 0.);
  for (std::size_t iz = 0; iz < fEdepSum.size(); ++iz) {
    totalEdep += fEdepSum[iz];
    for (G4int is = 0; is < kNSpecies; ++is) speciesEdep[is] += fSpeciesEdep[iz * kNSpecies + is];
  }

  std::ofstream out(path);
  out << "geant4_version: " << G4Version << '\n'
      << "physics_list: " << fConfig.physicsList << '\n'
      << "primaries: " << numberOfEvent << '\n'
      << "phantom_material: " << fDetector->GetPhantomMaterial()->GetName() << '\n'
      << "phantom_depth_mm: " << fDetector->GetPhantomDepth() / mm << '\n'
      << "phantom_width_mm: " << fDetector->GetPhantomWidth() / mm << '\n'
      << "max_step_mm: " << fDetector->GetMaxStep() / mm << '\n'
      << "scoring_radius_mm: " << fConfig.scoringRadius / mm << " (<=0: whole slab)\n"
      << "dose_bins: " << fDoseAxis.n << " x " << fDoseAxis.Width() / mm << " mm\n"
      << "spectrum_depth_bins: " << fSpectrumAxis.n << " x " << fSpectrumAxis.Width() / mm << " mm\n"
      << "spectrum_energy_bins: " << fEnergyAxis.n << " log from " << fEnergyAxis.lo / MeV << " to "
      << fEnergyAxis.hi / MeV << " MeV\n"
      << "edep_scored_MeV_per_primary: " << totalEdep / MeV / numberOfEvent << '\n'
      << "edep_fraction_by_species:\n";
  for (G4int is = 0; is < kNSpecies; ++is)
    out << "  " << SpeciesName(is) << ": " << (totalEdep > 0. ? speciesEdep[is] / totalEdep : 0.)
        << '\n';
}
