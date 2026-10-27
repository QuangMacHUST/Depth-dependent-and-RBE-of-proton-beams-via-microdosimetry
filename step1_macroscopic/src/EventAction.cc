#include "EventAction.hh"

#include "Run.hh"
#include "RunAction.hh"

EventAction::EventAction(RunAction* runAction) : fRunAction(runAction) {}

void EventAction::BeginOfEventAction(const G4Event*)
{
  // The number of bins may change between runs
  const std::size_t nBins = fRunAction->GetRun()->NDoseBins();
  if (fEdep.size() != nBins) {
    fEdep.assign(nBins, 0.);
    fTouched.clear();
  }
}

void EventAction::EndOfEventAction(const G4Event*)
{
  fRunAction->GetRun()->AddEventEdep(fEdep, fTouched);
  for (G4int bin : fTouched) fEdep[bin] = 0.;
  fTouched.clear();
}
