#ifndef ScoringConfig_h
#define ScoringConfig_h 1

#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4UImessenger.hh"
#include "globals.hh"

class G4UIcmdWithAnInteger;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAString;
class G4UIdirectory;

// Scoring settings shared by all threads. Written only from the master
// thread between runs (via ScoringMessenger); each Run copies it at creation.
struct ScoringConfig
{
  G4int nDepthBins = 800;  // dose / LET grid
  G4double depthMax = 0.;  // <= 0: use the phantom depth
  G4int nSpectrumDepthBins = 200;  // fluence-spectrum grid (coarser)
  G4double eMin = 1. * keV;  // log energy axis for spectra
  G4double eMax = 1. * GeV;
  G4int nEnergyBins = 180;
  G4double scoringRadius = 0.;  // <= 0: whole phantom cross-section
  G4String outputPrefix = "output/run";
  G4String physicsList;

  static ScoringConfig& Instance();
};

class ScoringMessenger : public G4UImessenger
{
  public:
    ScoringMessenger();
    ~ScoringMessenger() override;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:
    G4UIdirectory* fRbeDir = nullptr;
    G4UIdirectory* fScoreDir = nullptr;
    G4UIdirectory* fOutputDir = nullptr;
    G4UIcmdWithAnInteger* fDepthBinsCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fDepthMaxCmd = nullptr;
    G4UIcmdWithAnInteger* fSpecDepthBinsCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fEMinCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fEMaxCmd = nullptr;
    G4UIcmdWithAnInteger* fEBinsCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fRadiusCmd = nullptr;
    G4UIcmdWithAString* fPrefixCmd = nullptr;
};

#endif
