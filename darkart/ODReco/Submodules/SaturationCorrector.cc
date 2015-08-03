////////////////////////////////////////////////////////////////////////
// Class:       SaturationCorrector
// Module Type: Submodule of PulseCorrector
// File:        SaturationCorrector.cc
//
// Generated at Tue Feb  3 08:28:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

/*
This is a submodule of PulseCorrector
Parameters are set by parameter list: baselineparams
Required Parameters:
  -  : [int]

Optional Parameters:
  - (none)
*/
#include <iostream>

#include "darkart/ODReco/Submodules/SaturationCorrector.hh"
#include "darkart/ODReco/Tools/SatCorrTriangle.hh"

#include "Rtypes.h"
//colors
const char cyan[] = { 0x1b, '[', '1', ';', '3', '6', 'm', 0 };
const char magenta[] = { 0x1b, '[', '1', ';', '3', '5', 'm', 0 };
const char red[] = { 0x1b, '[', '1', ';', '3', '8', 'm', 0 };
const char green[] = { 0x1b, '[', '1', ';', '3', '2', 'm', 0 };
const char yellow[] = { 0x1b, '[', '1', ';', '3', '3', 'm', 0 };
const char blue[] = "\x1b[1;34m";
const char bold[] = "\x1b[1;39m";
const char whiteOnRed[]    = "\x1b[1;41m";
const char whiteOnGreen[]  = "\x1b[1;42m";
const char whiteOnPurple[] = "\x1b[1;45m";
const char whiteOnViolet[] = "\x1b[1;44m";
const char whiteOnBrown[]  = "\x1b[1;43m";
const char whiteOnGray[]   = "\x1b[1;47m";
const char normal[] = { 0x1b, '[', '0', ';', '3', '9', 'm', 0 };

// Constructors

SaturationCorrector::SaturationCorrector(fhicl::ParameterSet const &p)
  : _replace_wf_with_sat_trig (p.get<bool>("replace_wf_with_sat_trig", 0))
  , _min_consecutive_samples  (p.get<int>("min_consecutive_samples", -1))
  , _verbosity                (p.get<int>("verbosity", 0))
  , _min_voltage              (p.get<int>("min_voltage", -1))
  , _is_saturated(false)
  , _correction(0.)
    
{;}

