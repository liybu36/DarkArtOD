#include "darkart/TpcMC/SignalGenerator.hh"

#include "TMath.h"
#include <iostream>
#include <vector>

darkart::MC::SignalGenerator::SignalGenerator(fhicl::ParameterSet const& p)
  : _pre_trigger_time(p.get<double>("preTriggerTime")) //[us]
  , _post_trigger_time(p.get<double>("postTriggerTime")) //[us]
  , _sample_time(p.get<double>("sampleTime")) //[us]
  , _use_delta_spe_response(p.get<bool>("useDeltaSpeResponse"))
{ }


double darkart::MC::SignalGenerator::time_response(double t, double t0, double Q)
{
  double mwf = Q*4*(TMath::Landau(t, t0+10.47, 3.2119, 0)*(-(t-t0)*0.03044+1.325 )/2.408);
  if(mwf < 0 ) return 0. ;
  return -mwf ;
}


darkart::Waveform darkart::MC::SignalGenerator::generate(darkart::MC::PeChargeVec const& qVec)
{
  const int nsamps = (int)((_post_trigger_time + _pre_trigger_time)/_sample_time + 0.001); //.001 for rounding error
  const int trigger_index = (int) (_pre_trigger_time/_sample_time+0.001); //.001 for rounding error

  darkart::Waveform result;
  result.wave.resize(nsamps);
  result.trigger_index = trigger_index;
  result.sample_rate = 1./_sample_time; //MHz
  
  for (int i=0; i<nsamps; ++i) {
    double t = (-_pre_trigger_time + i*_sample_time)*1000.; //ns
    double signal_here = 0;
    for (auto const& q : qVec) {
      double t0 = q.time;

      if (_use_delta_spe_response) {
        if (t0 >= t && t0 < t+_sample_time*1000.)
          signal_here += -q.charge;
      }
      else {
        if (t>t0+100 || t<t0-5) {
          //this PE will not contribute to this sample
          continue;
        }
        signal_here += time_response(t, t0, q.charge);
      }
    }
    result.wave[i] = signal_here;
  }//loop over samples

  return std::move(result);
}
