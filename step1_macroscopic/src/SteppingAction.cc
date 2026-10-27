#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "Run.hh"
#include "RunAction.hh"
#include "Species.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "Randomize.hh"

SteppingAction::SteppingAction(const DetectorConstruction* detector, RunAction* runAction,
                               EventAction* eventAction)
  : fDetector(detector), fRunAction(runAction), fEventAction(eventAction)
{}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  const G4StepPoint* pre = step->GetPreStepPoint();
  if (pre->GetPhysicalVolume() != fDetector->GetPhantomPV()) return;

  Run* run = fRunAction->GetRun();
  const G4StepPoint* post = step->GetPostStepPoint();
  const G4ThreeVector& p1 = pre->GetPosition();
  const G4ThreeVector& p2 = post->GetPosition();
  const G4Track* track = step->GetTrack();
  const G4ParticleDefinition* particle = track->GetDefinition();
  const G4int species = ClassifyParticle(particle, track->GetTrackID());

  // Energy deposit: attributed to a random point along the step
  const G4double edep = step->GetTotalEnergyDeposit();
  if (edep > 0.) {
    const G4ThreeVector point = p1 + G4UniformRand() * (p2 - p1);
    const G4int bin = run->DoseBin(point.z());
    if (bin >= 0 && run->InScoringRegion(point)) {
      fEventAction->AddEdep(bin, edep);
      run->AddSpeciesEdep(bin, species, edep);
    }
  }

  // LET and fluence: charged hadrons and ions only (no electrons)
  if (species > kOtherHadron) return;
  const G4double length = step->GetStepLength();
  if (length <= 0. || !run->InScoringRegion(0.5 * (p1 + p2))) return;

  // Mean kinetic energy over the step; if the track was absorbed in a nuclear
  // interaction its post-step energy is zero, so fall back to the pre-step one
  const G4double ePre = pre->GetKineticEnergy();
  const G4double ePost = post->GetKineticEnergy();
  const G4double ekin = ePost > 0. ? 0.5 * (ePre + ePost) : ePre;
  const G4double dedx = fStoppingPower.GetDEDX(particle, pre->GetMaterial(), ekin);

  run->AddTrackSegment(species, p1.z(), p2.z(), length, ekin, dedx);
}
