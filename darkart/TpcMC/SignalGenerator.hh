/*

  2014-10-29 AFan

  Helper class to generate simulated waveform. Create simulated
  waveform that is discretized in time but is continuously distributed
  in charge. That is, the waveform is discretely distributed horizontally
  and continuously distributed vertically. Charge dicretization occurs in
  overlay step in downstream module.

 */

#ifndef darkart_TpcMC_SignalGenerator_hh
#define darkart_TpcMC_SignalGenerator_hh


#include "fhiclcpp/ParameterSet.h"

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/PeCharge.hh"

#include <vector>
#include <cmath>
#include <algorithm>


namespace darkart
{
  namespace MC
  {
    class SignalGenerator
    {
    public:
      SignalGenerator(fhicl::ParameterSet const& p);

      darkart::Waveform generate(darkart::MC::PeChargeVec const& qVec);
      
    private:

      double _pre_trigger_time;
      double _post_trigger_time;
      double _sample_time;
      bool   _use_delta_spe_response;

      
      double time_response(double t, double t0, double Q);
    };

  }

}

#endif
