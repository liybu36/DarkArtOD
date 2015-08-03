////////////////////////////////////////////////////////////////////////
// Class:       AmplitudeClusterFinder
// Module Type: Submodule of ClusterFinder
// File:        AmplitudeClusterFinder.cc
//
// Generated at Tue Feb  5 11:21:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////
#ifndef darkart_ODReco_AmplitudeClusterFinder_hh
#define darkart_ODReco_AmplitudeClusterFinder_hh

#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <iostream>

#include "darkart/ODReco/Tools/Utilities.hh"
#include "darkart/ODProducts/Cluster.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"
#include "darkart/ODProducts/SumWF.hh"

class AmplitudeClusterFinder {
public:
  // Constructor and Destructor
  AmplitudeClusterFinder();
  AmplitudeClusterFinder(int);
  AmplitudeClusterFinder(fhicl::ParameterSet const&,int);
  ~AmplitudeClusterFinder(){;}

  // Primary Methods
  void findClusters(const darkart::od::SumWF&,
		    darkart::od::ClusterVec&);

  // Helper methods
  void findClusterStart();
  void removeDuplicatesFromChPulseVec(darkart::od::ChPulsePairVec&);

  // Getters
  int   getVerbosity    ()   const { return _verbosity;       }
  float getThreshold    ()   const { return _threshold;       }
  float getFixedWidthNs ()   const { return _fixed_width_ns;  }
  // Setters
  void setVerbosity    (const int v  ) { _verbosity = v;       }
  void setThreshold    (const float t) { _threshold = t;       }
  void setFixedWidthNs (const float w) { _fixed_width_ns = w;  }
  void setUtilities    (darkart::od::Utilities *u) { _utils = u; }

private:
  // Values fed to class from FHiCL
  int   _verbosity;
  float _threshold;
  float _fixed_width_ns;
  float _pre_threshold_ns;

  // Values set from parent class
  int _channel_type;
  darkart::od::Utilities *_utils;

  // Values calculated from waveform
  darkart::od::SumWF _wf;
  int _wf_size;
  int _current_sample;
  int _samples_left_in_cluster;

  // Other calculated values
  int _fixed_width_samps;
  int _pre_threshold_samps;
  int _num_clusters;

};
#endif
