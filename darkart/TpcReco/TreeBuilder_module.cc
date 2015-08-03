////////////////////////////////////////////////////////////////////////
// Class:       TreeBuilder
// Module Type: analyzer
// File:        TreeBuilder_module.cc
//
// Generated at Fri Sep 13 13:28:54 2013 by Alden Fan using artmod
// from cetpkgsupport v1_02_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Core/FindOne.h"
#include "art/Framework/Core/FindMany.h"

#include "darkart/Products/EventData.hh"
#include "darkart/Products/EventInfo.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/Pmt.hh"
#include "darkart/Products/Baseline.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/Products/Pulse.hh"
#include "darkart/Products/PulseParam.hh"
#include "darkart/Products/Roi.hh"
#include "darkart/Products/Spe.hh"
#include "darkart/Products/TDCCluster.hh"
#include "darkart/Products/ProductAssns.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/ManyByChannel.hh"
#include "darkart/TpcReco/ByPulse.hh"
#include "darkart/TpcReco/PmtHelper.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/common_algs.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/maybe_ref.h"
#include "cetlib/exception.h"


#include "TTree.h"

#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include <cstdlib>

namespace darkart {
  class TreeBuilder;
}


//--------------------------------------------------------------------
// This module creates the darkart "super-product" which holds all the
// non-darkart::Waveform data products in one big class. Organizes
// objects with processed information into event-level and channel-
// level structure. Populates a TTree to be saved to output.
//--------------------------------------------------------------------



class darkart::TreeBuilder : public art::EDAnalyzer {
public:
  explicit TreeBuilder(fhicl::ParameterSet const & p);
  virtual ~TreeBuilder();

  void beginJob() override;
  void analyze(art::Event const & e) override;


private:

  art::InputTag event_info_tag_;
  art::InputTag ch_tag_;
  art::InputTag pmt_tag_;
  art::InputTag raw_wfm_tag_;
  art::InputTag baseline_tag_;
  art::InputTag bs_wfm_tag_;
  art::InputTag int_wfm_tag_;
  art::InputTag roi_tag_;
  art::InputTag sumch_tag_;
  art::InputTag sumch_raw_wfm_tag_;
  art::InputTag sumch_baseline_tag_;
  art::InputTag sumch_bs_wfm_tag_;
  art::InputTag sumch_int_wfm_tag_;
  art::InputTag pulse_tag_;
  art::InputTag pulseparam_tag_;
  art::InputTag sumch_pulse_tag_;
  art::InputTag sumch_pulseparam_tag_;
  art::InputTag event_pulse_tag_;
  art::InputTag event_pulseparam_tag_;
  art::InputTag pos_param_tag_;
  art::InputTag spe_tag_;
  art::InputTag tdccluster_tag_;


  art::ServiceHandle<art::TFileService> tfs_;
  TTree* tree_;

  EventData* eventData;

};


darkart::TreeBuilder::TreeBuilder(fhicl::ParameterSet const & p) :
  EDAnalyzer(p),
  event_info_tag_(p.get<std::string>("event_info_tag")),
  ch_tag_(p.get<std::string>("ch_tag")),
  pmt_tag_(p.get<std::string>("pmt_tag")),
  raw_wfm_tag_(p.get<std::string>("raw_wfm_tag")),
  baseline_tag_(p.get<std::string>("baseline_tag")),
  bs_wfm_tag_(p.get<std::string>("bs_wfm_tag")),
  int_wfm_tag_(p.get<std::string>("int_wfm_tag")),
  roi_tag_(p.get<std::string>("roi_tag")),
  sumch_tag_(p.get<std::string>("sumch_tag")),
  sumch_raw_wfm_tag_(p.get<std::string>("sumch_raw_wfm_tag")),
  sumch_baseline_tag_(p.get<std::string>("sumch_baseline_tag")),
  sumch_bs_wfm_tag_(p.get<std::string>("sumch_bs_wfm_tag")),
  sumch_int_wfm_tag_(p.get<std::string>("sumch_int_wfm_tag")),
  pulse_tag_(p.get<std::string>("pulse_tag")),
  pulseparam_tag_(p.get<std::string>("pulseparam_tag")),
  sumch_pulse_tag_(p.get<std::string>("sumch_pulse_tag")),
  sumch_pulseparam_tag_(p.get<std::string>("sumch_pulseparam_tag")),
  event_pulse_tag_(p.get<std::string>("event_pulse_tag")),
  event_pulseparam_tag_(p.get<std::string>("event_pulseparam_tag")),
  pos_param_tag_(p.get<std::string>("pos_param_tag")),
  spe_tag_(p.get<std::string>("spe_tag")),
  tdccluster_tag_(p.get<std::string>("tdccluster_tag")),
  tfs_(),
  tree_(nullptr),
  eventData(nullptr)
{
}

