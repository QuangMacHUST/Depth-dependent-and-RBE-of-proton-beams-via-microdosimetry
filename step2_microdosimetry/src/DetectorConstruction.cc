#include "DetectorConstruction.hh"

#include "MicroConfig.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4ThreeVector.hh"

#include <algorithm>

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  const auto& cfg = MicroConfig::Instance();
  auto nist = G4NistManager::Instance();
  auto water = nist->FindOrBuildMaterial("G4_WATER");
  auto vacuum = nist->FindOrBuildMaterial("G4_Galactic");

  // Water from z = -buildup (minus a small margin, so that the primary starts
  // inside it) to z = zone + exit; particles leaving it are lost in vacuum.
  const G4double zLow = -1.01 * cfg.buildupLength;
  const G4double zHigh = cfg.zoneLength + cfg.exitLength;

  const G4double worldXY = 1.1 * cfg.halfWidth;
  const G4double worldZ = 1.1 * std::max(-zLow, zHigh);
  auto worldSolid = new G4Box("World", worldXY, worldXY, worldZ);
  auto worldLV = new G4LogicalVolume(worldSolid, vacuum, "World");
  auto worldPV = new G4PVPlacement(nullptr, G4ThreeVector(), worldLV, "World", nullptr, false, 0);

  auto waterSolid = new G4Box("Water", cfg.halfWidth, cfg.halfWidth, 0.5 * (zHigh - zLow));
  auto waterLV = new G4LogicalVolume(waterSolid, water, "Water");
  new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.5 * (zLow + zHigh)), waterLV, "Water", worldLV,
                    false, 0, true);

  G4cout << "\n Water: z from " << zLow / um << " to " << zHigh / um << " um, half-width "
         << cfg.halfWidth / um << " um; site centres in [0, " << cfg.zoneLength / um << ") um\n"
         << G4endl;
  return worldPV;
}
