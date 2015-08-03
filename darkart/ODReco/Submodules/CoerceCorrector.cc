////////////////////////////////////////////////////////////////////////
// Class:       CoerceCorrector
// Module Type: Submodule of PulseCorrector
// File:        CoerceCorrector.cc
//
// Generated at Tue Apr  14 08:28:02 2015 by Stefano Davini.
////////////////////////////////////////////////////////////////////////

/*
This is a submodule of PulseCorrector
Parameters are set by parameter list: baselineparams
Required Parameters:
  - coerce_max : [float]
  - coerce_min : [float]
Optional Parameters:
  - (none)
*/

#include "darkart/ODReco/Submodules/CoerceCorrector.hh"
#include <cmath>
#include <iostream>

// Constructors
CoerceCorrector::CoerceCorrector():
  _coerce_max(5.e20),
  _coerce_min(-5.e20),
  _coerced(false),
  _integral(0),
  _amplitude(0),
  _peak_sample(0)
{;}

CoerceCorrector::CoerceCorrector(fhicl::ParameterSet const &p, int verbosity):
  _verbosity(verbosity),
  _coerce_max(p.get<float>("coerce_max",  5.e20)),
  _coerce_min(p.get<float>("coerce_min", -5.e20)),
  _coerced(false),
  _integral(0),
  _amplitude(0),
  _peak_sample(0)
{;}


// Loop over the waveform and coerce it
void CoerceCorrector::coerce(darkart::od::ChannelWFs::PulseWF& wf, int channel_id){

  _coerced  = false;
  _integral = 0;
  _peak_sample = 0;
  double integral = 0.;
  float amplitude_min = +5.e20; // unphysical value
  int   sample_min = 0; 
  const double sample_rate_Hz = _utils->getSampleRateHz(_utils->getChannelType(channel_id));
  
  // Loop over waveform and coerce it
  const size_t n_samples = wf.data.size(); 
  for(size_t samp = 0; samp < n_samples; samp++){
    const double wf_data_samp = wf.data[samp]; // before coercing
    if (wf_data_samp > _coerce_max) { wf.data[samp] = _coerce_max; _coerced=true;}
    if (wf_data_samp < _coerce_min) { wf.data[samp] = _coerce_min; _coerced=true;}
    integral += wf.data[samp];

    // recompute peak sample and amplitude; this assumes that the waveform is in negative polarity,
    // e.g. before spe conversion
    // the algorithm looks for the minimum 
    // in future development, one can use a boolean fhicl option "polarity" to alternatively look for minumum or maximum
    if (wf.data[samp] < amplitude_min){
        amplitude_min = wf.data[samp];
	sample_min = samp;
      }
  }
  _integral    = integral/sample_rate_Hz;
  _amplitude   = amplitude_min;
  _peak_sample = sample_min;

}

