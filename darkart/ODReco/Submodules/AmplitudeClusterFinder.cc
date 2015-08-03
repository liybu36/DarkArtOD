////////////////////////////////////////////////////////////////////////
// Class:       AmplitudeClusterFinder
// Module Type: Submodule of ClusterFinder
// File:        AmplitudeClusterFinder.cc
//
// Generated at Tue Feb  5 11:21:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

/*
This is a submodule of ClusterFinder
Parameters are set by parameter list: amplitudeparams
Required Parameters:
  - fixed_width_ns
Optional Parameters:
  - threshold
  - pre_threshold_ns
  - verbosity
*/

#include "darkart/ODReco/Submodules/AmplitudeClusterFinder.hh"

// Constructors
AmplitudeClusterFinder::AmplitudeClusterFinder():
  _verbosity(0)
{}
AmplitudeClusterFinder::AmplitudeClusterFinder(int nsamps):
  _verbosity(0)
{
  nsamps += 0;
}

AmplitudeClusterFinder::AmplitudeClusterFinder(fhicl::ParameterSet const &p, int channel_type)
  : _verbosity(p.get<int>("verbosity",0))
  , _threshold(p.get<float>("threshold",0))
  , _fixed_width_ns(p.get<float>("fixed_width_ns",0))
  , _pre_threshold_ns(p.get<float>("pre_threshold_ns",0))
  , _channel_type(channel_type)
{
  if(_fixed_width_ns <= 0)
    throw cet::exception("AmplitudeClusterFinder") << "Invalid value for _fixed_width_ns = " << _fixed_width_ns << std::endl;

  _num_clusters        = 0;
}

void AmplitudeClusterFinder::findClusters(const darkart::od::SumWF& wf, 
					  darkart::od::ClusterVec& cluster_vec){
  const double sample_rate_GHz = _utils->getSampleRateGHz(_channel_type);
  const double sample_rate_Hz = _utils->getSampleRateHz(_channel_type);

  _num_clusters = 0;
  _fixed_width_samps   = int(_fixed_width_ns * sample_rate_GHz);
  _pre_threshold_samps = int(_pre_threshold_ns * sample_rate_GHz);
  _wf = wf;
  _wf_size = int(_wf.size());
  if(_wf_size == 0)
    throw cet::exception("AmplitudeClusterFinder") << "Input waveform to findClusters function is blank" << std::endl;

  _current_sample = 0;

  
  while(_current_sample >= 0 && _current_sample < _wf_size){
    findClusterStart();
    if(_current_sample >= _wf_size) break;

    // Step back by the pre threshold time
    _current_sample = (_current_sample > _pre_threshold_samps) ? _current_sample - _pre_threshold_samps : 0;
    // Create the cluster
    darkart::od::Cluster cluster;
    cluster.cluster_id = _num_clusters;
    cluster.start_sample = _current_sample;
    cluster.start_ns = _wf.at(_current_sample).sample_ns;
    _num_clusters++;
    
    // Loop over events in the cluster
    darkart::od::ChPulsePairVec a_ch_pulse_pair_vec;
    _samples_left_in_cluster = _fixed_width_samps;

    while( _samples_left_in_cluster > 0 && _current_sample < _wf_size){
      _samples_left_in_cluster--;

      // Integrate
      cluster.charge += _wf.at(_current_sample).amplitude;
      // Keep track of contributing pulses
      // Check pulse starts
      if(!_wf.at(_current_sample).ch_pulse_start_pairs.empty()){
	// Copy pairs into the vector
	a_ch_pulse_pair_vec.insert(a_ch_pulse_pair_vec.end(),
				   _wf.at(_current_sample).ch_pulse_start_pairs.begin(),
				   _wf.at(_current_sample).ch_pulse_start_pairs.end());
	
      }
      // Check pulse ends
      if(!_wf.at(_current_sample).ch_pulse_end_pairs.empty()){
	// Copy pairs into the set
	a_ch_pulse_pair_vec.insert(a_ch_pulse_pair_vec.end(),
				   _wf.at(_current_sample).ch_pulse_end_pairs.begin(),
				   _wf.at(_current_sample).ch_pulse_end_pairs.end());
	
      }
      
      _current_sample++;
    } // while _samples_left_in_cluster

    if(_current_sample == _wf_size) _current_sample--;
    
    removeDuplicatesFromChPulseVec(a_ch_pulse_pair_vec);
    cluster.end_sample = _current_sample;
    cluster.end_ns = _wf.at(_current_sample).sample_ns;
    cluster.ch_pulse_pairs = a_ch_pulse_pair_vec;
    cluster.charge /= sample_rate_Hz;

    
    // Find the cluster height
    darkart::od::SumWF::iterator peakit = std::max_element(_wf.begin()+cluster.start_sample,
							   _wf.begin()+cluster.end_sample,
							   [](const darkart::od::SumWFSample sampA, const darkart::od::SumWFSample sampB) -> bool
							   {
							     return sampA.amplitude < sampB.amplitude;
							   });
    cluster.peak_sample = peakit->sample;
    cluster.peak_ns = peakit->sample_ns;
    cluster.height = peakit->amplitude;

    // Find cluster max multiplicity
    darkart::od::SumWF::iterator mult_it = std::max_element(_wf.begin()+cluster.start_sample,
							    _wf.begin()+cluster.end_sample,
							    [](const darkart::od::SumWFSample sampA, const darkart::od::SumWFSample sampB) -> bool
							    {
							      return sampA.amplitude_discr < sampB.amplitude_discr;
							    });
    cluster.max_multiplicity = mult_it->amplitude_discr;
    cluster_vec.push_back(cluster);

  }
  
  
  if(_verbosity > 2){
    LOG_INFO("AmplitudeClusterFinder") << " N clusters = " << cluster_vec.size() << std::endl;
    for(size_t i = 0; i < cluster_vec.size(); i++){
      LOG_INFO("AmplitudeClusterFinder") << "\ti:\tcharge  = " << cluster_vec.at(i).charge 
					 << "\tstart_ns  = " << cluster_vec.at(i).start_ns 
					 << "\tend_ns    = " << cluster_vec.at(i).end_ns
					 << "\tnum pulses= " << cluster_vec.at(i).ch_pulse_pairs.size()
					 << "\tpeak_ns   = " << cluster_vec.at(i).peak_ns
					 << "\theight    = " << cluster_vec.at(i).height
					 << "\tmax_mult  = " << cluster_vec.at(i).max_multiplicity
					 << std::endl;
    }
  }

}

// Find the sample at which the waveform goes above threshold
void AmplitudeClusterFinder::findClusterStart(){
  while(_current_sample >= 0 && _current_sample < _wf_size &&
	_wf.at(_current_sample).amplitude < _threshold)
    _current_sample++;
}

// Remove duplicates from the the ChPulsePairVec before adding it to the new cluster
void AmplitudeClusterFinder::removeDuplicatesFromChPulseVec(darkart::od::ChPulsePairVec& vec){
  if(vec.size() <=1 ) return;
  std::sort(vec.begin(), vec.end(),
	    [](const darkart::od::ChPulsePair & pairA, const darkart::od::ChPulsePair & pairB) -> bool
	    {
	      return ((pairA.first > pairB.first) || 
		      (pairA.first == pairB.first && pairA.second > pairB.second));
	    });
  vec.erase(std::unique(vec.begin(),vec.end(),
			[](const darkart::od::ChPulsePair & pairA, const darkart::od::ChPulsePair & pairB) -> bool
			{
			  return (pairA.first == pairB.first && pairA.second == pairB.second);
			}),vec.end());
}
