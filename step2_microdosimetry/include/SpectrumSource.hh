#ifndef SpectrumSource_h
#define SpectrumSource_h 1

#include "MicroConfig.hh"

#include "globals.hh"

#include <vector>

// Samples (species, kinetic energy) from the step-1 track-length fluence
// spectra at one depth. Bins are drawn with probability proportional to
// Phi(E) dE (times an optional per-species bias); the energy is log-uniform
// inside the bin. Equal segment lengths per primary then reproduce the
// track-length weighting of the field.
class SpectrumSource
{
  public:
    struct Sample
    {
      G4int species;
      G4double energy;  // total kinetic energy of the step-1 particle
      G4double weight;  // corrects for the bias, mean 1
    };

    void Load(const MicroConfig& config);
    G4bool IsLoaded() const { return !fCumulative.empty(); }
    Sample Draw() const;

  private:
    struct Bin
    {
      G4int species;
      G4double eLow, eHigh;
    };

    std::vector<Bin> fBins;
    std::vector<G4double> fCumulative;
    G4double fWeights[kNSourceSpecies] = {};
};

#endif
