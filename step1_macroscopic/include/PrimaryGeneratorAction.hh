#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"

class G4GeneralParticleSource;

// Beam defined entirely from macros with /gps/... ; several weighted GPS
// sources (/gps/source/add) give a spread-out Bragg peak.
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event* event) override;

  private:
    G4GeneralParticleSource* fSource = nullptr;
};

#endif
