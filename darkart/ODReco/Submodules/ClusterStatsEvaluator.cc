////////////////////////////////////////////////////////////////////////
// Class:       ClusterStatsEvaluator
// Module Type: Submodule of ClusterEvaluator
// File:        ClusterStatsEvaluator.cc
//
// Generated at Tue Feb  5 12:58:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

/*
This is a submodule of ClusterEvaluator
Parameters are set by parameter list: shapeparams
Required Parameters:
  - (none)
Optional Parameters:
  - verbosity
*/

#include "darkart/ODReco/Submodules/ClusterStatsEvaluator.hh"

// Constructors
ClusterStatsEvaluator::ClusterStatsEvaluator():
  _verbosity(0)
{;}
ClusterStatsEvaluator::ClusterStatsEvaluator(int nsamps):
  _verbosity(0)
{
  nsamps += 0;
}

ClusterStatsEvaluator::ClusterStatsEvaluator(fhicl::ParameterSet const &p):
  //  _num_baseline_samples(p.get<int>("num_baseline_samples")),
  _verbosity(0)
{
  p.get<int>("dummy");
}

void ClusterStatsEvaluator::eval(const darkart::od::SumWF& wf,
				 const darkart::od::ChannelDataVec& channel_data_vec,
				 const darkart::od::ChannelWFsVec& channel_wfs_vec,
				 darkart::od::ClusterVec& cluster_vec){
  // Doesn't do anything yet
  wf.size();
  channel_data_vec.size();
  channel_wfs_vec.size();
  cluster_vec.size();
}
