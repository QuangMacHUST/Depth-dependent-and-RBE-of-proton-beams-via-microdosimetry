#include "StoppingPowerTable.hh"

#include "G4SystemOfUnits.hh"

#include <algorithm>
#include <cmath>

namespace
{
// Total kinetic energy grid, 100 points per decade
const G4double kEMin = 1. * keV;
const G4double kEMax = 1. * GeV;
const G4int kNPoints = 601;
const G4double kLogStep = std::log(kEMax / kEMin) / (kNPoints - 1);
}  // namespace

const std::vector<G4double>& StoppingPowerTable::Table(const Key& key)
{
  if (key == fLastKey) return *fLastTable;

  auto it = fTables.find(key);
  if (it == fTables.end()) {
    std::vector<G4double> dedx(kNPoints);
    for (G4int i = 0; i < kNPoints; ++i) {
      const G4double e = kEMin * std::exp(i * kLogStep);
      dedx[i] = fCalculator.ComputeElectronicDEDX(e, key.first, key.second);
    }
    it = fTables.emplace(key, std::move(dedx)).first;
  }
  fLastKey = key;
  fLastTable = &it->second;
  return it->second;
}

G4double StoppingPowerTable::GetDEDX(const G4ParticleDefinition* particle,
                                     const G4Material* material, G4double ekin)
{
  const auto& table = Table({particle, material});
  if (ekin <= kEMin) return table.front();
  if (ekin >= kEMax) return table.back();
  const G4double x = std::log(ekin / kEMin) / kLogStep;
  const G4int i = std::min(static_cast<G4int>(x), kNPoints - 2);
  const G4double f = x - i;
  return (1. - f) * table[i] + f * table[i + 1];
}
