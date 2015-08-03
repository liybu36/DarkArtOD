////////////////////////////////////////////////////////////////////////
// Class:       TopDownWaveformGenerator
// Module Type: Submodule of SumWaveformGenerator
// File:        TopDownClusterFinder.cc
//
// Generated at Tue Feb  3 03:43:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

/*
This is a submodule of ClusterFinder
Parameters are set by parameter list: amplitudesumparams
Required Parameters:
  - max_gap_ns
Optional Parameters:
  - threshold
  - min_width_ns
  - verbosity
*/

#include "darkart/ODReco/Submodules/TopDownClusterFinder.hh"

// Constructors
TopDownClusterFinder::TopDownClusterFinder():
  _verbosity(0)
{;}
TopDownClusterFinder::TopDownClusterFinder(int nsamps):
  _verbosity(0)
{
  nsamps += 0;
}

TopDownClusterFinder::TopDownClusterFinder(fhicl::ParameterSet const &p, int channel_type)
  : _verbosity(p.get<int>("verbosity",0))
  , _channel_type(channel_type)
  , _threshold(p.get<double>("threshold",0))
  , _max_gap_ns(p.get<double>("max_gap_ns",0))
  , _min_width_ns(p.get<double>("min_width_ns",0))
{}

void TopDownClusterFinder::findClusters(const darkart::od::SumWF& wf,
					darkart::od::ClusterVec& cluster_vec){
  // Copy the input waveform into two new waveforms: time- and charge-sorted
  _wf_time_ordered.clear();
  _wf_amplitude_ordered.clear();

  const double sample_rate_Hz = _utils->getSampleRateHz(_channel_type);

  // Sort the charge-sorted wf (time-sorted should already be sorted)
  // Fill the time- and charge-ordered waveform maps
  for(size_t samp = 0; samp < wf.size(); samp++){
    _wf_time_ordered[samp] = wf[samp];
    _wf_amplitude_ordered[wf[samp]] = samp;
  }

  _time_ordered_wf_t::iterator cluster_start_it, cluster_end_it;
  int cluster_peak_sample, cluster_start_sample, cluster_end_sample;
  int cluster_max_multiplicity;
  double cluster_peak_ns, cluster_start_ns, cluster_end_ns;
  double cluster_integral, cluster_height;
  darkart::od::ChPulsePairVec cluster_ch_pulse_pair_vec;
  while(!_wf_time_ordered.empty() && !_wf_amplitude_ordered.empty()){
    // Reset cluster parameters
    cluster_peak_sample = -1;
    cluster_start_sample = -1;
    cluster_end_sample = -1;
    cluster_peak_ns = -1;
    cluster_start_ns = -1;
    cluster_end_ns = -1;
    cluster_integral = 0;
    cluster_max_multiplicity = 0;
    cluster_height = 0;
    cluster_ch_pulse_pair_vec.clear();
    
    // Check if the highest point is below the threshold
    std::pair<darkart::od::SumWFSample,int> next_charge = *_wf_amplitude_ordered.begin();
    if(next_charge.first.amplitude < _threshold) break;

    // Find the first peak of the waveform and go there
    _current_sample = _wf_time_ordered.find(next_charge.second);
    // Find the matching sample in the time-ordered waveform
    cluster_peak_sample = next_charge.second;
    cluster_peak_ns = next_charge.first.sample_ns;
    cluster_height = next_charge.first.amplitude;

    // Search for the start of the cluster to the left of the peak
    findClusterStart();
    cluster_start_it = _current_sample;
    cluster_start_sample = _current_sample->first;
    cluster_start_ns = (_current_sample->second).sample_ns;

    // Search for the end of the cluster to the right of the peak
    _current_sample = _wf_time_ordered.find(cluster_peak_sample);
    findClusterEnd(cluster_start_it);
    cluster_end_it = _current_sample;
    cluster_end_sample = _current_sample->first;
    cluster_end_ns = (_current_sample->second).sample_ns;   

    // Loop over the cluster from start to end, calculating quantities
    for(_current_sample = cluster_start_it; 
	_current_sample->first <= cluster_end_it->first &&
	  _current_sample != _wf_time_ordered.end();
	_current_sample++){
      cluster_integral += (_current_sample->second).amplitude;
      if((_current_sample->second).amplitude_discr > cluster_max_multiplicity)
	cluster_max_multiplicity = (_current_sample->second).amplitude_discr;

      // Add ch_pulse_pairs
      // Check pulse starts
      if(!(_current_sample->second).ch_pulse_start_pairs.empty() &&
	 (_current_sample->second).amplitude != 0){
	cluster_ch_pulse_pair_vec.insert(cluster_ch_pulse_pair_vec.end(),
					 (_current_sample->second).ch_pulse_start_pairs.begin(),
					 (_current_sample->second).ch_pulse_start_pairs.end());
      }
      // Check pulse ends
      if(!(_current_sample->second).ch_pulse_end_pairs.empty() &&
	 (_current_sample->second).amplitude){
	// Copy pairs into the vector
	cluster_ch_pulse_pair_vec.insert(cluster_ch_pulse_pair_vec.end(),
					 (_current_sample->second).ch_pulse_end_pairs.begin(),
					 (_current_sample->second).ch_pulse_end_pairs.end());
      }      
      
      // Delete this sample from the _wf_amplitude_ordered so it doesn't get double counted
      _wf_amplitude_ordered.erase(_current_sample->second);
      
      if(_wf_amplitude_ordered.empty()) break;            
    } // End loop over cluster

    // Remove duplicate ch_pulse_pairs
    removeDuplicatesFromChPulseVec(cluster_ch_pulse_pair_vec);

    // Find cluster max multiplicity
    _time_ordered_wf_t::iterator mult_it;
    mult_it = std::max_element(cluster_start_it,
			       cluster_end_it,
			       [](const std::pair<int, darkart::od::SumWFSample> sampA, 
				  const std::pair<int, darkart::od::SumWFSample> sampB)
			       -> bool 
			       {
				 return (sampA.second).amplitude_discr < (sampB.second).amplitude_discr;
			       });
    cluster_max_multiplicity = (mult_it->second).amplitude_discr;

    // Package values up into a cluster and add the cluster to the cluster vec
    darkart::od::Cluster cluster;
    cluster.start_sample = cluster_start_sample;
    cluster.end_sample = cluster_end_sample;
    cluster.peak_sample = cluster_peak_sample;
    cluster.start_ns = cluster_start_ns;
    cluster.end_ns = cluster_end_ns;
    cluster.peak_ns = cluster_peak_ns;
    cluster.charge = cluster_integral/sample_rate_Hz;
    cluster.height = cluster_height;
    cluster.ch_pulse_pairs = cluster_ch_pulse_pair_vec;
    cluster.max_multiplicity = cluster_max_multiplicity;
    cluster.cluster_id = -1; // -1 for now, we'll fill it in when we sort the clusters
    cluster_vec.push_back(cluster);

    // Remove these points from the _wf_time_ordered
    // Temporarily increment the iterator by 1 to include it in the stuff that gets erased
    cluster_end_it++;
    _wf_time_ordered.erase(cluster_start_it, cluster_end_it);
    cluster_end_it--;
  } // End cluster-finding loop
  
  // Sort clusters by start time
  std::sort(cluster_vec.begin(),
	    cluster_vec.end(),
	    [](darkart::od::Cluster clusterA, darkart::od::Cluster clusterB){
	      return clusterA.start_ns < clusterB.start_ns;
	    });

  // Assign IDs to clusters
  for(size_t clust_id = 0; clust_id < cluster_vec.size(); clust_id++)
    cluster_vec[clust_id].cluster_id = clust_id;

  if(_verbosity > 2){
    LOG_INFO("TopDownClusterFinder") << " N clusters = " << cluster_vec.size() << std::endl;
    for(size_t i = 0; i < cluster_vec.size(); i++){
      LOG_INFO("TopDownClusterFinder") << "\ti:\t" << cluster_vec.at(i).charge
				       << "\t" << cluster_vec.at(i).height
				       << "\t" << cluster_vec.at(i).start_ns 
				       << "\t" << cluster_vec.at(i).end_ns-cluster_vec.at(i).start_ns
				       << "\t" << cluster_vec.at(i).max_multiplicity
				       << "\t" << cluster_vec.at(i).ch_pulse_pairs.size()
				       << std::endl;
    }
  }
}


