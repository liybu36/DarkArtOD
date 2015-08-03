////////////////////////////////////////////////////////////////////////
// Class:       BaselineCorrector
// Module Type: Submodule of PulseCorrector
// File:        BaselineCorrector.cc
//
// Generated at Tue Feb  3 08:28:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

/*
This is a submodule of PulseCorrector
Parameters are set by parameter list: baselineparams
Required Parameters:
  - num_baseline_samples : [int]
Optional Parameters:
  - (none)
*/

#include "darkart/ODReco/Submodules/BaselineCorrector.hh"
#include <cmath>
#include <iostream>

// Constructors
BaselineCorrector::BaselineCorrector():
  _num_baseline_samples(0),
  _pedestal_mean(0),
  _pedestal_rms(-1),
  _integral(0),
  _amplitude(0)
{;}
BaselineCorrector::BaselineCorrector(int nsamps):
  _num_baseline_samples(nsamps),
  _pedestal_mean(0),
  _pedestal_rms(-1),
  _integral(0),
  _amplitude(0) 
{;}

BaselineCorrector::BaselineCorrector(fhicl::ParameterSet const &p):
  _num_baseline_samples(p.get<int>("num_baseline_samples", 16)),
  _pedestal_mean(0),
  _pedestal_rms(-1),
  _integral(0),
  _amplitude(0)
{;}

// Calculates the baseline mean and RMS from the first
// _num_baseline_samples of the waveform
void BaselineCorrector::findBaseline(darkart::od::ChannelWFs::PulseWF& wf){
  // Check that necessary parameters have been set
  if(_num_baseline_samples <= 0)
    throw cet::exception("BaselineCorrector") << "Invalid value of _num_baseline_samples in FindBaseline(wf): " << _num_baseline_samples;

  // if a pulse happens to be at the boundary of the acquisition gate, the number of samples digitized can be < _num_baseline_samples
  // we just do not apply baseline correction to these (rare) pulses
  if(_num_baseline_samples>=static_cast<int>(wf.data.size())){
    _pedestal_mean = 0.00001; // !=0 so it does not complain later
    _pedestal_rms  = 0.00001; // !=0 so it does not complain later
  }
    
  // Calculate the mean and RMS
  double mean = 0;
  double rms = 0;
  for(int samp = 0; samp < _num_baseline_samples; samp++){
    const float wf_sample =  wf.data.at(samp);
    mean += wf_sample;
    rms += wf_sample*wf_sample;
  }
  mean /= double(_num_baseline_samples);
  rms /= double(_num_baseline_samples);
  rms = sqrt(rms-mean*mean);
  _pedestal_mean = mean;
  _pedestal_rms = rms;
}

// Loop over the waveform and subtract out the baseline mean
void BaselineCorrector::subtractBaseline(darkart::od::ChannelWFs::PulseWF& wf){
  // Check that the pedestal parameters have all been set/calculated
  if(_num_baseline_samples <= 0)
    throw cet::exception("BaselineCorrector") << "Invalid value of _num_baseline_samples in SubtractBaseline(wf)";
  if(_pedestal_mean == 0)
    throw cet::exception("BaselineCorrector") << "Invalid value of _pedestal_mean in SubtractBaseline(wf)";
  if(_pedestal_rms == -1)
    throw cet::exception("BaselineCorrector") << "Invalid value of _pedestal_rms in SubtractBaseline(wf)";

  // Loop over waveform and subtract out the pedestal
  const size_t n_samples = wf.data.size(); 
  for(size_t samp = 0; samp < n_samples; samp++){
    wf.data[samp] -= _pedestal_mean;
  }
}

// Calculate pulse integral
void BaselineCorrector::integrate(darkart::od::ChannelWFs::PulseWF& wf, 
				  darkart::od::ChannelData::Pulse& pulse,
				  int channel_id){
  const double sample_rate_Hz = _utils->getSampleRateHz(_utils->getChannelType(channel_id));
  // Check that necessary parameters have been set
  if(_num_baseline_samples <= 0)
    throw cet::exception("BaselineCorrector") << "Invalid value of _num_baseline_samples in integrate(wf): " << _num_baseline_samples;

  if(_num_baseline_samples > static_cast<int>(wf.data.size()))
    throw cet::exception("BaselineCorrector") << "_num_baseline_samples (" << _num_baseline_samples << ") is bigger than wf.data.size() (" << wf.data.size() << ") in integrate(wf)";

  const float baseline_integral = _pedestal_mean*float(wf.data.size())/sample_rate_Hz;
  _integral = pulse.integral - baseline_integral;
}

// Calcualte pulse amplitude
void BaselineCorrector::calculateAmplitude(darkart::od::ChannelData::Pulse& pulse){
  // Check that necessary parameters have been set
  if(_num_baseline_samples <= 0)
    throw cet::exception("BaselineCorrector") << "Invalid value of _num_baseline_samples in calculateAmplitude(pulse): " << _num_baseline_samples;

  // Subtract baseline from old ampltiude
  _amplitude = pulse.peak_amplitude - _pedestal_mean;
}
