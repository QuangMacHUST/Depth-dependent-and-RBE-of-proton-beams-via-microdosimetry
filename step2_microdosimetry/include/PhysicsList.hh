#ifndef PhysicsList_h
#define PhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

// Electromagnetic Geant4-DNA physics only (nuclear interactions are
// negligible over a few micrometres). Names: dna_opt0, dna_opt2, dna_opt4,
// dna_opt6 for G4EmDNAPhysics[_optionN].
class PhysicsList : public G4VModularPhysicsList
{
  public:
    explicit PhysicsList(const G4String& name);

    static G4bool IsKnown(const G4String& name);
};

#endif