darkart::TreeBuilder::~TreeBuilder()
{
}

void darkart::TreeBuilder::beginJob()
{
  tree_ = tfs_->make<TTree>("Events", "Events");
  tree_->Branch("EventData", &eventData);
  
}


void darkart::TreeBuilder::analyze(art::Event const& e)
{

  eventData->clear();

  eventData->darkart_version = std::getenv("CETPKG_VERSION");
  
  // Get the event-level identifier info
  auto const& evtInfo = e.getValidHandle<EventInfo>(event_info_tag_);
  eventData->event_info = *evtInfo;
  eventData->channels.reserve(evtInfo->nchans);

  // Get the input channels
  art::Handle<darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_tag_, chVecHandle);
  auto const& chVec(*chVecHandle);

  // Get handle to the Pulse objects. necessary for object retrieval
  art::Handle<darkart::PulseVec> pulseVecHandle;
  e.getByLabel(pulse_tag_, pulseVecHandle);


  // Helpers for retrieving products by association
  darkart::PmtHelper get_pmt(e, pmt_tag_);
  darkart::ByChannel<darkart::WaveformInfo> get_raw_wfm_info (chVecHandle, e, raw_wfm_tag_);
  darkart::ByChannel<darkart::WaveformInfo> get_bs_wfm_info  (chVecHandle, e, bs_wfm_tag_);
  darkart::ByChannel<darkart::Baseline>     get_baseline     (chVecHandle, e, baseline_tag_);
  darkart::ManyByChannel<darkart::Roi>      get_roi_vec      (chVecHandle, e, roi_tag_);
  darkart::ByChannel<darkart::WaveformInfo> get_integral     (chVecHandle, e, int_wfm_tag_);
  darkart::ManyByChannel<darkart::Pulse>    get_pulse_vec    (chVecHandle, e, pulse_tag_);
  darkart::ByPulse<darkart::PulseParam>     get_pulse_param  (pulseVecHandle, e, pulseparam_tag_);
  darkart::ManyByChannel<darkart::Spe>      get_spe_vec      (chVecHandle, e, spe_tag_);




  
  //-------------------------------------------------------------------------------
  // First take care of the physical channels
  //-------------------------------------------------------------------------------
  
  for (darkart::Channel const& channel : chVec) {
    darkart::ChannelData chdata;


    darkart::Channel::ChannelID const& chID = channel.channelID;
    
    chdata.channel  = channel;
    chdata.pmt      = get_pmt(chID);
    chdata.raw_wfm  = *get_raw_wfm_info(chID);
    chdata.baseline_subtracted_wfm = *get_bs_wfm_info(chID);
    chdata.baseline = *get_baseline(chID);
    chdata.integral = *get_integral(chID);

    //ManyByChannel::operator() returns a vector<Ptr<T>>
    auto roiVec = get_roi_vec(chID);
    // sort roiVec because getter doesn't always preserve order
    std::sort(roiVec.begin(), roiVec.end());
    for (auto const& region : roiVec)
      chdata.regions.push_back(*region);
    

    // Get the pulses for this channel and make and fill a PulseData object for each one
    auto pulseVec = get_pulse_vec(chID);
    // sort pulseVec because getter doesn't always preserve order
    std::sort(pulseVec.begin(), pulseVec.end());
    for (auto const& pulse : pulseVec) {
      darkart::PulseData pulsedata;

      // pulse is a Ptr.
      pulsedata.pulse = *pulse;
      pulsedata.param = *get_pulse_param(pulse->pulseID);
      
      chdata.pulses.push_back(pulsedata);
    }
    //chdata.n_pulses = chdata.pulses.size();
    
    // Get the spe vector
    auto speVec = get_spe_vec(chID);
    // sort speVec because getter doesn't always preserve order
    std::sort(speVec.begin(), speVec.end());
    for (auto const& spe : speVec) {
      // spe is Ptr
      chdata.single_pe.push_back(*spe);
    }
    
    eventData->channels.push_back(chdata);
  }
  eventData->n_channels = eventData->channels.size();

  //-------------------------------------------------------------------------------
  // Now take care of the sum channel
  //-------------------------------------------------------------------------------

  art::Handle<darkart::ChannelVec> sumchVecHandle;
  e.getByLabel(sumch_tag_, sumchVecHandle);
  auto const& sumch(sumchVecHandle->front());
  
  darkart::ChannelData sumchannel;
    
  darkart::Channel::ChannelID const& sumchID = sumch.channelID;

  darkart::PmtHelper get_sum_pmt(e, sumch_tag_);
  darkart::ByChannel<darkart::WaveformInfo> get_sum_raw_wfm_info (sumchVecHandle, e, sumch_raw_wfm_tag_);
  darkart::ByChannel<darkart::WaveformInfo> get_sum_bs_wfm_info  (sumchVecHandle, e, sumch_bs_wfm_tag_);
  darkart::ByChannel<darkart::Baseline>     get_sum_baseline     (sumchVecHandle, e, sumch_baseline_tag_);
  darkart::ByChannel<darkart::WaveformInfo> get_sum_integral     (sumchVecHandle, e, sumch_int_wfm_tag_);
  darkart::ManyByChannel<darkart::Pulse>    get_sum_pulse_vec    (sumchVecHandle, e, sumch_pulse_tag_);
  darkart::ByPulse<darkart::PulseParam>     get_sum_pulse_param  (pulseVecHandle, e, sumch_pulseparam_tag_);
  
  sumchannel.channel  = sumch;
  sumchannel.pmt      = get_sum_pmt(sumchID);
  sumchannel.raw_wfm  = *get_sum_raw_wfm_info(sumchID);
  sumchannel.baseline_subtracted_wfm = *get_sum_bs_wfm_info(sumchID);
  sumchannel.baseline = *get_sum_baseline(sumchID);
  sumchannel.integral = *get_sum_integral(sumchID);


  //going to fill sumchannel.regions?

  

  // get the pulses for the sum channel
  auto sumchPulseVec = get_sum_pulse_vec(sumchID);
  // sort for consistency
  std::sort(sumchPulseVec.begin(), sumchPulseVec.end());
  for (auto const& pulse : sumchPulseVec) {
    darkart::PulseData pulsedata;

    // pulse is a Ptr.
    pulsedata.pulse = *pulse;
    pulsedata.param = *get_sum_pulse_param(pulse->pulseID);
    
    sumchannel.pulses.push_back(pulsedata);
  }
  //sumchannel.n_pulses = sumchannel.pulses.size();
  
  eventData->sumchannel = sumchannel;

  //-------------------------------------------------------------------------------
  // Now create PulseData objects at the event level
  //-------------------------------------------------------------------------------
  
  // Retrieve the event-level pulse objects
  art::Handle<darkart::PulseVec> eventPulseVecHandle;
  e.getByLabel(event_pulse_tag_, eventPulseVecHandle);
  auto const& evtPlsVec(*eventPulseVecHandle);
  darkart::ByPulse<darkart::PulseParam> get_evt_pulse_param(eventPulseVecHandle, e, event_pulseparam_tag_);
  darkart::ByPulse<darkart::PositionParam> get_evt_pos_param(eventPulseVecHandle, e, pos_param_tag_);

  for (auto const& pulse : evtPlsVec) {
    darkart::PulseData pulsedata;

    pulsedata.pulse = pulse;
    pulsedata.param = *get_evt_pulse_param(pulse.pulseID);
    pulsedata.position = *get_evt_pos_param(pulse.pulseID);
    
    eventData->pulses.push_back(pulsedata);
  }
  eventData->n_pulses = eventData->pulses.size();

  //Get TDC Clusters
  auto const& tdc_clusters = e.getValidHandle<TDCClusters>(tdccluster_tag_);
  eventData->tdc_clusters = *tdc_clusters;
  
  tree_->Fill();
}

DEFINE_ART_MODULE(darkart::TreeBuilder)
