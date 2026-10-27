// Step 2 (microscopic): lineal-energy spectra f(y), d(y) and yF, yD, y* in a
// spherical water site with Geant4-DNA, for a single particle (/gun/) or for
// the step-1 fluence spectra at a given depth.
//
// Usage:
//   protonMicro -m macro [-p dna_opt0|dna_opt2|dna_opt4|dna_opt6] [-t nThreads]

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "MicroConfig.hh"
#include "PhysicsList.hh"

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"

#include <cstdlib>

namespace
{
void PrintUsage()
{
  G4cerr << "Usage: protonMicro -m macro [-p dna_opt0|dna_opt2|dna_opt4|dna_opt6] [-t nThreads]\n"
         << "  default physics: dna_opt4\n";
}
}  // namespace

int main(int argc, char** argv)
{
  G4String macro;
  G4String physicsName = "dna_opt4";
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
  if (macro.empty() || !PhysicsList::IsKnown(physicsName)) {
    PrintUsage();
    return 1;
  }

  auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
  if (nThreads > 0) runManager->SetNumberOfThreads(nThreads);

  // Deleted before the run manager, which also deletes the UI manager
  auto messenger = new MicroMessenger();
  MicroConfig::Instance().physicsList = physicsName;

  runManager->SetUserInitialization(new DetectorConstruction());
  runManager->SetUserInitialization(new PhysicsList(physicsName));
  runManager->SetUserInitialization(new ActionInitialization());

  G4UImanager::GetUIpointer()->ApplyCommand("/control/execute " + macro);

  delete messenger;
  delete runManager;
  return 0;
}
