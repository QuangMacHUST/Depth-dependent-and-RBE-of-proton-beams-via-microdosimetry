#ifndef StoppingPowerTable_h
#define StoppingPowerTable_h 1

#include "G4EmCalculator.hh"
#include "globals.hh"

#include <map>
#include <utility>
#include <vector>

class G4Material;
class G4ParticleDefinition;

// Thread-local cache of unrestricted electronic stopping powers, built lazily
// per (particle, material) from G4EmCalculator on a log energy grid and
// interpolated linearly in log(E). Calling G4EmCalculator on every step
// would be far too slow.
class StoppingPowerTable
{
  public:
    G4double GetDEDX(const G4ParticleDefinition* particle, const G4Material* material,
                     G4double ekin);

  private:
    using Key = std::pair<const G4ParticleDefinition*, const G4Material*>;

    const std::vector<G4double>& Table(const Key& key);

    G4EmCalculator fCalculator;
    std::map<Key, std::vector<G4double>> fTables;
    Key fLastKey{nullptr, nullptr};
    const std::vector<G4double>* fLastTable = nullptr;
};

#endif
