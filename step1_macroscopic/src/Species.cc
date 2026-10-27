#include "Species.hh"

#include "G4Alpha.hh"
#include "G4Deuteron.hh"
#include "G4Electron.hh"
#include "G4He3.hh"
#include "G4ParticleDefinition.hh"
#include "G4Positron.hh"
#include "G4Proton.hh"
#include "G4Triton.hh"

const char* SpeciesName(G4int species)
{
  static const char* names[kNSpecies] = {"proton_primary", "proton_secondary", "deuteron",
                                         "triton", "he3", "alpha", "ion_other",
                                         "hadron_other", "electron", "other"};
  return (species >= 0 && species < kNSpecies) ? names[species] : "unknown";
}

Species ClassifyParticle(const G4ParticleDefinition* def, G4int trackID)
{
  if (def == G4Proton::Definition()) return trackID == 1 ? kPrimaryProton : kSecondaryProton;
  if (def == G4Deuteron::Definition()) return kDeuteron;
  if (def == G4Triton::Definition()) return kTriton;
  if (def == G4He3::Definition()) return kHe3;
  if (def == G4Alpha::Definition()) return kAlpha;
  if (def->GetParticleType() == "nucleus") return kOtherIon;
  if (def == G4Electron::Definition() || def == G4Positron::Definition()) return kElectron;
  if (def->GetPDGCharge() != 0. && def->GetParticleType() != "lepton") return kOtherHadron;
  return kOther;
}
