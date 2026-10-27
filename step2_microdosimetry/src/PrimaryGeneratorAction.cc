#include "PrimaryGeneratorAction.hh"

#include "MicroConfig.hh"

#include "G4Alpha.hh"
#include "G4Deuteron.hh"
#include "G4He3.hh"
#include "G4IonTable.hh"
#include "G4ParticleGun.hh"
#include "G4Proton.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Triton.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction() : fGun(new G4ParticleGun(1))
{
  fGun->SetParticleDefinition(G4Proton::Definition());
  fGun->SetParticleEnergy(10. * MeV);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fGun;
}

void PrimaryGeneratorAction::PrepareRun()
{
  const auto& cfg = MicroConfig::Instance();
  fStartZ = -cfg.buildupLength;
  fUseSpectrum = (cfg.sourceMode == "spectrum");
  if (!fUseSpectrum) return;

  fSpectrum.Load(cfg);
  fOtherIon = cfg.otherIonZ > 0 ? G4IonTable::GetIonTable()->GetIon(cfg.otherIonZ, cfg.otherIonA, 0.)
                                : nullptr;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  if (fUseSpectrum) {
    const SpectrumSource::Sample sample = fSpectrum.Draw();
    const G4double mp = G4Proton::Definition()->GetPDGMass();
    const G4double ma = G4Alpha::Definition()->GetPDGMass();
    G4ParticleDefinition* particle = G4Proton::Definition();
    G4double energy = sample.energy;
    switch (sample.species) {
      case kSourceDeuteron:
        energy *= mp / G4Deuteron::Definition()->GetPDGMass();
        break;
      case kSourceTriton:
        energy *= mp / G4Triton::Definition()->GetPDGMass();
        break;
      case kSourceHe3:
        particle = G4Alpha::Definition();
        energy *= ma / G4He3::Definition()->GetPDGMass();
        break;
      case kSourceAlpha:
        particle = G4Alpha::Definition();
        break;
      case kSourceOtherIon:
        particle = fOtherIon;
        break;
      default:
        break;
    }
    fGun->SetParticleDefinition(particle);
    fGun->SetParticleEnergy(energy);
    fLastSpecies = sample.species;
    fLastWeight = sample.weight;
  }
  else {
    fLastSpecies = kSourceGun;
    fLastWeight = 1.;
  }

  fGun->SetParticlePosition(G4ThreeVector(0., 0., fStartZ));
  fGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
  fGun->GeneratePrimaryVertex(event);
}
