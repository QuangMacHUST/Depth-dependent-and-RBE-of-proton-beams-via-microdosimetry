#include "ScoringConfig.hh"

#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIdirectory.hh"

ScoringConfig& ScoringConfig::Instance()
{
  static ScoringConfig instance;
  return instance;
}

namespace
{
// Commands only touch the shared config on the master: never broadcast
template <class Cmd>
void Setup(Cmd* cmd, const char* guidance)
{
  cmd->SetGuidance(guidance);
  cmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  cmd->SetToBeBroadcasted(false);
}
}  // namespace

ScoringMessenger::ScoringMessenger()
{
  fRbeDir = new G4UIdirectory("/rbe/");
  fRbeDir->SetGuidance("Proton depth / LET / spectrum scoring");
  fScoreDir = new G4UIdirectory("/rbe/score/");
  fScoreDir->SetGuidance("Scoring grid settings (applied at next /run/beamOn)");
  fOutputDir = new G4UIdirectory("/rbe/output/");
  fOutputDir->SetGuidance("Output settings");

  fDepthBinsCmd = new G4UIcmdWithAnInteger("/rbe/score/depthBins", this);
  Setup(fDepthBinsCmd, "Number of depth bins for dose and LET");
  fDepthBinsCmd->SetParameterName("n", false);
  fDepthBinsCmd->SetRange("n>0");

  fDepthMaxCmd = new G4UIcmdWithADoubleAndUnit("/rbe/score/depthMax", this);
  Setup(fDepthMaxCmd, "Maximum scored depth (<=0: phantom depth)");
  fDepthMaxCmd->SetParameterName("zmax", false);
  fDepthMaxCmd->SetUnitCategory("Length");

  fSpecDepthBinsCmd = new G4UIcmdWithAnInteger("/rbe/score/spectrumDepthBins", this);
  Setup(fSpecDepthBinsCmd, "Number of depth bins for fluence spectra");
  fSpecDepthBinsCmd->SetParameterName("n", false);
  fSpecDepthBinsCmd->SetRange("n>0");

  fEMinCmd = new G4UIcmdWithADoubleAndUnit("/rbe/score/eMin", this);
  Setup(fEMinCmd, "Lower edge of the (log) spectrum energy axis");
  fEMinCmd->SetParameterName("emin", false);
  fEMinCmd->SetUnitCategory("Energy");

  fEMaxCmd = new G4UIcmdWithADoubleAndUnit("/rbe/score/eMax", this);
  Setup(fEMaxCmd, "Upper edge of the (log) spectrum energy axis");
  fEMaxCmd->SetParameterName("emax", false);
  fEMaxCmd->SetUnitCategory("Energy");

  fEBinsCmd = new G4UIcmdWithAnInteger("/rbe/score/eBins", this);
  Setup(fEBinsCmd, "Number of (log) spectrum energy bins");
  fEBinsCmd->SetParameterName("n", false);
  fEBinsCmd->SetRange("n>0");

  fRadiusCmd = new G4UIcmdWithADoubleAndUnit("/rbe/score/radius", this);
  Setup(fRadiusCmd, "Score only within this radius of the beam axis (<=0: whole slab)");
  fRadiusCmd->SetParameterName("r", false);
  fRadiusCmd->SetUnitCategory("Length");

  fPrefixCmd = new G4UIcmdWithAString("/rbe/output/prefix", this);
  Setup(fPrefixCmd, "Output file prefix (may contain a directory)");
  fPrefixCmd->SetParameterName("prefix", false);
}

ScoringMessenger::~ScoringMessenger()
{
  delete fDepthBinsCmd;
  delete fDepthMaxCmd;
  delete fSpecDepthBinsCmd;
  delete fEMinCmd;
  delete fEMaxCmd;
  delete fEBinsCmd;
  delete fRadiusCmd;
  delete fPrefixCmd;
  delete fOutputDir;
  delete fScoreDir;
  delete fRbeDir;
}

void ScoringMessenger::SetNewValue(G4UIcommand* cmd, G4String value)
{
  auto& cfg = ScoringConfig::Instance();
  if (cmd == fDepthBinsCmd) cfg.nDepthBins = fDepthBinsCmd->GetNewIntValue(value);
  else if (cmd == fDepthMaxCmd) cfg.depthMax = fDepthMaxCmd->GetNewDoubleValue(value);
  else if (cmd == fSpecDepthBinsCmd) cfg.nSpectrumDepthBins = fSpecDepthBinsCmd->GetNewIntValue(value);
  else if (cmd == fEMinCmd) cfg.eMin = fEMinCmd->GetNewDoubleValue(value);
  else if (cmd == fEMaxCmd) cfg.eMax = fEMaxCmd->GetNewDoubleValue(value);
  else if (cmd == fEBinsCmd) cfg.nEnergyBins = fEBinsCmd->GetNewIntValue(value);
  else if (cmd == fRadiusCmd) cfg.scoringRadius = fRadiusCmd->GetNewDoubleValue(value);
  else if (cmd == fPrefixCmd) cfg.outputPrefix = value;
}
