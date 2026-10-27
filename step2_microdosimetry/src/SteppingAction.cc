#include "SteppingAction.hh"

#include "EventAction.hh"

#include "G4Step.hh"
#include "G4VProcess.hh"

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  const G4double edep = step->GetTotalEnergyDeposit();
  if (edep <= 0.) return;

  // Geant4-DNA processes deposit at the interaction point (post-step).
  // Standard (condensed-history) models, used above the DNA limits, lose
  // energy continuously along the step, which is then spread along it.
  const G4VProcess* process = step->GetPostStepPoint()->GetProcessDefinedStep();
  const G4bool discrete = process && process->GetProcessName().find("DNA") != std::string::npos;

  fEventAction->AddDeposit(step->GetPreStepPoint()->GetPosition(),
                           step->GetPostStepPoint()->GetPosition(), edep, discrete);
}
