#include "MicroConfig.hh"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIdirectory.hh"

#include <sstream>

MicroConfig& MicroConfig::Instance()
{
  static MicroConfig instance;
  return instance;
}

namespace
{
// Commands only touch the shared config on the master: never broadcast
template <class Cmd>
Cmd* Make(const char* path, G4UImessenger* messenger, const char* guidance, G4bool preInitOnly)
{
  auto cmd = new Cmd(path, messenger);
  cmd->SetGuidance(guidance);
  if (preInitOnly) cmd->AvailableForStates(G4State_PreInit);
  else cmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  cmd->SetToBeBroadcasted(false);
  return cmd;
}

G4UIcmdWithADoubleAndUnit* MakeLength(const char* path, G4UImessenger* messenger,
                                      const char* guidance, G4bool preInitOnly)
{
  auto cmd = Make<G4UIcmdWithADoubleAndUnit>(path, messenger, guidance, preInitOnly);
  cmd->SetParameterName("value", false);
  cmd->SetRange("value>0.");
  cmd->SetUnitCategory("Length");
  return cmd;
}
}  // namespace

MicroMessenger::MicroMessenger()
{
  const char* dirs[5][2] = {{"/micro/", "Geant4-DNA microdosimetry (step 2)"},
                            {"/micro/det/", "Track-segment geometry (before /run/initialize)"},
                            {"/micro/score/", "Site and lineal-energy scoring"},
                            {"/micro/source/", "Primary particles"},
                            {"/micro/output/", "Output"}};
  for (G4int i = 0; i < 5; ++i) {
    fDirs[i] = new G4UIdirectory(dirs[i][0]);
    fDirs[i]->SetGuidance(dirs[i][1]);
  }

  fBuildupCmd = MakeLength("/micro/det/buildup", this, "Water upstream of the scoring zone", true);
  fZoneCmd = MakeLength("/micro/det/zone", this, "Length of the zone holding site centres", true);
  fExitCmd = MakeLength("/micro/det/exit", this, "Water downstream of the scoring zone", true);
  fHalfWidthCmd = MakeLength("/micro/det/halfWidth", this, "Lateral half-width of the water box", true);

  fSiteCmd = MakeLength("/micro/score/siteDiameter", this, "Diameter of the spherical site", false);
  fSamplesCmd = Make<G4UIcmdWithAnInteger>("/micro/score/samples", this,
                                           "Site placements tried per event", false);
  fSamplesCmd->SetParameterName("n", false);
  fSamplesCmd->SetRange("n>0");
  fSpacingCmd = MakeLength("/micro/score/pointSpacing", this,
                           "Max spacing of points spreading condensed-history energy loss", false);
  fYMinCmd = Make<G4UIcmdWithADouble>("/micro/score/yMin", this, "Lowest y bin edge (keV/um)", false);
  fYMinCmd->SetParameterName("y", false);
  fYMaxCmd = Make<G4UIcmdWithADouble>("/micro/score/yMax", this, "Highest y bin edge (keV/um)", false);
  fYMaxCmd->SetParameterName("y", false);
  fBinsCmd = Make<G4UIcmdWithAnInteger>("/micro/score/binsPerDecade", this,
                                        "Logarithmic y bins per decade", false);
  fBinsCmd->SetParameterName("n", false);
  fBinsCmd->SetRange("n>0");
  fY0Cmd = Make<G4UIcmdWithADouble>("/micro/score/y0", this,
                                    "Saturation parameter y0 of y* (keV/um)", false);
  fY0Cmd->SetParameterName("y0", false);

  fPrefixCmd = Make<G4UIcmdWithAString>("/micro/output/prefix", this, "Output file prefix", false);
  fPrefixCmd->SetParameterName("prefix", false);

  fModeCmd = Make<G4UIcmdWithAString>("/micro/source/mode", this,
                                      "gun: particle from /gun/; spectrum: step-1 fluence spectra", false);
  fModeCmd->SetParameterName("mode", false);
  fModeCmd->SetCandidates("gun spectrum");
  fSpectrumCmd = Make<G4UIcmdWithAString>("/micro/source/spectrumPrefix", this,
                                          "Step-1 output prefix (files <prefix>_spectrum_*.csv)", false);
  fSpectrumCmd->SetParameterName("prefix", false);
  fDepthCmd = MakeLength("/micro/source/depth", this, "Depth at which the spectra are taken", false);
  fOtherIonCmd = Make<G4UIcmdWithAString>("/micro/source/otherIon", this,
                                          "Represent the ion_other spectrum by the ion 'Z A' (0 0: skip)", false);
  fOtherIonCmd->SetParameterName("ZA", false);
  fBiasCmd = Make<G4UIcmdWithAString>("/micro/source/bias", this,
                                      "Oversample a species: '<proton|deuteron|triton|he3|alpha|ion_other> factor'",
                                      false);
  fBiasCmd->SetParameterName("speciesFactor", false);
}

