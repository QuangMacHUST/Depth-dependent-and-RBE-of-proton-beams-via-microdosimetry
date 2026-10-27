#ifndef Species_h
#define Species_h 1

#include "globals.hh"

class G4ParticleDefinition;

// Particle classes used for dose decomposition, LET groups and spectra.
// The order matters: the first kNSpectrumSpecies get fluence spectra and
// everything up to kOtherHadron counts as a charged hadron/ion for LET.
enum Species : G4int
{
  kPrimaryProton = 0,
  kSecondaryProton,
  kDeuteron,
  kTriton,
  kHe3,
  kAlpha,
  kOtherIon,  // Z > 2 recoils and other nuclei
  kOtherHadron,  // charged pions, kaons, ...
  kElectron,  // e- and e+
  kOther,  // neutral particles
  kNSpecies
};

constexpr G4int kNSpectrumSpecies = kOtherIon + 1;

const char* SpeciesName(G4int species);

Species ClassifyParticle(const G4ParticleDefinition* def, G4int trackID);

#endif
