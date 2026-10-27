#ifndef EventAction_h
#define EventAction_h 1

#include "SiteSampler.hh"

#include "G4ThreeVector.hh"
#include "G4UserEventAction.hh"
#include "globals.hh"

#include <vector>

class PrimaryGeneratorAction;
class RunAction;

// Collects the energy-transfer points of one event (in um and keV) and, at
// the end of the event, places sites around them (SiteSampler).
class EventAction : public G4UserEventAction
{
  public:
    EventAction(RunAction* runAction, const PrimaryGeneratorAction* primary);

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    void AddDeposit(const G4ThreeVector& pre, const G4ThreeVector& post, G4double edep,
                    G4bool atPostStep);

  private:
    void Store(const G4ThreeVector& point, G4double edep);

    RunAction* fRunAction = nullptr;
    const PrimaryGeneratorAction* fPrimary = nullptr;
    std::vector<TransferPoint> fPoints;
    G4double fKeepZLow = 0.;  // only points that can fall in a site centred
    G4double fKeepZHigh = 0.;  // in the scoring zone are kept
    G4double fSpacing = 0.;
};

#endif
