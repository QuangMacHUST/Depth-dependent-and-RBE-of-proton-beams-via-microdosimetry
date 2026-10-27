#include "PhysicsList.hh"

#include "G4EmDNAPhysics.hh"
#include "G4EmDNAPhysics_option2.hh"
#include "G4EmDNAPhysics_option4.hh"
#include "G4EmDNAPhysics_option6.hh"
#include "G4SystemOfUnits.hh"

G4bool PhysicsList::IsKnown(const G4String& name)
{
  return name == "dna_opt0" || name == "dna_opt2" || name == "dna_opt4" || name == "dna_opt6";
}

PhysicsList::PhysicsList(const G4String& name)
{
  // Cuts only matter above the DNA model limits, where standard models take over
  SetDefaultCutValue(1. * um);

  if (name == "dna_opt0") RegisterPhysics(new G4EmDNAPhysics());
  else if (name == "dna_opt2") RegisterPhysics(new G4EmDNAPhysics_option2());
  else if (name == "dna_opt4") RegisterPhysics(new G4EmDNAPhysics_option4());
  else if (name == "dna_opt6") RegisterPhysics(new G4EmDNAPhysics_option6());
  else G4Exception("PhysicsList", "Micro001", FatalException, ("unknown physics " + name).c_str());
}
