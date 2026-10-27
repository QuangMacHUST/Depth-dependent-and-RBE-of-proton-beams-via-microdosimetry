// Step 1 (macroscopic): proton depth-dose, LET_t / LET_d and particle
// fluence spectra versus depth in a water phantom.
//
// Usage:
//   protonDepth [-m macro] [-p physicsList] [-t nThreads]
// Without -m an interactive session with visualisation is started.

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "ScoringConfig.hh"

#include "G4PhysListFactory.hh"
#include "G4RunManagerFactory.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VModularPhysicsList.hh"
#include "G4VisExecutive.hh"

#include <cstdlib>

namespace
{
void PrintUsage()
{
  G4cerr << "Usage: protonDepth [-m macro] [-p physicsList] [-t nThreads]\n"
         << "  default physics list: QGSP_BIC_EMZ\n";
}
}  // namespace

int main(int argc, char** argv)
{
  G4String macro;
  G4String physicsName = "QGSP_BIC_EMZ";
  G4int nThreads = 0;

  for (G4int i = 1; i < argc; i += 2) {
    const G4String opt = argv[i];
    if (i + 1 >= argc) {
      PrintUsage();
      return 1;
    }
    if (opt == "-m") macro = argv[i + 1];
    else if (opt == "-p") physicsName = argv[i + 1];
    else if (opt == "-t") nThreads = std::atoi(argv[i + 1]);
    else {
      PrintUsage();
      return 1;
    }
  }

  G4UIExecutive* ui = macro.empty() ? new G4UIExecutive(argc, argv) : nullptr;

  auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
  if (nThreads > 0) runManager->SetNumberOfThreads(nThreads);

  // Scoring commands (/rbe/score/..., /rbe/output/...) live on the master.
  // Deleted before the run manager, which also deletes the UI manager.
  auto scoringMessenger = new ScoringMessenger();
  ScoringConfig::Instance().physicsList = physicsName;

  auto detector = new DetectorConstruction();
  runManager->SetUserInitialization(detector);

  G4PhysListFactory factory;
  if (!factory.IsReferencePhysList(physicsName)) {
    G4cerr << "Unknown reference physics list: " << physicsName << G4endl;
    return 1;
  }
  G4VModularPhysicsList* physics = factory.GetReferencePhysList(physicsName);
  // Enables G4UserLimits (max step in the phantom) for charged particles
  physics->RegisterPhysics(new G4StepLimiterPhysics());
  runManager->SetUserInitialization(physics);

  runManager->SetUserInitialization(new ActionInitialization(detector));

  G4VisManager* visManager = nullptr;
  auto uiManager = G4UImanager::GetUIpointer();
  if (ui) {
    visManager = new G4VisExecutive();
    visManager->Initialize();
    uiManager->ApplyCommand("/control/execute vis.mac");
    ui->SessionStart();
    delete ui;
  }
  else {
    uiManager->ApplyCommand("/control/execute " + macro);
  }

  delete visManager;
  delete scoringMessenger;
  delete runManager;
  return 0;
}
