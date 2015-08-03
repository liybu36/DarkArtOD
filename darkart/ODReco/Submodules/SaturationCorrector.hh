////////////////////////////////////////////////////////////////////////
// Class:       SaturationCorrector
// Module Type: Submodule of PulseCorrector
// File:        SaturationCorrector.cc
//
// Generated at Tue Feb  3 08:28:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

#include "darkart/ODReco/Tools/Utilities.hh"
#include "darkart/ODProducts/ODEventInfo.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"

class SaturationCorrector {
public:
  // Constructor and Destructor
  SaturationCorrector(fhicl::ParameterSet const &p);
  ~SaturationCorrector(){;}

  // Primary Methods
  void isSaturated(darkart::od::ChannelWFs::PulseWF&,
		   darkart::od::ChannelData::Pulse&,
		   const double&,
		   const double&,
		   const int&);

  // Getters
  bool  getReplaceWFWithSatTrig()  const { return _replace_wf_with_sat_trig; }
  int   getMinConsecutiveSamples() const { return _min_consecutive_samples;  }
  int   getVerbosity()             const { return _verbosity;                }
  float getMinVoltage()            const { return _min_voltage;              }
  bool  getIsSaturated()           const { return _is_saturated;             }
  float getCorrection ()           const { return _correction;               }

  // Setters
  void setUtilities(darkart::od::Utilities *u) { _utils = u; }

private:
  bool  _replace_wf_with_sat_trig;
  int   _min_consecutive_samples;
  int   _verbosity;
  float _min_voltage;
  bool  _is_saturated; 
  float _correction;

  darkart::od::Utilities *_utils;
};
