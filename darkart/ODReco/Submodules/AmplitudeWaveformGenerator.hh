////////////////////////////////////////////////////////////////////////
// Class:       AmplitudeWaveformGenerator
// Module Type: Submodule of SumWaveformGenerator
// File:        AmplitudeWaveformGenerator.cc
//
// Generated at Tue Feb  5 03:43:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

#ifndef AmplitudeWaveformGenerator_hh
#define AmplitudeWaveformGenerator_hh

#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <utility>
#include <vector>

#include "darkart/ODProducts/ODEventInfo.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"
#include "darkart/ODProducts/SumWF.hh"

class AmplitudeWaveformGenerator {
public:
  // Constructor and Destructor
  AmplitudeWaveformGenerator();
  AmplitudeWaveformGenerator(int);
  AmplitudeWaveformGenerator(fhicl::ParameterSet const&,int);
  ~AmplitudeWaveformGenerator(){;}

  // Primary Methods
  void sum(const darkart::od::ChannelDataVec&,
           const darkart::od::ChannelWFsVec&,
           darkart::od::SumWF&, int);

  // Getters
  int getVerbosity() const { return _verbosity; }
  // Setters
  void setVerbosity(const int v) { _verbosity = v; }

private:
  int _verbosity;
  int _channel_type;
};

#endif
