#include "darkart/TpcReco/converter_algs.hh"
#include "darkart/TpcReco/common_algs.hh"
#include "darksidecore/Data/Config.hh"
#include "art/Utilities/Exception.h"
#include "artdaq-core/Data/Fragment.hh"

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>

//----------------------------------------------------------------------------
std::vector<double>
darkart::ConverterAlgs::convertWaveform(ds50::V172xFragment::adc_type const * begin,
                                        ds50::V172xFragment::adc_type const * end)
{
  std::vector<double> wave;
  wave.reserve(end-begin);
  for ( ; begin != end; ++begin)
    wave.push_back(*begin);
  return wave;
}


//----------------------------------------------------------------------------
int darkart::ConverterAlgs::channelID(int board_id, int board_size, int board_ch_num)
{
  if (board_id < 0 || board_size <= 0 || board_ch_num < 0)
    return -1;
  
  return board_size*board_id+board_ch_num;
}


//----------------------------------------------------------------------------
darkart::Channel
darkart::ConverterAlgs::fillChannelInfo(ds50::V172xFragment const& overlay,
                                        ds50::V172xFragment::metadata const& md,
                                        int board_ch_num,
                                        darkart::WaveformInfo const& wfmInfo)
{
  darkart::Channel ch;
  ch.board_id      = overlay.board_id();
  ch.channel_num   = board_ch_num;
  ch.channelID.channel_id = channelID(overlay.board_id(),
                                      overlay.enabled_channels(),
                                      board_ch_num);
  ch.trigger_count = overlay.event_counter();
  ch.sample_bits   = md.sample_bits;
  ch.sample_rate   = md.sample_rate_MHz;
  ch.trigger_index = (int)(md.record_length - md.record_length*md.post_trigger/100.);
  ch.nsamps        = wfmInfo.nsamps;
  ch.saturated     = (wfmInfo.minimum == 0 ||
                      wfmInfo.maximum == ch.GetVerticalRange());
  
  return ch;
}


//----------------------------------------------------------------------------
void darkart::ConverterAlgs::convertFragments(artdaq::Fragments const& frags,
                                              darkart::WaveformVec& wfmVec,
                                              darkart::WaveformInfoVec& wfmInfoVec,
                                              darkart::ChannelVec& chVec,
                                              std::vector<int> skip_ch)
{
  for (auto const& frag : frags) {

    // Use the overlay to allow us to read channels
    ds50::V172xFragment overlay(frag);

    // Loop over all channels in the fragment 
    for (size_t ch_num=0; ch_num<overlay.enabled_channels(); ++ch_num) {

      // Some of the info is in V172xFragment metadata
      ds50::V172xFragment::metadata const md =
        *(frag.metadata<ds50::V172xFragment::metadata>());



      // If told to skip this channel, don't do anything
      int channel_id = channelID(overlay.board_id(),
                                 overlay.enabled_channels(),
                                 ch_num);
      if (std::find(skip_ch.begin(), skip_ch.end(), channel_id) != skip_ch.end())
        continue;
      
      
      // Make a Waveform for each channel and fill it
      wfmVec.emplace_back();
      auto & wfm = wfmVec.back();
      wfm.wave = convertWaveform(overlay.chDataBegin(ch_num), 
                                 overlay.chDataEnd(ch_num));
      wfm.sample_rate   = md.sample_rate_MHz;
      wfm.trigger_index = (int)(md.record_length - md.record_length*md.post_trigger/100.);

      // Make and fill a WaveformInfo object
      wfmInfoVec.push_back(fillWaveformInfo(wfm));
      
      // Make and fill a Channel object
      chVec.push_back(fillChannelInfo(overlay, md, ch_num, wfmInfoVec.back()));


    } // end loop over channels
  } // end loop over Fragment objects

}

//----------------------------------------------------------------------------
darkart::EventInfo
darkart::ConverterAlgs::fillEventInfo(artdaq::Fragment const& frag,
                                      darkart::ChannelVec const& chVec)
{
  darkart::EventInfo eventInfo;
  
  // Set the event's run and event IDs. 
  ds50::V1495Fragment overlay(frag);
  eventInfo.run_id = overlay.run_number();
  eventInfo.event_id = overlay.trigger_counter();

  eventInfo.gps_coarse = overlay.gps_coarse_s();
  eventInfo.gps_fine = overlay.gps_fine_20ns();
  eventInfo.gps_pps = overlay.gps_pps_20ns();

  eventInfo.total_inhibit_time_us = overlay.total_inhibit_time_us();
  eventInfo.incremental_inhibit_time_20ns = overlay.incremental_inhibit_time_100ns()*5;
  eventInfo.live_time_20ns = overlay.live_time_100ns()*5;

  //Set trigger info.
  eventInfo.trigger_type = overlay.trigger_type();
  eventInfo.tpc_he_events_prescale = overlay.tpc_he_events_prescale();
  eventInfo.tpc_digital_sum = overlay.tpc_digital_sum();

  eventInfo.nchans = chVec.size();
  
  eventInfo.trigger_pattern = overlay.trigger_pattern();
  eventInfo.trigger_multiplicity = overlay.trigger_multiplicity();
  unsigned long trigger_pattern = eventInfo.trigger_pattern;
  for (int i=0; i<eventInfo.nchans; ++i) {
    if (i<19) {int hit = trigger_pattern & 1; eventInfo.trigger_multiplicity_bot += hit; trigger_pattern = trigger_pattern >> 1; }
    else      {int hit = trigger_pattern & 1; eventInfo.trigger_multiplicity_top += hit; trigger_pattern = trigger_pattern >> 1; }
  }

  
  // Set the saturated flag and the status flag
  
  bool saturated = false;
  int trigger_index = chVec.front().trigger_index;
  for (int i=0; i<eventInfo.nchans; ++i) {

    // if any channel is saturated, then whole event is flagged saturated
    if (chVec[i].saturated)
      saturated = true;

    // check for ID_MISMATCH
    if (chVec[i].trigger_count != eventInfo.event_id)
      eventInfo.status |= darkart::EventInfo::ID_MISMATCH;

    // check that trigger indices are aligned
    if (chVec[i].trigger_index != trigger_index)
      eventInfo.status |= darkart::EventInfo::TRIGGER_MISMATCH;

  }
  eventInfo.saturated = saturated;

  if (eventInfo.status != darkart::EventInfo::NORMAL) {
    throw art::errors::DataCorruption;
    return eventInfo;
  }
  
  // Set all the timing information. The timestamp is stored in V1495 metadata.
  uint64_t timestamp = 0;  //seconds
  uint64_t timestamp_usec = 0;  //seconds
  ds50::V1495Fragment::metadata const* md = 
    frag.metadata<ds50::V1495Fragment::metadata>();
  timestamp = md -> trigger_time.tv_sec;
  timestamp_usec = md->trigger_time.tv_usec;
  
  if (timestamp==0) {
    eventInfo.status |= darkart::EventInfo::BAD_TIMESTAMP;
    return eventInfo;
  }
  eventInfo.timestamp_sec = timestamp;
  eventInfo.timestamp_usec = timestamp_usec;
  uint64_t full_timestamp = 1e6*timestamp + timestamp_usec;  //microseconds
  if (eventInfo.event_id==1) {
    start_time_ = full_timestamp;
    previous_event_time_ = start_time_;
  }
  eventInfo.event_time_usec = full_timestamp - start_time_;
  eventInfo.dt_usec = full_timestamp - previous_event_time_;
  previous_event_time_ = full_timestamp;
  return eventInfo;
}
