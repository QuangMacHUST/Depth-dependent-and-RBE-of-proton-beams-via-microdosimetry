#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"

class PrimaryGeneratorAction;
class Run;

class RunAction : public G4UserRunAction
{
  public:
    // primary is null on the master thread
    explicit RunAction(PrimaryGeneratorAction* primary = nullptr) : fPrimary(primary) {}

    G4Run* GenerateRun() override;
    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    Run* GetRun() const { return fRun; }

  private:
    PrimaryGeneratorAction* fPrimary = nullptr;
    Run* fRun = nullptr;
};

#endif
