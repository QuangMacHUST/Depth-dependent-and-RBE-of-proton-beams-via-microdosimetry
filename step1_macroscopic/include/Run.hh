#ifndef Run_h
#define Run_h 1

#include "Axes.hh"
#include "ScoringConfig.hh"
#include "Species.hh"

#include "G4Run.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

#include <vector>

class DetectorConstruction;

// Per-thread accumulators, merged into the master run at the end.
//
//  - dose per depth bin (event-by-event sums for the statistical uncertainty)
//    and its decomposition by particle species
//  - LET_t / LET_d moments (sum l, sum S*l, sum S^2*l) per depth bin for three
//    groups: primary protons, all protons, all charged hadrons and ions
//  - track-length fluence spectra dPhi/dE per depth bin and species
class Run : public G4Run
{
  public:
    enum LetGroup : G4int { kLetPrimaryProton = 0, kLetAllProton, kLetAllCharged, kNLetGroups };

    Run(const DetectorConstruction* detector, const ScoringConfig& config);

    void Merge(const G4Run* other) override;

    G4bool InScoringRegion(const G4ThreeVector& point) const;
    G4int DoseBin(G4double z) const { return fDoseAxis.Index(z); }
    G4int NDoseBins() const { return fDoseAxis.n; }

    void AddSpeciesEdep(G4int bin, G4int species, G4double edep);
    void AddEventEdep(const std::vector<G4double>& edep, const std::vector<G4int>& bins);

    // Straight charged-particle segment from z1 to z2 with true length
    // `length`, mean kinetic energy `ekin` and electronic stopping power `dedx`
    void AddTrackSegment(G4int species, G4double z1, G4double z2, G4double length,
                         G4double ekin, G4double dedx);

    void Write() const;

  private:
    G4double SlabArea() const;
    void WriteDepthFile(const G4String& path) const;
    void WriteSpectrumFile(const G4String& path, const std::vector<G4int>& species) const;
    void WriteSummary(const G4String& path) const;

    const DetectorConstruction* fDetector = nullptr;
    ScoringConfig fConfig;
    LinearAxis fDoseAxis;
    LinearAxis fSpectrumAxis;
    LogAxis fEnergyAxis;

    std::vector<G4double> fEdepSum;  // [iz]
    std::vector<G4double> fEdepSum2;  // [iz], event-wise squares
    std::vector<G4double> fSpeciesEdep;  // [iz * kNSpecies + s]
    std::vector<G4double> fLet;  // [(g * nz + iz) * 3 + moment]
    std::vector<G4double> fTrackLength;  // [(s * nzSpec + iz) * nE + ie]
};

#endif
