#include "darkart/TpcReco/integrator_algs.hh"
#include "darkart/TpcReco/common_algs.hh"

#include <algorithm>
#include <cmath>

void darkart::integrator(darkart::Waveform const& wfm,
                         darkart::Waveform & integratedWfm,
                         darkart::WaveformInfo & integralData,
                         double threshold)
{
  //get data
  const int nsamps = wfm.wave.size();
  integratedWfm = wfm;

  //perform integration
  for(int samp = 1 ; samp<nsamps; samp++){
    double onestep = integratedWfm.wave[samp];
    integratedWfm.wave[samp] = integratedWfm.wave[samp-1] + (std::abs(onestep) > threshold ? onestep : 0);
  }

  //fill waveform info
  integralData = fillWaveformInfo(integratedWfm);

 //baseline interpolation things? need spe (or clarification...)

}
