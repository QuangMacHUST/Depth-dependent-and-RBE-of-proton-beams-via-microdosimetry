#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "StoppingPowerTable.hh"

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class DetectorConstruction;
class EventAction;
class RunAction;

class SteppingAction : public G4UserSteppingAction
{
  public:
    SteppingAction(const DetectorConstruction* detector, RunAction* runAction,
                   EventAction* eventAction);

    void UserSteppingAction(const G4Step* step) override;

  private:
    const DetectorConstruction* fDetector = nullptr;
    RunAction* fRunAction = nullptr;
    EventAction* fEventAction = nullptr;
    StoppingPowerTable fStoppingPower;
};

#endif
