#ifndef darkart_TpcReco_integrator_algs_hh
#define darkart_TpcReco_integrator_algs_hh

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"

namespace darkart
{
  //baseline waveform, integrated waveform, integratordata, threshold, chinfo
  void integrator(darkart::Waveform const& wfm,
		  darkart::Waveform & integratedWfm,
		  darkart::WaveformInfo & integralData,
		  double threshold);

}

#endif
