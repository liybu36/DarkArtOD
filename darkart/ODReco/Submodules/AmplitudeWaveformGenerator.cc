////////////////////////////////////////////////////////////////////////
// Class:       AmplitudeWaveformGenerator
// Module Type: Submodule of SumWaveformGenerator
// File:        AmplitudeWaveformGenerator.cc
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
#include "darkart/ODReco/Submodules/AmplitudeWaveformGenerator.hh"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iostream>

// Constructors
AmplitudeWaveformGenerator::AmplitudeWaveformGenerator():
  _verbosity(0)
{;}
AmplitudeWaveformGenerator::AmplitudeWaveformGenerator(int nsamps):
  _verbosity(0)
{
  nsamps += 0;
}

AmplitudeWaveformGenerator::AmplitudeWaveformGenerator(fhicl::ParameterSet const &p, int default_type=0)
  : _verbosity(0)
  , _channel_type(p.get<int>("channel_type", default_type))
{}

void AmplitudeWaveformGenerator::sum(const darkart::od::ChannelDataVec& channel_data_vec,
				     const darkart::od::ChannelWFsVec& channel_wfs_vec,
				     darkart::od::SumWF& sum_wf, int desired_channel = -1){
  if (_verbosity>0)
    LOG_INFO("AmplitudeWaveformGenerator") << " N channels: "<< channel_data_vec.size() << std::endl;
  

  const size_t ch_data_vec_size = channel_data_vec.size(); 
  // loop over ChannelDataVec
  for (size_t icd=0; icd<ch_data_vec_size; icd++){
    darkart::od::ChannelData const& channel_data = channel_data_vec[icd];
    const int ch_id = channel_data.channel_id;
    // Check if channel number is the desired channel
    // desired_channel == -1 means all channels are desired
    if(ch_id != desired_channel && desired_channel != -1 )
      continue;    
    // Check the channel type (lsv = 1, wt = 2, disabled lsv = -1, disabled wt = -2)
    // and skip if it is not the desired type
    if(channel_data.channel_type != _channel_type)
      continue;

    
    // loop over pulses
    const size_t ch_data_pulses_size = channel_data.pulses.size();
    for (size_t ip=0; ip<ch_data_pulses_size; ip++){
      darkart::od::ChannelData::Pulse const& pulse = channel_data.pulses[ip];
      const int pulse_id = pulse.pulse_id;
      const int pulse_start_sample = pulse.start_sample; // retrieve start_sample for the pulse

      darkart::od::ChannelWFs::PulseWF const& wf = channel_wfs_vec[ch_id].pulse_wfs[pulse_id];

      // make a record of the channel and pulse at the pulse start and end
      darkart::od::ChPulsePair ch_pulse_pair (ch_id, pulse_id);
      sum_wf[pulse_start_sample].ch_pulse_start_pairs.push_back(std::move(ch_pulse_pair));
      // record end_pairs only if it is between the boundaries of the waveform (avoid segfault)
      if(pulse_start_sample+int(wf.data.size()) < int(sum_wf.size()))
	sum_wf[pulse_start_sample+int(wf.data.size())].ch_pulse_end_pairs.push_back(std::move(ch_pulse_pair));
      
      // loop over the waveform
      const size_t wf_data_size = wf.data.size();
      for (size_t iwf=0; iwf<wf_data_size; iwf++){
	const size_t sum_idx = pulse_start_sample + iwf;
	if (sum_idx >= sum_wf.size()) {
	  LOG_INFO("AmplitudeWaveformGenerator") << " Index of the sum waveform "<< sum_idx << " out of the waveform boundaries: " << sum_wf.size() << " at pulse " << ip << " of channel " << icd << std::endl;
	  break;
	}
        sum_wf[pulse_start_sample+iwf].amplitude += wf.data[iwf];
      } // end loop over waveform
    }//end loop over pulses
  } // end loop over ChannelDataVec



}//sum()

