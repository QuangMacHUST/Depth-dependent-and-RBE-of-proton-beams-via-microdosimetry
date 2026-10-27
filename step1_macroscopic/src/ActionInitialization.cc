#include "ActionInitialization.hh"

#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"

void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new RunAction(fDetector));
}

void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction());
  auto runAction = new RunAction(fDetector);
  SetUserAction(runAction);
  auto eventAction = new EventAction(runAction);
  SetUserAction(eventAction);
  SetUserAction(new SteppingAction(fDetector, runAction, eventAction));
}
