#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"

class DetectorConstruction;

class ActionInitialization : public G4VUserActionInitialization
{
  public:
    explicit ActionInitialization(const DetectorConstruction* detector) : fDetector(detector) {}

    void BuildForMaster() const override;
    void Build() const override;

  private:
    const DetectorConstruction* fDetector = nullptr;
};

#endif
