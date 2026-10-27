#include "SpectrumSource.hh"

#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"
#include "Randomize.hh"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

namespace
{
std::vector<G4double> SplitNumbers(const std::string& line)
{
  std::vector<G4double> values;
  std::stringstream ss(line);
  std::string item;
  while (std::getline(ss, item, ',')) values.push_back(std::stod(item));
  return values;
}

// Reads the energy edges (MeV) and the dPhi/dE row (cm^-2 MeV^-1) of the depth
// bin containing depthMm. Returns false if the file or the row is missing.
G4bool ReadSpectrumRow(const G4String& path, G4double depthMm, std::vector<G4double>& edges,
                       std::vector<G4double>& row)
{
  std::ifstream in(path);
  if (!in) return false;
  const std::string edgeTag = "# E_edges_MeV,";
  std::string line;
  G4bool headerSeen = false;
  while (std::getline(in, line)) {
    if (line.empty()) continue;
    if (line[0] == '#') {
      if (line.compare(0, edgeTag.size(), edgeTag) == 0)
        edges = SplitNumbers(line.substr(edgeTag.size()));
      continue;
    }
    if (!headerSeen) {  // z_lo_mm,z_hi_mm,<bin centres>
      headerSeen = true;
      continue;
    }
    const std::vector<G4double> values = SplitNumbers(line);
    if (values.size() < 2 || depthMm < values[0] || depthMm >= values[1]) continue;
    row.assign(values.begin() + 2, values.end());
    return !edges.empty() && row.size() + 1 == edges.size();
  }
  return false;
}
}  // namespace

void SpectrumSource::Load(const MicroConfig& config)
{
  fBins.clear();
  fCumulative.clear();
  const G4bool verbose = G4Threading::G4GetThreadId() <= 0;

  G4double trueSum[kNSpectrumSources] = {};
  G4double trueTotal = 0., biasedTotal = 0.;
  std::vector<G4double> biasedProb;

  for (G4int is = 0; is < kNSpectrumSources; ++is) {
    if (is == kSourceOtherIon && config.otherIonZ == 0) continue;
    const G4String path = config.spectrumPrefix + "_spectrum_" + SourceFileName(is) + ".csv";
    std::vector<G4double> edges, row;
    if (!ReadSpectrumRow(path, config.spectrumDepth / mm, edges, row)) {
      G4Exception("SpectrumSource::Load", "Micro010", JustWarning,
                  ("cannot read depth row from " + path + ", species skipped").c_str());
      continue;
    }
    for (std::size_t ie = 0; ie < row.size(); ++ie) {
      const G4double p = row[ie] * (edges[ie + 1] - edges[ie]);
      if (p <= 0.) continue;
      fBins.push_back({is, edges[ie] * MeV, edges[ie + 1] * MeV});
      biasedProb.push_back(p * config.bias[is]);
      trueSum[is] += p;
      trueTotal += p;
      biasedTotal += p * config.bias[is];
    }
  }
  if (fBins.empty())
    G4Exception("SpectrumSource::Load", "Micro011", FatalException,
                ("no fluence found at the requested depth for prefix " + config.spectrumPrefix).c_str());

  fCumulative.resize(biasedProb.size());
  G4double sum = 0.;
  for (std::size_t i = 0; i < biasedProb.size(); ++i) fCumulative[i] = (sum += biasedProb[i]);
  for (G4int is = 0; is < kNSpectrumSources; ++is)
    fWeights[is] = (biasedTotal / trueTotal) / config.bias[is];

  if (verbose) {
    G4cout << "\n Fluence at " << config.spectrumDepth / mm << " mm (" << config.spectrumPrefix
           << "):\n";
    for (G4int is = 0; is < kNSpectrumSources; ++is)
      if (trueSum[is] > 0.)
        G4cout << "   " << SourceName(is) << ": " << 100. * trueSum[is] / trueTotal
               << " % of track length, bias " << config.bias[is] << '\n';
    G4cout << G4endl;
  }
}

SpectrumSource::Sample SpectrumSource::Draw() const
{
  const G4double u = G4UniformRand() * fCumulative.back();
  const std::size_t i = std::min<std::size_t>(
    std::upper_bound(fCumulative.begin(), fCumulative.end(), u) - fCumulative.begin(),
    fCumulative.size() - 1);
  const Bin& bin = fBins[i];
  const G4double energy = bin.eLow * std::pow(bin.eHigh / bin.eLow, G4UniformRand());
  return {bin.species, energy, fWeights[bin.species]};
}
