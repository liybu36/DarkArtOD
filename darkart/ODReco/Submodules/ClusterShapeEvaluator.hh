////////////////////////////////////////////////////////////////////////
// Class:       ClusterShapeEvaluator
// Module Type: Submodule of ClusterFinder
// File:        ClusterShapeEvaluator.cc
//
// Generated at Tue Feb  5 11:21:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////
#ifndef darkart_ODReco_ClusterShapeEvaluator_hh
#define darkart_ODReco_ClusterShapeEvaluator_hh

#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

#include "darkart/ODReco/Tools/Utilities.hh"
#include "darkart/ODProducts/Cluster.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"
#include "darkart/ODProducts/SumWF.hh"

class ClusterShapeEvaluator {
public:
  // Constructor and Destructor
  ClusterShapeEvaluator();
  ClusterShapeEvaluator(int);
  ClusterShapeEvaluator(fhicl::ParameterSet const &p);
  ~ClusterShapeEvaluator(){;}

  // Primary Methods
  void eval(const darkart::od::SumWF&,darkart::od::ClusterVec&, size_t);

  // Getters
  int getVerbosity() const { return _verbosity; }

  // Setters
  void setVerbosity (const int v) { _verbosity = v; }
  void setUtilities (darkart::od::Utilities *u) { _utils = u; }

private:
  int         _verbosity;
  std::string _mode;
  float       _threshold;
  int         _channel_type;

  darkart::od::Utilities *_utils;
};

#endif
