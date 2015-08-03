/*

  2014-10-29 AFan

  Helper class to overlay simulated signal waveforms on baseline.
  Baseline is usually real signal, but could be flat baseline.

 */

#ifndef darkart_TpcMC_SignalOverlayer_hh
#define darkart_TpcMC_SignalOverlayer_hh

#include "fhiclcpp/ParameterSet.h"
#include "darkart/Products/Waveform.hh"
#include "TRandom3.h"
#include <string>

namespace darkart
{
  class SignalOverlayer {
  public:
    SignalOverlayer(fhicl::ParameterSet const& p);

    darkart::Waveform generateBaseline(darkart::Waveform const& raw,
                                       darkart::Waveform const& sig);

    darkart::Waveform smearBaseline(darkart::Waveform const& input_baseline,
                                    darkart::Waveform const& sig);
    
    darkart::Waveform overlay(darkart::Waveform const& baseline,
                              darkart::Waveform const& sig);


    
  private:

    int _seed;
    TRandom3* _ran;
    
    bool   _use_flat_baseline;
    bool   _smear_baseline;
    double _overlay_start_time;
    std::string  _quantization_mode;
    
  }; // SignalOverlay

}//darkart namespace

#endif