void TopDownClusterFinder::findClusterStart(){
  bool found_start = false;
  do{
    // Check to see if you are ready to end the search for cluster start
    // If you are at the start of the waveform, you're done
    if(_current_sample->first <= _wf_time_ordered.begin()->first) found_start = true;
    else{
      // Otherwise, calculate the size of the gap between the current point
      // and the next nonzero point
      _time_ordered_wf_t::iterator check_sample = _current_sample;
      do{
	if(check_sample->first <= _wf_time_ordered.begin()->first ||
	   _current_sample->second.sample_ns - check_sample->second.sample_ns > _max_gap_ns){
	  found_start = true;
	  break;
	}
	check_sample--;
      }while((check_sample->second).amplitude == 0);
      
      // Is this gap larger than the max gap? 
      // If so, the point before the gap is the cluster start
      const double gap = (_current_sample->second).sample_ns - (check_sample->second).sample_ns;
      if(!found_start &&
	 gap > _max_gap_ns){
	found_start = true;
      }
    }

    if(_current_sample == _wf_time_ordered.begin()) break;
    if(!found_start) _current_sample--;
  }while(!found_start);
}

void TopDownClusterFinder::findClusterEnd(_time_ordered_wf_t::iterator cluster_start_sample){
  bool found_end = false;
  do{
    // Check to see if you are ready to end the search for cluster end
    // If you are at the start of the waveform, you're done
    if(_current_sample == _wf_time_ordered.end()) {
      _current_sample--;
      break;
    }
    if(_current_sample->first >= _wf_time_ordered.rbegin()->first) found_end = true;

    // Only search for the end if ending here would make the cluster wider than
    // the minimum width
    else if((_current_sample->second).sample_ns -
	    (cluster_start_sample->second).sample_ns > _min_width_ns){
      // Otherwise, calculate the size fo the gap between the current point
      // and the next nonzero point
      _time_ordered_wf_t::iterator check_sample = _current_sample;
      do{
	if(check_sample->first >= _wf_time_ordered.rbegin()->first){
	  found_end = true;
	  break;
	}
	check_sample++;
      }while((check_sample->second).amplitude == 0);
      
      // Is this gap larger than the max gap? 
      // If so, the point before the gap is the cluster start
      if(!found_end &&
	 (check_sample->second).sample_ns - 
	 (_current_sample->second).sample_ns > _max_gap_ns)
	found_end = true;
      else found_end = false;
    }

    if(!found_end) _current_sample++;
  }while(!found_end);
}

// Remove duplicates from the the ChPulsePairVec before adding it to the new cluster
void TopDownClusterFinder::removeDuplicatesFromChPulseVec(darkart::od::ChPulsePairVec& vec){
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
