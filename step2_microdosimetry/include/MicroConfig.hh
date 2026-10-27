#ifndef MicroConfig_h
#define MicroConfig_h 1

#include "SourceSpecies.hh"

#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4UImessenger.hh"
#include "globals.hh"

#include <array>

class G4UIcmdWithADouble;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIdirectory;

// Settings shared by all threads, written only from the master thread
// between runs (via MicroMessenger) and copied by each Run at creation.
struct MicroConfig
{
  // Geometry of the track segment (along +z): the primary starts at
  // z = -buildup, sites are centred in 0 <= z < zone, the water ends at
  // z = zone + exit. Set before /run/initialize.
  G4double buildupLength = 3. * um;
  G4double zoneLength = 2. * um;
  G4double exitLength = 1. * um;
  G4double halfWidth = 5. * um;

  // Scoring
  G4double siteDiameter = 1. * um;
  G4int samplesPerEvent = 200;
  G4double pointSpacing = 10. * nm;  // splitting of condensed-history steps
  G4double yMin = 1e-3;  // keV/um
  G4double yMax = 1e4;  // keV/um
  G4int binsPerDecade = 50;
  G4double y0 = 150.;  // keV/um, saturation parameter for y*
  G4String outputPrefix = "output/micro";

  // Source
  G4String sourceMode = "gun";  // gun | spectrum
  G4String spectrumPrefix;  // step-1 prefix, e.g. output/pristine_150MeV
  G4double spectrumDepth = 0.;
  G4int otherIonZ = 0;  // 0: ion_other spectrum skipped
  G4int otherIonA = 0;
  std::array<G4double, kNSourceSpecies> bias{1., 1., 1., 1., 1., 1., 1.};

  G4String physicsList;

  static MicroConfig& Instance();
};

class MicroMessenger : public G4UImessenger
{
  public:
    MicroMessenger();
    ~MicroMessenger() override;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:
    G4UIdirectory* fDirs[5] = {};
    G4UIcmdWithADoubleAndUnit* fBuildupCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fZoneCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fExitCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fHalfWidthCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fSiteCmd = nullptr;
    G4UIcmdWithAnInteger* fSamplesCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fSpacingCmd = nullptr;
    G4UIcmdWithADouble* fYMinCmd = nullptr;
    G4UIcmdWithADouble* fYMaxCmd = nullptr;
    G4UIcmdWithAnInteger* fBinsCmd = nullptr;
    G4UIcmdWithADouble* fY0Cmd = nullptr;
    G4UIcmdWithAString* fPrefixCmd = nullptr;
    G4UIcmdWithAString* fModeCmd = nullptr;
    G4UIcmdWithAString* fSpectrumCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fDepthCmd = nullptr;
    G4UIcmdWithAString* fOtherIonCmd = nullptr;
    G4UIcmdWithAString* fBiasCmd = nullptr;
};

#endif
