#ifndef darkart_TpcReco_converter_algs_hh
#define darkart_TpcReco_converter_algs_hh

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/Fragments.hh"

#include "darksidecore/Data/V172xFragment.hh"
#include "darksidecore/Data/V1495Fragment.hh"

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/EventInfo.hh"

#include <vector>

namespace darkart
{
  // Need to keep track of trigger timing from event to event, so put these
  // these functions in a class and keep one instance of the class in the
  // Converter producer module.
  
  class ConverterAlgs {
  public:
    ConverterAlgs() { }

    // Convert the data for one channel held in a Fragment (as integral
    // raw ADC counts) to the values stored as floating-point
    // numbers. 
    std::vector<double>
    convertWaveform(ds50::V172xFragment::adc_type const * begin,
                    ds50::V172xFragment::adc_type const * end);


    // Dedicated function to compute the global channel ID.
    // For now this is just the usual channel ID calculation of
    // 8*boardID+board_ch_num. It is very possible that the global channel
    // IDs will be completely redefined in software in the future.
    int channelID(int board_id, int board_size, int ch_num);
    

    // Fill the Channel object with channel-level identifier info. Some
    // of the info is stored in the V172xFragment metadata, which cannot be
    // accessed directly from the V172xFragment object?! So pass in the
    // metadata separately.
    darkart::Channel
    fillChannelInfo(ds50::V172xFragment const& overlay,
                    ds50::V172xFragment::metadata const& md,
                    int ch_num,
                    darkart::WaveformInfo const& wfmInfo);

    
    // Loop over all one artdaq::Fragments. Translate each artdaq::Fragment
    // into a series of darkart::Channel and darkart::Waveform objects.
    void
    convertFragments(artdaq::Fragments const& frags,
                     darkart::WaveformVec& wfmVec,
                     darkart::WaveformInfoVec& wfmInfoVec,
                     darkart::ChannelVec& chVec,
                     std::vector<int> skip_ch);


    // Fill the EventInfo object with event-level identifier info. A lot of
    // the information is stored in a V1495Fragment, so must pass that in.
    darkart::EventInfo
    fillEventInfo(artdaq::Fragment const& frag,
                  darkart::ChannelVec const& chVec);

  private:
    uint64_t          start_time_; //start time of the run in us
    uint64_t previous_event_time_; //time of the previous event in us
  };

}

#endif
