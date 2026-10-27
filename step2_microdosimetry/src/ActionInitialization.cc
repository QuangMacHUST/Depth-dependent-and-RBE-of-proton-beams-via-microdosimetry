#include "ActionInitialization.hh"

#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"

void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
  auto primary = new PrimaryGeneratorAction();
  SetUserAction(primary);
  auto runAction = new RunAction(primary);
  SetUserAction(runAction);
  auto eventAction = new EventAction(runAction, primary);
  SetUserAction(eventAction);
  SetUserAction(new SteppingAction(eventAction));
}
