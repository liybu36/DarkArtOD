#include "darkart/TpcMC/SignalOverlayer.hh"
#include <iomanip>
#include <cmath>

darkart::SignalOverlayer::SignalOverlayer(fhicl::ParameterSet const& p)
  : _seed(p.get<int>("seed"))
  , _use_flat_baseline(p.get<bool>("use_flat_baseline"))
  , _smear_baseline(p.get<bool>("smear_baseline"))
  , _overlay_start_time(p.get<int>("overlay_start_time"))
  , _quantization_mode(p.get<std::string>("quantization_mode"))
{
  _ran = new TRandom3(_seed);
}

darkart::Waveform darkart::SignalOverlayer::generateBaseline(darkart::Waveform const& raw,
                                                             darkart::Waveform const& sig)
{
  const int nsamps = sig.wave.size();
  
  // generate baseline 
  std::vector<double> baseline(nsamps);

  //--------------------------------
  if (_use_flat_baseline) {
    for (int i=0; i<nsamps; ++i) {
      baseline[i] = 3800;
    }
  }
  //--------------------------------
  else { //use real waveform as baseline

    // Find sample in raw waveform to start overlaying signal.
    // If _overlay_start_index, start in random location.
    int start_samp;
    if (_overlay_start_time == -999)
      start_samp = _ran->Integer(nsamps);
    else
      start_samp = raw.TimeToSample(_overlay_start_time);
    
    for (int i=0; i<nsamps; ++i) {
      
      // samp indexes to raw waveform. if beyond edge, wrap around to beginning.
      int samp = (i+start_samp) % raw.wave.size();

      baseline[i] = raw.wave[samp];

    }//loop over samps
  }

  darkart::Waveform result;
  result.wave = std::move(baseline);
  result.trigger_index = raw.trigger_index;
  result.sample_rate = raw.sample_rate;

  return std::move(result);
}

darkart::Waveform darkart::SignalOverlayer::smearBaseline(darkart::Waveform const& input_baseline,
                                                          darkart::Waveform const& sig)
{
  const int nsamps = input_baseline.wave.size();
  
  // generate baseline 
  std::vector<double> baseline(input_baseline.wave);

  if (_smear_baseline) {
    for (int i=0; i<nsamps; ++i) {
      if (std::abs(sig.wave[i]) > 1E-6) {
        // smear only in presence of signal. otherwise unnecessary.
        if      (_quantization_mode == "FLOOR") baseline[i] += _ran->Uniform();
        else if (_quantization_mode == "CEIL" ) baseline[i] -= _ran->Uniform();
        else if (_quantization_mode == "ROUND") baseline[i] += _ran->Uniform() - 0.5;
      }
    }// loop over samples
  }
  darkart::Waveform result;
  result.wave = std::move(baseline);
  result.trigger_index = input_baseline.trigger_index;
  result.sample_rate = input_baseline.sample_rate;

  return std::move(result);
  
}


darkart::Waveform darkart::SignalOverlayer::overlay(darkart::Waveform const& baseline,
                                                    darkart::Waveform const& sig)
{

  const int nsamps = sig.wave.size();

  // calculate total waveform
  darkart::Waveform result;
  result.wave.resize(nsamps);
  result.trigger_index = sig.trigger_index;
  result.sample_rate = sig.sample_rate;
  
  for (int i=0; i<nsamps; ++i) {
    result.wave[i] = baseline.wave[i] + sig.wave[i];

    // When signal is small, baseline is un-smeared, regardless of smearing or quantization
    // mode. When signal is small AND we want to quantize, then round to nearest integer.
    // When signal is not small and we want to quantize, do normal quantization. When we
    // don't want to quantize, leave alone.
    if ((_quantization_mode == "FLOOR" ||
         _quantization_mode == "CEIL" ||
         _quantization_mode == "ROUND") &&
        std::abs(sig.wave[i]) < 1E-6) {
      result.wave[i] = std::round(result.wave[i]);
    }
    else {
      if      (_quantization_mode == "FLOOR") result.wave[i] = std::floor(result.wave[i]);
      else if (_quantization_mode == "CEIL" ) result.wave[i] = std::ceil (result.wave[i]);
      else if (_quantization_mode == "ROUND") result.wave[i] = std::round(result.wave[i]);
    }

    // simulate saturation
    if (result.wave[i] < 0) result.wave[i] = 0;
  }

  return std::move(result);
}//overlay
