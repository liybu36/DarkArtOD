////////////////////////////////////////////////////////////////////////
// Class:       DiscriminatorWaveformGenerator
// Module Type: Submodule of SumWaveformGenerator
// File:        DiscriminatorWaveformGenerator.cc
//
// Generated at Tue Feb  3 03:43:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

/*
This is a submodule of SumWaveformGenerator
Parameters are set by parameter list: amplitudesumparams
Required Parameters:
  - (none)
Optional Parameters:
  - verbosity
*/

#include "darkart/ODReco/Submodules/DiscriminatorWaveformGenerator.hh"

// Constructors
DiscriminatorWaveformGenerator::DiscriminatorWaveformGenerator():
  _verbosity(0)
{;}
DiscriminatorWaveformGenerator::DiscriminatorWaveformGenerator(int nsamps):
  _verbosity(0)
{
  nsamps += 0;
}

DiscriminatorWaveformGenerator::DiscriminatorWaveformGenerator(fhicl::ParameterSet const &p,int default_type=0)
  : _verbosity(0)
  , _channel_type(p.get<int>("channel_type", default_type))
  , _width_ns(p.get<float>("width_ns", -1.))
{
  if(_width_ns < 0.8)
    throw cet::exception("DiscriminatorWaveformGenerator") << "Must specify a physical value (>= 0.8) for width_ns: " << _width_ns << std::endl;
}

void DiscriminatorWaveformGenerator::sum(const darkart::od::ChannelDataVec& channel_data_vec,
					 darkart::od::SumWF& wf,
					 int desired_channel){
  _sample_rate_GHz = _utils->getSampleRateGHz(_channel_type);
  _width_samps = int(_width_ns * _sample_rate_GHz);
  _zs_presamples = _utils->getZSPresamples(_channel_type);
  _zs_postsamples = _utils->getZSPostsamples(_channel_type);

  // Loop over ChannelDataVec
  const size_t ch_data_vec_size = channel_data_vec.size();
  for (size_t icd=0; icd<ch_data_vec_size; icd++){
    darkart::od::ChannelData const& channel_data = channel_data_vec.at(icd);
    const int ch_id = channel_data.channel_id;
    // Check if channel number is the desired channel
    // desired_channel == -1 means all channels are desired
    if(ch_id != desired_channel && desired_channel != -1 )
      continue;
    // Check the channel type (lsv = 1, wt = 2, disabled lsv = -1, disabled wt = -2)
    // and skip if it is not the desired type
    if(channel_data.channel_type != _channel_type)
      continue;
    
    // Construct a discriminator for this channel
    const size_t wf_size = wf.size();
    std::vector<int> channel_discrim(wf_size, 0);
    const size_t channel_discrim_size = channel_discrim.size();

    // Loop over pulses
    for (int ip=0; ip < channel_data.npulses; ip++){
      darkart::od::ChannelData::Pulse const& pulse = channel_data.pulses.at(ip);
      
      // Get signal start and end
      const int& start_sample = pulse.start_sample + _zs_postsamples;
      const int& pulse_width  = pulse.size_sample - _zs_presamples - _zs_postsamples;
      int end_sample = start_sample + _width_samps;

      // If the pulse width is greater than the discriminator logic pulse width
      //   add the pulse width to the logic pulse width
      if(pulse_width > _width_samps)
	end_sample += pulse_width;
      
      // Check that sample is within range
      if(start_sample < 0 || start_sample >= int(channel_discrim_size))
	throw cet::exception("DiscriminatorWaveformGenerator") << "Pulse " << ip << " of channel " << icd << " starts out of range of sum waveform: " << start_sample << " ("<< channel_discrim_size << ")" <<   std::endl;
      if(end_sample >= int(channel_discrim_size)) 
	end_sample = int(channel_discrim_size)-1;

      // Set discriminator values for this channel
      std::fill(channel_discrim.begin()+start_sample,
		channel_discrim.begin()+end_sample,
		1);
    } //end for ip (pulses)
    
    // Add this channel's discriminator to the total discriminator
    for(size_t iwf=0; iwf < wf_size; iwf++){
      wf.at(iwf).amplitude_discr += channel_discrim.at(iwf);
    }
    
  } // end for icd
}
