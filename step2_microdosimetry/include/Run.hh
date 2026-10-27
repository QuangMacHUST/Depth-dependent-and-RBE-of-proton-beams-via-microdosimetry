#ifndef Run_h
#define Run_h 1

#include "MicroConfig.hh"
#include "SourceSpecies.hh"

#include "G4Run.hh"
#include "globals.hh"

#include <array>
#include <vector>

// Lineal-energy accumulators (y in keV/um), merged over threads.
//
// Each accepted site sample carries a weight w (energy-like, see SiteSampler):
// sums of w give d(y), sums of w / y give f(y). Per event the sums
//   A = sum w y,  B = sum w,  C = sum w / y,  S = sum (w / y)(1 - exp(-y^2 / y0^2))
// are formed, so that yD = A / B, yF = B / C and y* = y0^2 S / B, with
// statistical uncertainties from the event-to-event spread (ratio estimator).
class Run : public G4Run
{
  public:
    explicit Run(const MicroConfig& config);

    void Merge(const G4Run* other) override;

    const MicroConfig& Config() const { return fConfig; }
    G4double MeanChord() const { return fMeanChord; }  // um

    void BeginEvent();
    void AddSample(G4int species, G4double weight, G4double y);
    void EndEvent(G4int species);

    void Write() const;

  private:
    enum Moment : G4int { kA = 0, kB, kC, kS, kAA, kBB, kCC, kSS, kAB, kBC, kSB, kNMoments };

    G4int Bin(G4double y) const;
    G4double Edge(G4int bin) const;
    G4double RatioUncertainty(G4int x, G4int y, G4int xx, G4int yy, G4int xy) const;
    void WriteDistribution(const G4String& path) const;
    void WriteSummary(const G4String& path) const;

    MicroConfig fConfig;
    G4double fMeanChord = 0.;
    G4int fNBins = 0;

    std::vector<G4double> fHistD;  // [species * nBins + bin], sum of w
    std::vector<G4double> fHistF;  // [species * nBins + bin], sum of w / y
    std::array<G4double, kNMoments> fMoments{};
    std::array<std::array<G4double, 4>, kNSourceSpecies> fSpeciesSums{};  // A, B, C, S
    std::array<G4double, kNSourceSpecies> fSpeciesEvents{};
    G4double fOutOfRange = 0.;  // sum of w outside the y axis
    G4double fEventsWithHits = 0.;
    G4double fAccepted = 0.;

    G4double fEvent[4] = {};  // A, B, C, S of the current event
};

#endif
