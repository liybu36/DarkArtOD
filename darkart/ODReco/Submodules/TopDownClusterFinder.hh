////////////////////////////////////////////////////////////////////////
// Class:       TopDownClusterFinder
// Module Type: Submodule of ClusterFinder
// File:        TopDownClusterFinder.cc
//
// Generated at Tue Feb  5 11:21:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////
#ifndef darkart_ODReco_TopDownClusterFinder_hh
#define darkart_ODReco_TopDownClusterFinder_hh

#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <algorithm>
#include <iostream>
#include <map>
#include <utility>

#include "darkart/ODReco/Tools/Utilities.hh"
#include "darkart/ODProducts/Cluster.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"
#include "darkart/ODProducts/SumWF.hh"

class order_by_sample{
public:
  bool operator() (const float sampA, const float sampB) { return (sampA < sampB); }
};

class order_by_amplitude{
public:
  bool operator() (const darkart::od::SumWFSample sampA,
		   const darkart::od::SumWFSample sampB){
    if(sampA.amplitude == sampB.amplitude) { return sampA.sample < sampB.sample; }
    else { return sampA.amplitude > sampB.amplitude; }
  }
};

class TopDownClusterFinder {
private:
  // Useful typedefs
  typedef std::map<int,darkart::od::SumWFSample,order_by_sample> _time_ordered_wf_t;
  typedef std::map<darkart::od::SumWFSample,int,order_by_amplitude> _amplitude_ordered_wf_t;

public:
  // Constructor and Destructor
  TopDownClusterFinder();
  TopDownClusterFinder(int);
  TopDownClusterFinder(fhicl::ParameterSet const &p,int channel_type);
  ~TopDownClusterFinder(){;}

  // Primary Methods
  void findClusters(const darkart::od::SumWF&,darkart::od::ClusterVec&);

  // Helper methods
  void findClusterStart();
  void findClusterEnd(_time_ordered_wf_t::iterator);
  void removeDuplicatesFromChPulseVec(darkart::od::ChPulsePairVec&);

  // Getters
  int getVerbosity() const { return _verbosity; }
  // Setters
  void setVerbosity(const int v) { _verbosity = v; }
  void setUtilities(darkart::od::Utilities *u) { _utils = u; }

private:
  int _verbosity;
  int _channel_type;
  double _threshold;
  double _max_gap_ns;
  double _min_width_ns;
  darkart::od::Utilities *_utils;

  _time_ordered_wf_t::iterator _current_sample;

  // Copies of the time- and amplitude-ordered waveforms
  _time_ordered_wf_t _wf_time_ordered;
  _amplitude_ordered_wf_t _wf_amplitude_ordered;
};

#endif
