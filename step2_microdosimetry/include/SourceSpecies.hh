#ifndef SourceSpecies_h
#define SourceSpecies_h 1

#include "globals.hh"

// Primary particle classes. The first six match the fluence-spectrum files of
// step 1 (<prefix>_spectrum_<name>.csv); kSourceGun is a particle set with /gun/.
enum SourceSpecies : G4int
{
  kSourceProton = 0,
  kSourceDeuteron,
  kSourceTriton,
  kSourceHe3,
  kSourceAlpha,
  kSourceOtherIon,
  kSourceGun,
  kNSourceSpecies
};

constexpr G4int kNSpectrumSources = kSourceGun;

inline const char* SourceName(G4int species)
{
  static const char* names[kNSourceSpecies] = {"proton", "deuteron", "triton", "he3",
                                               "alpha", "ion_other", "gun"};
  return (species >= 0 && species < kNSourceSpecies) ? names[species] : "unknown";
}

// Step-1 spectrum file suffix for each spectrum source
inline const char* SourceFileName(G4int species)
{
  static const char* names[kNSpectrumSources] = {"proton_all", "deuteron", "triton",
                                                 "he3", "alpha", "ion_other"};
  return (species >= 0 && species < kNSpectrumSources) ? names[species] : "unknown";
}

#endif
