////////////////////////////////////////////////////////////////////////
// Class:       ClusterPMTDistributionEvaluator
// Module Type: Submodule of ClusterFinder
// File:        ClusterPMTDistributionEvaluator.cc
//
// Generated at Tue Feb  5 11:21:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////
#ifndef darkart_ODReco_ClusterPMTDistributionEvaluator_hh
#define darkart_ODReco_ClusterPMTDistributionEvaluator_hh

#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

#include "darkart/ODReco/Tools/Utilities.hh"
#include "darkart/ODProducts/Cluster.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"
#include "darkart/ODProducts/SumWF.hh"

class ClusterPMTDistributionEvaluator {
public:
  // Constructor and Destructor
  ClusterPMTDistributionEvaluator();
  ClusterPMTDistributionEvaluator(int);
  ClusterPMTDistributionEvaluator(fhicl::ParameterSet const &p);
  ~ClusterPMTDistributionEvaluator(){;}

  // Primary Methods
  void eval(const darkart::od::ChannelDataVec&, const darkart::od::ChannelWFsVec&,darkart::od::ClusterVec&, int);

  // Getters
  int getVerbosity() const { return _verbosity; }
  // Setters
  void setVerbosity(const int v) { _verbosity = v; }
  void setUtilities    (darkart::od::Utilities *u) { _utils = u; }

private:
  int _verbosity;
  int _channel_type;

  darkart::od::Utilities *_utils;

};

#endif
