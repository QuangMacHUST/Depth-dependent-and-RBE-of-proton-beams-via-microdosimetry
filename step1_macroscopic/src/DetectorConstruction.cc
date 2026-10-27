#include "DetectorConstruction.hh"

#include "DetectorMessenger.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4ThreeVector.hh"
#include "G4UserLimits.hh"

DetectorConstruction::DetectorConstruction()
{
  fMaterial = G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER");
  fMessenger = new DetectorMessenger(this);
}

DetectorConstruction::~DetectorConstruction()
{
  delete fMessenger;
}

void DetectorConstruction::SetMaterial(const G4String& name)
{
  auto material = G4NistManager::Instance()->FindOrBuildMaterial(name);
  if (material) fMaterial = material;
  else G4cerr << "DetectorConstruction: material " << name << " not found, keeping "
              << fMaterial->GetName() << G4endl;
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  auto vacuum = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");

  // World is symmetric in z, large enough to hold the source upstream of z = 0
  const G4double worldXY = 0.5 * fWidth + 1. * cm;
  const G4double worldZ = fDepth + 5. * cm;
  auto worldSolid = new G4Box("World", worldXY, worldXY, worldZ);
  auto worldLV = new G4LogicalVolume(worldSolid, vacuum, "World");
  auto worldPV = new G4PVPlacement(nullptr, G4ThreeVector(), worldLV, "World", nullptr, false, 0);

  auto phantomSolid = new G4Box("Phantom", 0.5 * fWidth, 0.5 * fWidth, 0.5 * fDepth);
  auto phantomLV = new G4LogicalVolume(phantomSolid, fMaterial, "Phantom");
  fUserLimits = std::make_unique<G4UserLimits>(fMaxStep);
  phantomLV->SetUserLimits(fUserLimits.get());
  fPhantomPV = new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.5 * fDepth), phantomLV,
                                 "Phantom", worldLV, false, 0, true);

  G4cout << "\n Phantom: " << fMaterial->GetName() << ", depth " << fDepth / cm << " cm, width "
         << fWidth / cm << " cm, max step " << fMaxStep / mm << " mm\n"
         << G4endl;

  return worldPV;
}
