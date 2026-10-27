#include "PrimaryGeneratorAction.hh"

#include "G4GeneralParticleSource.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction() : fSource(new G4GeneralParticleSource()) {}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fSource;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  fSource->GeneratePrimaryVertex(event);
}
