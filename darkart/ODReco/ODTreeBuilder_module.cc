////////////////////////////////////////////////////////////////////////
// Class:       ODTreeBuilder
// Module Type: producer
// File:        ODTreeBuilder_module.cc
//
// Generated at Fri Feb  6 04:24:33 2015 by Alden Fan using artmod
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
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"


#include <memory>
#include <cstdlib>

#include "darkart/ODProducts/ODEventData.hh"
#include "darkart/ODProducts/RunInfo.hh"
#include "darkart/ODProducts/ODEventInfo.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/Cluster.hh"
#include "darkart/ODProducts/ROI.hh"
#include "darkart/ODProducts/Slider.hh"
#include "darkart/ODProducts/ROICfg.hh"
#include "darkart/ODProducts/SliderCfg.hh"
#include "darkart/ODProducts/TotalCharge.hh"

#include "TTree.h"

namespace darkart {
  namespace od {
    class ODTreeBuilder;
  }
}

////////////////////////////////////////////////////////////////////////
// This module is the OD analog to the TPC's TreeBuilder module. It
// fills a super-product to be written to ROOT file separately from
// ART's native ROOT output. Run-level information (i.e. RunInfo) is
// filled once per file in a separate branch.
// 
// We access data products using plain art::Handle (rather than
// art::ValidHandle) to better deal with modules being turned off.
////////////////////////////////////////////////////////////////////////



