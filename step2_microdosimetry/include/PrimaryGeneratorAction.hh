#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "SpectrumSource.hh"

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

class G4ParticleDefinition;
class G4ParticleGun;

// One primary per event, starting at (0, 0, -buildup) along +z.
//  - gun mode: particle and energy from /gun/particle and /gun/energy
//  - spectrum mode: drawn from the step-1 fluence spectra at one depth.
//    Geant4-DNA has no models for d, t and 3He, so they are replaced by the
//    proton (d, t) or alpha (3He) of the same velocity, E' = E * m' / m.
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    // Re-reads the configuration; called at the start of each run
    void PrepareRun();
    void GeneratePrimaries(G4Event* event) override;

    G4int LastSpecies() const { return fLastSpecies; }
    G4double LastWeight() const { return fLastWeight; }

  private:
    G4ParticleGun* fGun = nullptr;
    SpectrumSource fSpectrum;
    G4bool fUseSpectrum = false;
    G4double fStartZ = 0.;
    G4ParticleDefinition* fOtherIon = nullptr;
    G4int fLastSpecies = kSourceGun;
    G4double fLastWeight = 1.;
};

#endif
