////////////////////////////////////////////////////////////////////////
// Class:       ClusterStatsEvaluator
// Module Type: Submodule of ClusterFinder
// File:        ClusterStatsEvaluator.cc
//
// Generated at Tue Feb  5 11:21:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////
#ifndef darkart_ODReco_ClusterStatsEvaluator_hh
#define darkart_ODReco_ClusterStatsEvaluator_hh

#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

#include "darkart/ODProducts/Cluster.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"
#include "darkart/ODProducts/SumWF.hh"

class ClusterStatsEvaluator {
public:
  // Constructor and Destructor
  ClusterStatsEvaluator();
  ClusterStatsEvaluator(int);
  ClusterStatsEvaluator(fhicl::ParameterSet const &p);
  ~ClusterStatsEvaluator(){;}

  // Primary Methods
  void eval(const darkart::od::SumWF&,const darkart::od::ChannelDataVec&,const darkart::od::ChannelWFsVec&,darkart::od::ClusterVec&);

  // Getters
  int getVerbosity() { return _verbosity; }
  // Setters
  void setVerbosity(const int v) { _verbosity = v; }

private:
  int _verbosity;
};

#endif
