#include "RunAction.hh"

#include "MicroConfig.hh"
#include "PrimaryGeneratorAction.hh"
#include "Run.hh"

G4Run* RunAction::GenerateRun()
{
  fRun = new Run(MicroConfig::Instance());
  return fRun;
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  if (fPrimary) fPrimary->PrepareRun();
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  // Worker runs have already been merged into the master run here
  if (IsMaster() && run->GetNumberOfEvent() > 0) fRun->Write();
}
