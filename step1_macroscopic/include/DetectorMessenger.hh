#ifndef DetectorMessenger_h
#define DetectorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class DetectorConstruction;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAString;
class G4UIdirectory;

class DetectorMessenger : public G4UImessenger
{
  public:
    explicit DetectorMessenger(DetectorConstruction* detector);
    ~DetectorMessenger() override;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:
    DetectorConstruction* fDetector = nullptr;
    G4UIdirectory* fDir = nullptr;
    G4UIcmdWithADoubleAndUnit* fDepthCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fWidthCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fMaxStepCmd = nullptr;
    G4UIcmdWithAString* fMaterialCmd = nullptr;
};

#endif