// Decides if saturation is present and applies the saturation correction
void SaturationCorrector::isSaturated(darkart::od::ChannelWFs::PulseWF& wf,
				      darkart::od::ChannelData::Pulse& pulse, 
				      const double& v_range, 
				      const double& v_offset, 
				      const int& channel_id){
  const double sample_rate_GHz = _utils->getSampleRateGHz(_utils->getChannelType(channel_id));
  
  const float vmin = float(v_offset - v_range/2.)*0.9;
  const double start_time = pulse.start_sample*sample_rate_GHz;
  // Check that necessary parameters have been set
  if(_min_voltage <= 0) cet::exception("SaturationCorrector") << "Invalid value of _min_voltage in isSaturated: " << _min_voltage;
  if(_min_consecutive_samples <= 0) cet::exception("SaturationCorrector") << "Invalid value of _min_consecutive_samples in isSaturated: " << _min_consecutive_samples;
  
  
  float past_raw_sample = 0;
  int n_equal_samples = 0;//TODO: In the old algo this was initialized to 0 but in fact it should be 1. Fix after comparison with odrec has been done  
  bool in_saturation = false;
  
  sat_corr_triangle_t sat_trig;
  float saturation_corr  = 0;
  float saturation_width = 0;
  bool is_saturated = false;
  unsigned int sat_starts = 0;
  unsigned int sat_ends = 0;
  
  double min_voltage_found=0;
  const size_t wf_data_size = wf.data.size();
  for(size_t samp = 0; samp < wf_data_size; samp++){
    const float pulse_voltage = wf.data.at(samp);
    //integral += pulse_voltage;
    if(wf.data.at(samp)<min_voltage_found) min_voltage_found=wf.data.at(samp);
    /*TODO: remove this. It is  only to remind us that the peak_time should be defined using the triangle.
      if(pulse_voltage < _min_voltage){
      _min_voltage = pulse_voltage; 
      peak_time = samp;
      }*/
    //More than one sample
    if(samp>0){// look for repeated value of samples to find a saturation candidate
      if((past_raw_sample==pulse_voltage) && (pulse_voltage<vmin)){
        n_equal_samples++;
        if(n_equal_samples==_min_consecutive_samples) { //Saturation starts
          is_saturated=true;
          in_saturation = true;
          //stores coordinate where the saturation starts
	  // These lookback times should be made into parameters for the submodule
          const unsigned int k1 = ((samp) < 2) ? 0 : (samp-2);
          sat_starts=k1;
          const double p1y = wf.data.at(k1);//before:raw_data[k1]*gain + offset
          const double p1x = start_time + k1/sample_rate_GHz;
          sat_corr_triangle_t::coordinate p1 (p1x, p1y);
          sat_trig.setP1(p1);
          // stores coordinate p0 (before saturation)
	  // These lookback times should be made into parameters for the submodule
          const int k0 = ((samp) < 6) ? 0 : (samp-6); 
          const double p0y = wf.data.at(k0);
          const double p0x = start_time +k0/sample_rate_GHz;
          sat_corr_triangle_t::coordinate p0 (p0x, p0y);
          sat_trig.setP0(p0);
        }
      }
      else{// this sample not equal to last sample
        if(in_saturation){
          // store coordinate p2 (end of saturation)
	  // These lookforward times should be made into parameters for the submodule
	  int k2 = samp-1; 
	  if(k2 < 0) k2 = 0;
          sat_ends = k2;
          const double p2y = wf.data.at(k2);
          const double p2x = start_time + k2*sample_rate_GHz;
          sat_corr_triangle_t::coordinate p2 (p2x, p2y);
          sat_trig.setP2(p2);
          // store coordinate p3 (few samples after end of saturation) 
	  // These lookforward times should be made into parameters for the submodule
          const unsigned k3 = ((samp+3) < wf.data.size()) ? samp+3 : wf.data.size()-1;
          const double p3y = wf.data.at(k3);
          const double p3x = start_time + k3/sample_rate_GHz;
          sat_corr_triangle_t::coordinate p3 (p3x, p3y);
          sat_trig.setP3(p3);
          // compute correction
          saturation_corr += sat_trig.computeCorrection()/1e9;
          saturation_width+=float(n_equal_samples)/sample_rate_GHz; 
        }
        in_saturation = false;
        n_equal_samples = 0;//TODO: In the old algo this was initialized to 0 but in fact it should be 1. Fix after comparison with odrec has been done
      }
      past_raw_sample = wf.data.at(samp);
    }
  }
  
  _is_saturated = is_saturated;
  _correction   = saturation_corr;

  //Apply corrections when saturation is present
  if(is_saturated){
    //Replace values inside ChannelData
    //pulse.peak_sample=peak_time;//TODO: Leaving it as in ODRec but it is keeping the first of the saturated values.
    pulse.integral = pulse.integral-saturation_corr;
    pulse.saturation_correction = saturation_corr;
    pulse.saturation_width = saturation_width;
    pulse.is_saturated = is_saturated;
    
    
    //Check the modification in the waveform
    //Print the values of the waveform. Used to debug
    if(_verbosity>1){
      for(unsigned int samp=0;samp<wf.data.size();samp++){
        if(samp>=sat_starts && samp<=sat_ends)std::cout <<whiteOnPurple<<wf.data.at(samp)<<normal<<" ";
        else std::cout <<wf.data.at(samp)<<" ";
      }
      std::cout <<std::endl;
    }
    
    //Replaces the values in the saturated region
    if(_replace_wf_with_sat_trig){
      darkart::od::ChannelWFs::PulseWF wf_with_sat_corr = wf;
      for(unsigned int samp=sat_starts;samp<sat_ends+1;samp++){
        const double y=sat_trig.computeY(start_time +samp*1.0/sample_rate_GHz);
        wf_with_sat_corr.data.at(samp)=y;
      }
      wf=wf_with_sat_corr;
    }
    //Print the values of the waveform after saturation. Used to debug
    if(_verbosity>1){
      for(unsigned int samp=0;samp<wf.data.size();samp++){
        if(samp>=sat_starts && samp<=sat_ends)std::cout <<whiteOnPurple<<wf.data.at(samp)<<normal<<" ";
        else std::cout <<wf.data.at(samp)<<" ";
      }
      std::cout <<std::endl;
    }
  }
  
}