MicroMessenger::~MicroMessenger()
{
  delete fBuildupCmd;
  delete fZoneCmd;
  delete fExitCmd;
  delete fHalfWidthCmd;
  delete fSiteCmd;
  delete fSamplesCmd;
  delete fSpacingCmd;
  delete fYMinCmd;
  delete fYMaxCmd;
  delete fBinsCmd;
  delete fY0Cmd;
  delete fPrefixCmd;
  delete fModeCmd;
  delete fSpectrumCmd;
  delete fDepthCmd;
  delete fOtherIonCmd;
  delete fBiasCmd;
  for (G4int i = 4; i >= 0; --i) delete fDirs[i];
}

void MicroMessenger::SetNewValue(G4UIcommand* cmd, G4String value)
{
  auto& cfg = MicroConfig::Instance();
  if (cmd == fBuildupCmd) cfg.buildupLength = fBuildupCmd->GetNewDoubleValue(value);
  else if (cmd == fZoneCmd) cfg.zoneLength = fZoneCmd->GetNewDoubleValue(value);
  else if (cmd == fExitCmd) cfg.exitLength = fExitCmd->GetNewDoubleValue(value);
  else if (cmd == fHalfWidthCmd) cfg.halfWidth = fHalfWidthCmd->GetNewDoubleValue(value);
  else if (cmd == fSiteCmd) cfg.siteDiameter = fSiteCmd->GetNewDoubleValue(value);
  else if (cmd == fSamplesCmd) cfg.samplesPerEvent = fSamplesCmd->GetNewIntValue(value);
  else if (cmd == fSpacingCmd) cfg.pointSpacing = fSpacingCmd->GetNewDoubleValue(value);
  else if (cmd == fYMinCmd) cfg.yMin = fYMinCmd->GetNewDoubleValue(value);
  else if (cmd == fYMaxCmd) cfg.yMax = fYMaxCmd->GetNewDoubleValue(value);
  else if (cmd == fBinsCmd) cfg.binsPerDecade = fBinsCmd->GetNewIntValue(value);
  else if (cmd == fY0Cmd) cfg.y0 = fY0Cmd->GetNewDoubleValue(value);
  else if (cmd == fPrefixCmd) cfg.outputPrefix = value;
  else if (cmd == fModeCmd) cfg.sourceMode = value;
  else if (cmd == fSpectrumCmd) cfg.spectrumPrefix = value;
  else if (cmd == fDepthCmd) cfg.spectrumDepth = fDepthCmd->GetNewDoubleValue(value);
  else if (cmd == fOtherIonCmd) {
    std::istringstream input(value);
    G4int z = 0, a = 0;
    if (!(input >> z >> a) || z < 0 || a < z) {
      G4cerr << "/micro/source/otherIon: expected 'Z A', got '" << value << "'" << G4endl;
      return;
    }
    cfg.otherIonZ = z;
    cfg.otherIonA = a;
  }
  else if (cmd == fBiasCmd) {
    std::istringstream input(value);
    G4String name;
    G4double factor = 0.;
    if (!(input >> name >> factor) || factor <= 0.) {
      G4cerr << "/micro/source/bias: expected '<species> <factor > 0>', got '" << value << "'"
             << G4endl;
      return;
    }
    for (G4int is = 0; is < kNSpectrumSources; ++is) {
      if (name == SourceName(is)) {
        cfg.bias[is] = factor;
        return;
      }
    }
    G4cerr << "/micro/source/bias: unknown species '" << name << "'" << G4endl;
  }
}
