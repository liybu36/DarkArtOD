////////////////////////////////////////////////////////////////////////
// Class:       ClusterPMTDistributionEvaluator
// Module Type: Submodule of ClusterEvaluator
// File:        ClusterPMTDistributionEvaluator.cc
//
// Generated at Tue Feb  3 12:58:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

/*
This is a submodule of ClusterEvaluator
Parameters are set by parameter list: shapeparams
Required Parameters:
  - (none)
Optional Parameters:
  - verbosity
*/

#include "darkart/ODReco/Submodules/ClusterPMTDistributionEvaluator.hh"

// Constructors
ClusterPMTDistributionEvaluator::ClusterPMTDistributionEvaluator():
  _verbosity(0)
{;}
ClusterPMTDistributionEvaluator::ClusterPMTDistributionEvaluator(int nsamps):
  _verbosity(0)
{
  nsamps += 0;
}

ClusterPMTDistributionEvaluator::ClusterPMTDistributionEvaluator(fhicl::ParameterSet const &p):
  _verbosity(0)
,_channel_type  (p.get<int>        ( "channel_type", 1 ))
{
  p.get<int>("dummy");   // ???????
}

void ClusterPMTDistributionEvaluator::eval(const darkart::od::ChannelDataVec& channel_data_vec,
					   const darkart::od::ChannelWFsVec& channel_wfs_vec,
					   darkart::od::ClusterVec& cluster_vec, int cl_idx){
  // Doesn't do anything yet
  const float ene = cluster_vec.at(cl_idx).charge;
  channel_data_vec.size();
  channel_wfs_vec.size();

  //Define the new struct to fill for PMTDistribution
  darkart::od::Cluster::PMTDistribution* pmt_distr = new darkart::od::Cluster::PMTDistribution();


  //Sometimes there are... I don't know why cluster.pairs that are empty, this is why there is this first check

  std::vector<std::pair<int,int>> ch_pairs = cluster_vec.at(cl_idx).ch_pulse_pairs;
  if (ch_pairs.size() == 0) {// TODO: fill with defult unphysical values and return
    std::cout<<"Warning: ch_pulse_pairs with null size at cluster " << cl_idx << " with charge " <<ene<<std::endl; // TODO: use art info printout instead of std::cout
    return;
  }
   
  // Copy the channel_id of the pairs into a set to avoid double copies of the same channel
  // std::set orders from the smallest to the biggest channel_id
  std::set<int> active_ch_set;
  for(auto it=ch_pairs.begin(); it!=ch_pairs.end(); ++it){
    active_ch_set.insert((*it).first);  // insert the channel ID of any channel with a pulse; std::set avoids copies
  }

  //Since I need to access the single element of active_ch_set, I have to copy it into a vector
  const size_t active_ch_size = active_ch_set.size();
  std::vector<int> active_ch_vec(active_ch_size);
  std::copy(active_ch_set.begin(), active_ch_set.end(), active_ch_vec.begin());

  // Create a vector of sets which holds the pulses_id
  // the vector index is the channel number
  std::vector<std::set<int>> ch_pulse_id;
  for(auto it_ach=active_ch_set.begin(); it_ach!=active_ch_set.end(); ++it_ach){
    std::set<int> pulses_ids;
    for(size_t pair_idx = 0; pair_idx<ch_pairs.size(); pair_idx++){
      // is channel ID of ch_pairs equal to the active channel in the top iteration?
      if(ch_pairs.at(pair_idx).first == (*it_ach)){  
        pulses_ids.insert(ch_pairs.at(pair_idx).second);
      }
    }
    ch_pulse_id.push_back(std::move(pulses_ids));
  }

  //Now acces to each channel and calculate their total integral within the cluster (channel_ids have increasing order)
  float pmt_charge_mean     = 0.;
  std::vector<float> ch_charge_vec; // index is active channel
  for(size_t ach_it=0; ach_it<active_ch_size; ach_it++){
    float ch_charge = 0.;
    const int channel_id = active_ch_vec.at(ach_it);
    for(size_t pulses_idx =0; pulses_idx<ch_pulse_id.at(ach_it).size(); pulses_idx++){
      ch_charge += channel_data_vec.at(channel_id).pulses.at(pulses_idx).integral;
    }
    //Create  a vector with all the charges for each active channel for this cluster and calculate the total integral for this cluster
    ch_charge_vec.push_back(ch_charge);
    pmt_charge_mean += ch_charge;
  }

  //Access the max element of ch_charge_vec to get the max_channel and the fraction of the total integral for that channel
  const float max_ch_fraction =  *std::max_element(ch_charge_vec.begin(), ch_charge_vec.end()) / pmt_charge_mean;
  const int   max_ch_idx = std::distance(ch_charge_vec.begin(), std::max_element(ch_charge_vec.begin(), ch_charge_vec.end()));
  const int   max_ch = active_ch_vec.at(max_ch_idx);

  pmt_charge_mean /= active_ch_size;

  //Here calculate the variance relative to pmt_charge_mean
  float pmt_charge_variance = 0.;
  for(size_t ch_idx=0; ch_idx<active_ch_size; ch_idx++){
    pmt_charge_variance += std::pow((ch_charge_vec.at(ch_idx)-pmt_charge_mean),2.);
  }
  if (active_ch_size > 1) pmt_charge_variance /= (active_ch_size -1);
  else pmt_charge_variance = 0.;

//	std::cout<<"MC MF: " << max_ch << " " << max_ch_fraction << " " << pmt_charge_mean << " " << pmt_charge_variance << std::endl;

  //Write the new variables in the copy of cluster_vec, that can be put into the final product changing the fhicl file
  pmt_distr->max_ch = max_ch;
  pmt_distr->max_ch_fraction = max_ch_fraction;
  pmt_distr->pmt_charge_mean = pmt_charge_mean;
  pmt_distr->pmt_charge_variance = pmt_charge_variance;
  cluster_vec.at(cl_idx).pmt_distr = std::move(pmt_distr);
}
