#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"

class EventAction;

// Passes every energy deposit to the event as transfer points.
class SteppingAction : public G4UserSteppingAction
{
  public:
    explicit SteppingAction(EventAction* eventAction) : fEventAction(eventAction) {}

    void UserSteppingAction(const G4Step* step) override;

  private:
    EventAction* fEventAction = nullptr;
};

#endif
