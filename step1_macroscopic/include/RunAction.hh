#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"

class DetectorConstruction;
class Run;

class RunAction : public G4UserRunAction
{
  public:
    explicit RunAction(const DetectorConstruction* detector);

    G4Run* GenerateRun() override;
    void EndOfRunAction(const G4Run*) override;

    Run* GetRun() const { return fRun; }

  private:
    const DetectorConstruction* fDetector = nullptr;
    Run* fRun = nullptr;
};

#endif
