#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4SystemOfUnits.hh"
#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

#include <memory>

class DetectorMessenger;
class G4Material;
class G4UserLimits;
class G4VPhysicalVolume;

// Water phantom (box) in vacuum. The beam enters along +z and the phantom
// front face sits at z = 0, so the world z coordinate equals the depth.
class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    G4VPhysicalVolume* Construct() override;

    // Must be called before /run/initialize
    void SetPhantomDepth(G4double value) { fDepth = value; }
    void SetPhantomWidth(G4double value) { fWidth = value; }
    void SetMaxStep(G4double value) { fMaxStep = value; }
    void SetMaterial(const G4String& name);

    G4double GetPhantomDepth() const { return fDepth; }
    G4double GetPhantomWidth() const { return fWidth; }
    G4double GetMaxStep() const { return fMaxStep; }
    const G4Material* GetPhantomMaterial() const { return fMaterial; }
    const G4VPhysicalVolume* GetPhantomPV() const { return fPhantomPV; }

  private:
    G4double fDepth = 40. * cm;
    G4double fWidth = 40. * cm;
    G4double fMaxStep = 0.5 * mm;
    G4Material* fMaterial = nullptr;
    G4VPhysicalVolume* fPhantomPV = nullptr;
    std::unique_ptr<G4UserLimits> fUserLimits;
    DetectorMessenger* fMessenger = nullptr;
};

#endif
