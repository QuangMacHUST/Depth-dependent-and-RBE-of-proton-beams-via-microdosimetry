#include "EventAction.hh"

#include "PrimaryGeneratorAction.hh"
#include "Run.hh"
#include "RunAction.hh"

#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <cmath>

EventAction::EventAction(RunAction* runAction, const PrimaryGeneratorAction* primary)
  : fRunAction(runAction), fPrimary(primary)
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
  const MicroConfig& cfg = fRunAction->GetRun()->Config();
  const G4double radius = 0.5 * cfg.siteDiameter;
  fKeepZLow = -radius;
  fKeepZHigh = cfg.zoneLength + radius;
  fSpacing = cfg.pointSpacing;
  fPoints.clear();
}

void EventAction::Store(const G4ThreeVector& point, G4double edep)
{
  if (point.z() < fKeepZLow || point.z() > fKeepZHigh) return;
  fPoints.push_back({point.x() / um, point.y() / um, point.z() / um, edep / keV});
}

void EventAction::AddDeposit(const G4ThreeVector& pre, const G4ThreeVector& post, G4double edep,
                             G4bool atPostStep)
{
  const G4double length = (post - pre).mag();
  const G4int n = atPostStep ? 1 : std::max(1, static_cast<G4int>(std::ceil(length / fSpacing)));
  if (n == 1) {
    Store(atPostStep ? post : 0.5 * (pre + post), edep);
    return;
  }
  for (G4int k = 0; k < n; ++k) Store(pre + ((k + 0.5) / n) * (post - pre), edep / n);
}

void EventAction::EndOfEventAction(const G4Event*)
{
  Run* run = fRunAction->GetRun();
  const MicroConfig& cfg = run->Config();
  const G4int species = fPrimary->LastSpecies();

  run->BeginEvent();
  SiteSampler sampler(0.5 * cfg.siteDiameter / um, 0., cfg.zoneLength / um);
  sampler.SetPoints(fPoints);
  if (sampler.TotalEnergy() > 0.) {
    // Every accepted sample carries E / N (see SiteSampler), times the source weight
    const G4int trials = cfg.samplesPerEvent;
    const G4double weight = fPrimary->LastWeight() * sampler.TotalEnergy() / trials;
    const std::function<double()> uniform = [] { return G4UniformRand(); };
    for (G4int t = 0; t < trials; ++t) {
      const G4double eps = sampler.Sample(uniform);
      if (eps > 0.) run->AddSample(species, weight, eps / run->MeanChord());
    }
  }
  run->EndEvent(species);
}
