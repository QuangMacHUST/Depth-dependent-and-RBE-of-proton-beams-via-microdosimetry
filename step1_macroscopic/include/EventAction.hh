#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

#include <vector>

class RunAction;

// Buffers the energy deposited per depth bin during one event so that the
// run can accumulate event-wise sums of squares (statistical uncertainty).
class EventAction : public G4UserEventAction
{
  public:
    explicit EventAction(RunAction* runAction);

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    void AddEdep(G4int bin, G4double edep)
    {
      if (fEdep[bin] == 0.) fTouched.push_back(bin);
      fEdep[bin] += edep;
    }

  private:
    RunAction* fRunAction = nullptr;
    std::vector<G4double> fEdep;
    std::vector<G4int> fTouched;
};

#endif
