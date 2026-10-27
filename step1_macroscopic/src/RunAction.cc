#include "RunAction.hh"

#include "Run.hh"
#include "ScoringConfig.hh"

RunAction::RunAction(const DetectorConstruction* detector) : fDetector(detector) {}

G4Run* RunAction::GenerateRun()
{
  fRun = new Run(fDetector, ScoringConfig::Instance());
  return fRun;
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  // Worker runs have already been merged into the master run here
  if (IsMaster() && run->GetNumberOfEvent() > 0) fRun->Write();
}
