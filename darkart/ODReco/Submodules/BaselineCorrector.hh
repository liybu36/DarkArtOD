////////////////////////////////////////////////////////////////////////
// Class:       BaselineCorrector
// Module Type: Submodule of PulseCorrector
// File:        BaselineCorrector.cc
//
// Generated at Tue Feb  3 08:28:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

#include "darkart/ODReco/Tools/Utilities.hh"
#include "darkart/ODProducts/ODEventInfo.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"

class BaselineCorrector {
public:
  // Constructor and Destructor
  BaselineCorrector();
  BaselineCorrector(int);
  BaselineCorrector(fhicl::ParameterSet const &p);
  ~BaselineCorrector(){;}

  // Primary Methods
  // Note: unless _pedestal_mean and _pedestal_rms are set by hand
  // FindBaseline() must be run before SubtractBaseline()
  void findBaseline       (darkart::od::ChannelWFs::PulseWF&);
  void subtractBaseline   (darkart::od::ChannelWFs::PulseWF&);
  void integrate          (darkart::od::ChannelWFs::PulseWF&,
                           darkart::od::ChannelData::Pulse&,
			   int);
  void calculateAmplitude (darkart::od::ChannelData::Pulse&);

  // Getters
  int   getVerbosity()          const { return _verbosity;            }
  int   getNumBaselineSamples() const { return _num_baseline_samples; }
  float getPedestalMean()       const { return _pedestal_mean;        }
  float getPedestalRMS()        const { return _pedestal_rms;         }
  float getIntegral()           const { return _integral;             }
  float getAmplitude()          const { return _amplitude;            }
  // Setters
  void setVerbosity          (const int v)      { _verbosity = v;            }
  void setNumBaselineSamples (const int n)      { _num_baseline_samples = n; }
  void setPedestalMean       (const float mean) { _pedestal_mean = mean;     }
  void setPedestalRMS        (const float rms)  { _pedestal_rms = rms;       }
  void setIntegral           (const float i)    { _integral = i;             }
  void setAmplitude          (const float a)    { _amplitude = a;            }
  void setUtilities          (darkart::od::Utilities *u) { _utils = u;}

private:
  int   _verbosity;
  int   _num_baseline_samples;
  float _pedestal_mean;
  float _pedestal_rms;
  float _integral;
  float _amplitude;

  darkart::od::Utilities *_utils;
};
