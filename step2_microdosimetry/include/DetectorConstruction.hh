#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

// Box of liquid water (G4_WATER, required by the Geant4-DNA models) holding
// one track segment: z from -buildup to zone + exit (see MicroConfig).
// There is no sensitive volume: sites are placed afterwards around the
// recorded energy-transfer points.
class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    G4VPhysicalVolume* Construct() override;
};

#endif
