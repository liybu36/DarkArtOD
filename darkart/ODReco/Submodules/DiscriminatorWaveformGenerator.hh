////////////////////////////////////////////////////////////////////////
// Class:       DiscriminatorWaveformGenerator
// Module Type: Submodule of SumWaveformGenerator
// File:        DiscriminatorWaveformGenerator.cc
//
// Generated at Tue Feb  5 03:43:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////
#ifndef darkart_ODReco_DiscrimintaorWaveformGenerator_hh
#define darkart_ODReco_DiscriminatorWaveformGenerator_hh

#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <algorithm>
#include <iostream>

#include "darkart/ODReco/Tools/Utilities.hh"
#include "darkart/ODProducts/ODEventInfo.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"
#include "darkart/ODProducts/SumWF.hh"

class DiscriminatorWaveformGenerator {
public:
  // Constructor and Destructor
  DiscriminatorWaveformGenerator();
  DiscriminatorWaveformGenerator(int);
  DiscriminatorWaveformGenerator(fhicl::ParameterSet const&,int);
  ~DiscriminatorWaveformGenerator(){;}

  // Primary Methods
  void sum(const darkart::od::ChannelDataVec&,darkart::od::SumWF&,int);

  // Getters
  int getVerbosity()       const { return _verbosity;       }
  int getChannelType()     const { return _channel_type;    }
  int getZSPresamples()    const { return _zs_presamples;   }
  int getZSPostsamples()   const { return _zs_postsamples;  }
  float getWidthNs()       const { return _width_ns;        }
  float getSampleRateGHz() const { return _sample_rate_GHz; }
  // Setters
  void setVerbosity(const int v)            { _verbosity = v;       }
  void setChannelType(const int t)          { _channel_type = t;    } 
  void setWidthNs(const float w)            { _width_ns = w;        }
  void setSampleRateGHz(const float r)      { _sample_rate_GHz = r; }
  void setUtils(darkart::od::Utilities *_u) { _utils = _u;          }

private:
  // From FHiCL file
  int _verbosity;
  int _channel_type;
  float _width_ns;

  // Calculated or input
  darkart::od::Utilities *_utils;
  int _width_samps; 
  int _zs_presamples;
  int _zs_postsamples;
  float _sample_rate_GHz;
  
};

#endif
