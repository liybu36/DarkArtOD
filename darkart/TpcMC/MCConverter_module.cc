////////////////////////////////////////////////////////////////////////
// Class:       MCConverter
// Module Type: producer
// File:        MCConverter_module.cc
//
// Generated at Wed Oct 29 21:29:30 2014 by Alden Fan using artmod
// from cetpkgsupport v1_07_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <algorithm>

#include "darkart/Products/Channel.hh"
#include "darkart/Products/EventInfo.hh"
#include "darkart/Products/Pmt.hh"
#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/Products/ProductAssns.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/utilities.hh"

namespace darkart {
  class MCConverter;
}


////////////////////////////////////////////////////////////////////////
// 2014-10-29 AFan
//
// Modularization of existing electronics MC.
//
// This module creates new EventInfo and ChannelVec appropriate to MC.
// It also takes the overlaid darkart::Waveforms and darkart::WaveformIno
// and creates Asssns to them. To downstream modules, this module looks
// just like a real Converter module.
////////////////////////////////////////////////////////////////////////



class darkart::MCConverter : public art::EDProducer {
public:
  explicit MCConverter(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  MCConverter(MCConverter const &) = delete;
  MCConverter(MCConverter &&) = delete;
  MCConverter & operator = (MCConverter const &) = delete;
  MCConverter & operator = (MCConverter &&) = delete;

  // Required functions.
  void beginRun(art::Run & r) override;
  void produce(art::Event & e) override;


private:

  // Declare member data here.
  art::InputTag _event_info_tag;
  art::InputTag _ch_tag;
  art::InputTag _pmt_tag;
  art::InputTag _spe_mean_tag;
  art::InputTag _spe_sigma_tag;
  art::InputTag _mc_wfm_tag;

};


darkart::MCConverter::MCConverter(fhicl::ParameterSet const & p)
  : _event_info_tag(p.get<std::string>("event_info_tag"))
  , _ch_tag(p.get<std::string>("ch_tag"))
  , _pmt_tag(p.get<std::string>("pmt_tag"))
  , _spe_mean_tag(p.get<std::string>("spe_mean_tag"))
  , _spe_sigma_tag(p.get<std::string>("spe_sigma_tag"))
  , _mc_wfm_tag(p.get<std::string>("mc_wfm_tag"))
{
  produces<darkart::EventInfo>();
  produces<darkart::ChannelVec>();
  produces<darkart::PmtVec, art::InRun>();
  produces<darkart::WaveformVec>();
  produces<darkart::WaveformInfoVec>();
  produces<darkart::WaveformAssns>();
  produces<darkart::WaveformInfoAssns>();
}

void darkart::MCConverter::beginRun(art::Run & r)
{
  art::Handle<darkart::PmtVec> realPmtVecHandle; r.getByLabel(_pmt_tag, realPmtVecHandle);
  art::Handle<double> speMeanHandle; r.getByLabel(_spe_mean_tag, speMeanHandle);
  art::Handle<double> speSigmaHandle; r.getByLabel(_spe_sigma_tag, speSigmaHandle);
  std::unique_ptr<darkart::PmtVec> mcPmtVec(new darkart::PmtVec(*realPmtVecHandle));

  // reset spe means and sigmas
  for (auto & pmt : *mcPmtVec) {
    pmt.spe_mean = *speMeanHandle;
    pmt.spe_sigma = *speSigmaHandle;
  }
  r.put(std::move(mcPmtVec));
}


void darkart::MCConverter::produce(art::Event & e)
{
  // Retrieve the real EventInfo and ChannelVec
  auto const& realEvtInfoHandle = e.getValidHandle<darkart::EventInfo>(_event_info_tag);
  art::Handle<darkart::ChannelVec> realChVecHandle; e.getByLabel(_ch_tag, realChVecHandle);
  auto const& mcWfmVecHandle = e.getValidHandle<darkart::WaveformVec>(_mc_wfm_tag);
  auto const& mcWfmInfoVecHandle = e.getValidHandle<darkart::WaveformInfoVec>(_mc_wfm_tag);
  
  // Getter for MC Waveform and WaveformInfo
  darkart::ByChannel<darkart::Waveform> get_mc_wfm(realChVecHandle, e,  _mc_wfm_tag);
  darkart::ByChannel<darkart::WaveformInfo> get_mc_wfm_info(realChVecHandle, e, _mc_wfm_tag);
  
  // Instantiate new products to be same as old.
  std::unique_ptr<darkart::EventInfo         > mcEvtInfo      (new darkart::EventInfo(*realEvtInfoHandle));
  std::unique_ptr<darkart::ChannelVec        > mcChVec        (new darkart::ChannelVec(*realChVecHandle));
  std::unique_ptr<darkart::WaveformVec       > mcWfmVec       (new darkart::WaveformVec(*mcWfmVecHandle));
  std::unique_ptr<darkart::WaveformInfoVec   > mcWfmInfoVec   (new darkart::WaveformInfoVec(*mcWfmInfoVecHandle));
  std::unique_ptr<darkart::WaveformAssns     > mcWfmAssns     (new darkart::WaveformAssns);
  std::unique_ptr<darkart::WaveformInfoAssns > mcWfmInfoAssns (new darkart::WaveformInfoAssns);
  
  // Overwrite some values with new ones appropriate for MC.
  mcEvtInfo->saturated = false;
  for (auto & ch : *mcChVec) {
    auto const& mc_wfm = get_mc_wfm(ch.channelID); //Ptr
    auto const& wfm_info = get_mc_wfm_info(ch.channelID);
    
    ch.sample_rate = mc_wfm->sample_rate;
    ch.trigger_index = mc_wfm->trigger_index;
    ch.nsamps = mc_wfm->wave.size();
    ch.saturated = (wfm_info->minimum == 0 || wfm_info->maximum == ch.GetVerticalRange());
    if (ch.saturated)
      mcEvtInfo->saturated = true;
  }

  
  // Create assns between new products
  for (auto const& ch : *mcChVec) {
    auto const& wfm = get_mc_wfm(ch.channelID); //Ptr
    auto const& wfm_info = get_mc_wfm_info(ch.channelID); //Ptr

    util::createAssn( *this, e,
                      *mcChVec, ch,
                      *mcWfmVec, *wfm,
                      *mcWfmAssns );
    
    util::createAssn( *this, e,
                      *mcChVec, ch,
                      *mcWfmInfoVec, *wfm_info,
                      *mcWfmInfoAssns );
  }

  // Insert into event
  e.put(std::move(mcEvtInfo));
  e.put(std::move(mcChVec));
  e.put(std::move(mcWfmVec));
  e.put(std::move(mcWfmInfoVec));
  e.put(std::move(mcWfmAssns));
  e.put(std::move(mcWfmInfoAssns));
}

DEFINE_ART_MODULE(darkart::MCConverter)