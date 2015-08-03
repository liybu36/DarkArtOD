#ifndef darkart_TpcReco_integrator_algs_hh
#define darkart_TpcReco_integrator_algs_hh

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"

namespace darkart
{
  void smoother(darkart::Waveform const& wfm, 
                darkart::Waveform & smoothedWfm, 
                darkart::WaveformInfo & smoothedData, 
                int pre_samp, int post_samp);

}


#endif