class darkart::od::ODTreeBuilder : public art::EDProducer {
public:
  explicit ODTreeBuilder(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  ODTreeBuilder(ODTreeBuilder const &) = delete;
  ODTreeBuilder(ODTreeBuilder &&) = delete;
  ODTreeBuilder & operator = (ODTreeBuilder const &) = delete;
  ODTreeBuilder & operator = (ODTreeBuilder &&) = delete;

  void beginJob() override;
  void beginRun(art::Run & r) override;
  void produce(art::Event & e) override;


private:
  std::string _run_info_tag;
  std::string _event_info_tag;
  std::string _channel_data_tag;
  std::string _cluster_lsv_tag;
  std::string _cluster_wt_tag;
  std::string _roi_lsv_tag;
  std::string _roi_wt_tag;
  std::string _slider_lsv_tag;
  std::string _slider_wt_tag;
  std::string _total_charge_tag;

  art::ServiceHandle<art::TFileService> _tfs;
  TTree* _tree;
  TTree* _runtree;

  darkart::od::ODEventData* _event_data;
  darkart::od::RunInfo* _run_info;
  art::Handle<darkart::od::RunInfo> ri;

  bool _filled_run_info;  
};


darkart::od::ODTreeBuilder::ODTreeBuilder(fhicl::ParameterSet const & p)
  : _run_info_tag     (p.get<std::string>( "run_info_tag"     , ""))
  , _event_info_tag   (p.get<std::string>( "event_info_tag"   , ""))
  , _channel_data_tag (p.get<std::string>( "channel_data_tag" , ""))
  , _cluster_lsv_tag  (p.get<std::string>( "cluster_lsv_tag"  , ""))
  , _cluster_wt_tag   (p.get<std::string>( "cluster_wt_tag"   , ""))
  , _roi_lsv_tag      (p.get<std::string>( "roi_lsv_tag"      , ""))
  , _roi_wt_tag       (p.get<std::string>( "roi_wt_tag"       , ""))
  , _slider_lsv_tag   (p.get<std::string>( "slider_lsv_tag"   , ""))
  , _slider_wt_tag    (p.get<std::string>( "slider_wt_tag"    , ""))
  , _total_charge_tag (p.get<std::string>( "total_charge_tag" , ""))
  , _tfs()
  , _tree(nullptr)
  , _runtree(nullptr)
  , _event_data(nullptr)
  , _run_info(nullptr)
  , _filled_run_info(false)
{ }

void darkart::od::ODTreeBuilder::beginJob()
{
  _tree = _tfs->make<TTree>("Events", "Reconstructed OD event data");
  _tree->Branch("ODEventData", &_event_data);
  
  _runtree = _tfs->make<TTree>("Run", "Run info");
  _runtree->Branch("RunInfo",   &_run_info);
}

void darkart::od::ODTreeBuilder::beginRun(art::Run &r)
{
  r.getByLabel(_run_info_tag, ri);
  _filled_run_info = false;
}

void darkart::od::ODTreeBuilder::produce(art::Event & e)
{
  // Clear EventData before filling with new stuff
  _event_data->clear();

  // Get darkart version from environment
  _event_data->darkart_version = std::getenv("CETPKG_VERSION");

  // Fill the RunInfo tree once per run
  if(!_filled_run_info){
    *_run_info = *ri;
    art::Handle<darkart::od::ROICfg>         roicfg_lsv;    e.getByLabel(_roi_lsv_tag,    roicfg_lsv);
    art::Handle<darkart::od::ROICfg>         roicfg_wt;     e.getByLabel(_roi_wt_tag,     roicfg_wt);
    art::Handle<darkart::od::SliderCfg>      slidercfg_lsv; e.getByLabel(_slider_lsv_tag, slidercfg_lsv);
    art::Handle<darkart::od::SliderCfg>      slidercfg_wt;  e.getByLabel(_slider_wt_tag,  slidercfg_wt);
    if(roicfg_lsv.isValid())    _run_info->roicfg_lsv    = *roicfg_lsv;
    if(roicfg_wt.isValid())     _run_info->roicfg_wt     = *roicfg_wt;
    if(slidercfg_lsv.isValid()) _run_info->slidercfg_lsv = *slidercfg_lsv;
    if(slidercfg_wt.isValid())  _run_info->slidercfg_wt  = *slidercfg_wt;
    _runtree->Fill();

    _filled_run_info = true;
  }

  // retrieve data products from art::Event
  art::Handle<darkart::od::ODEventInfo>    event_info;    e.getByLabel(_event_info_tag,   event_info);
  art::Handle<darkart::od::ChannelDataVec> channel_data;  e.getByLabel(_channel_data_tag, channel_data);
  art::Handle<darkart::od::ClusterVec>     clusters_lsv;  e.getByLabel(_cluster_lsv_tag,  clusters_lsv);
  art::Handle<darkart::od::ClusterVec>     clusters_wt;   e.getByLabel(_cluster_wt_tag,   clusters_wt);
  art::Handle<darkart::od::ROI>            roi_lsv;       e.getByLabel(_roi_lsv_tag,      roi_lsv);
  art::Handle<darkart::od::ROI>            roi_wt;        e.getByLabel(_roi_wt_tag,       roi_wt);
  art::Handle<darkart::od::Slider>         slider_lsv;    e.getByLabel(_slider_lsv_tag,   slider_lsv);
  art::Handle<darkart::od::Slider>         slider_wt;     e.getByLabel(_slider_wt_tag,    slider_wt);
  art::Handle<darkart::od::TotalCharge>    total_charge;  e.getByLabel(_total_charge_tag, total_charge);

  // Fill EventData with retrieved data products
  if (event_info.isValid())   _event_data->event_info   = *event_info;
  if (channel_data.isValid()) _event_data->channel_data = *channel_data;
  if (clusters_lsv.isValid()) {
    _event_data->nclusters_lsv    = clusters_lsv->size();
    _event_data->clusters_lsv     = *clusters_lsv;
  }
  if (clusters_wt.isValid()) {
    _event_data->nclusters_wt    = clusters_wt->size();
    _event_data->clusters_wt     = *clusters_wt;
  }
  if(roi_lsv.isValid())          _event_data->roi_lsv      = *roi_lsv;
  if(roi_wt.isValid())           _event_data->roi_wt       = *roi_wt;
  if(slider_lsv.isValid())       _event_data->slider_lsv   = *slider_lsv;
  if(slider_wt.isValid())        _event_data->slider_wt    = *slider_wt;
  if(total_charge.isValid())     _event_data->total_charge = *total_charge;
  
  _tree->Fill();
}

DEFINE_ART_MODULE(darkart::od::ODTreeBuilder)
