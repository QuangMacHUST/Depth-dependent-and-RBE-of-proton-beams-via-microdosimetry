#include "DetectorMessenger.hh"

#include "DetectorConstruction.hh"

#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"

DetectorMessenger::DetectorMessenger(DetectorConstruction* detector) : fDetector(detector)
{
  fDir = new G4UIdirectory("/rbe/det/");
  fDir->SetGuidance("Phantom geometry (set before /run/initialize)");

  fDepthCmd = new G4UIcmdWithADoubleAndUnit("/rbe/det/depth", this);
  fDepthCmd->SetGuidance("Phantom thickness along the beam axis");
  fDepthCmd->SetParameterName("depth", false);
  fDepthCmd->SetRange("depth>0.");
  fDepthCmd->SetUnitCategory("Length");

  fWidthCmd = new G4UIcmdWithADoubleAndUnit("/rbe/det/width", this);
  fWidthCmd->SetGuidance("Phantom lateral size (square cross-section)");
  fWidthCmd->SetParameterName("width", false);
  fWidthCmd->SetRange("width>0.");
  fWidthCmd->SetUnitCategory("Length");

  fMaxStepCmd = new G4UIcmdWithADoubleAndUnit("/rbe/det/maxStep", this);
  fMaxStepCmd->SetGuidance("Maximum step of charged particles inside the phantom");
  fMaxStepCmd->SetParameterName("step", false);
  fMaxStepCmd->SetRange("step>0.");
  fMaxStepCmd->SetUnitCategory("Length");

  fMaterialCmd = new G4UIcmdWithAString("/rbe/det/material", this);
  fMaterialCmd->SetGuidance("Phantom material (NIST name, default G4_WATER)");
  fMaterialCmd->SetParameterName("material", false);

  G4UIcommand* commands[] = {fDepthCmd, fWidthCmd, fMaxStepCmd, fMaterialCmd};
  for (G4UIcommand* cmd : commands) {
    cmd->AvailableForStates(G4State_PreInit);
    cmd->SetToBeBroadcasted(false);
  }
}

DetectorMessenger::~DetectorMessenger()
{
  delete fDepthCmd;
  delete fWidthCmd;
  delete fMaxStepCmd;
  delete fMaterialCmd;
  delete fDir;
}

void DetectorMessenger::SetNewValue(G4UIcommand* cmd, G4String value)
{
  if (cmd == fDepthCmd) fDetector->SetPhantomDepth(fDepthCmd->GetNewDoubleValue(value));
  else if (cmd == fWidthCmd) fDetector->SetPhantomWidth(fWidthCmd->GetNewDoubleValue(value));
  else if (cmd == fMaxStepCmd) fDetector->SetMaxStep(fMaxStepCmd->GetNewDoubleValue(value));
  else if (cmd == fMaterialCmd) fDetector->SetMaterial(value);
}
