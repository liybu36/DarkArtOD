////////////////////////////////////////////////////////////////////////
// Class:       CoerceCorrector
// Module Type: Submodule of PulseCorrector
// File:        CoerceCorrector.cc
//
// Generated at Tue Apr  14 08:28:02 2015 by Stefano Davini.
////////////////////////////////////////////////////////////////////////

#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

#include "darkart/ODReco/Tools/Utilities.hh"
#include "darkart/ODProducts/ODEventInfo.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"

class CoerceCorrector {
public:
  // Constructor and Destructor
  CoerceCorrector();
  CoerceCorrector(fhicl::ParameterSet const &p, int verbosity);
  ~CoerceCorrector(){;}

  // Primary Methods
  void coerce             (darkart::od::ChannelWFs::PulseWF&, int ch_id);

  // Getters
  int   getVerbosity()          const { return _verbosity;            }
  float getCoerceMax() 		const { return _coerce_max; 	      }
  float getCoerceMin() 		const { return _coerce_min; 	      }
  bool  getCoerced()            const { return _coerced;              }
  float getIntegral()           const { return _integral;             }
  float getAmplitude()          const { return _amplitude;            }
  int   getPeakSample()         const { return _peak_sample;          }
  // Setters
  void setVerbosity          (const int v)      { _verbosity  = v;           }
  void setCoerceMax          (const float max)  { _coerce_max = max;         }
  void setCoerceMin          (const float min)  { _coerce_min = min;         }
  void setUtilities          (darkart::od::Utilities *u) { _utils = u;}

private:
  int   _verbosity;
  float _coerce_max;
  float _coerce_min;
  bool  _coerced;
  float _integral;
  float _amplitude;
  int   _peak_sample;

  darkart::od::Utilities *_utils;
};
