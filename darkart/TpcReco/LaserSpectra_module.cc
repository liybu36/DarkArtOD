////////////////////////////////////////////////////////////////////////
// Class:       LaserSpectra
// Module Type: producer
// File:        LaserSpectra_module.cc
//
// Generated at Thu Aug  8 10:58:57 2013 by Jason Brodsky using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include "darkart/Products/EventInfo.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/Pmt.hh"
#include "darkart/Products/Baseline.hh"
#include "darkart/Products/ProductAssns.hh"
#include "darkart/Products/LaserSpectrum.hh"
#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/ManyByChannel.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/common_algs.hh"

#include "artdaq-core/Data/Fragments.hh"

#include "darksidecore/Data/V1495Fragment.hh"

#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>

#include "TFile.h"
#include "TH1F.h"

namespace darkart {
  class LaserSpectra;
}

class darkart::LaserSpectra : public art::EDProducer {
public:
  typedef cet::map_vector_key key;

  explicit LaserSpectra(fhicl::ParameterSet const & p);
  virtual ~LaserSpectra();

  void produce(art::Event & e) override;

  void beginJob() override;
  void beginRun(art::Run & r) override;
  void endRun(art::Run & r) override;

private:
  art::InputTag ch_producer_tag_;
  art::InputTag bs_wfm_producer_tag_;
  art::InputTag roi_producer_tag_;
  art::InputTag v1495_tag_;

  art::ServiceHandle<art::TFileService> tfs_;
  bool save_histograms;
  std::unique_ptr<darkart::LaserSpectrumMap> spectraMap;
  
  int laser_region;
  int laser_accidental_region;
  
  int pedestal_region;
  int pedestal_accidental_region;
 
  double accidental_threshold;
  
  int nbins, low_bin, high_bin;
  
  //temp
  std::vector<int>  channels;  

};


darkart::LaserSpectra::LaserSpectra(fhicl::ParameterSet const & p)
 :ch_producer_tag_(p.get<std::string>("ch_producer_tag")),
 bs_wfm_producer_tag_(p.get<std::string>("bs_wfm_producer_tag")),
 roi_producer_tag_(p.get<std::string>("roi_producer_tag")),
 v1495_tag_(p.get<std::string>("v1495_tag")),
  tfs_(), save_histograms(p.get<bool>("save_histograms")), 
 spectraMap(new LaserSpectrumMap()),
 laser_region(p.get<double>("laser_region")),
 laser_accidental_region(p.get<double>("laser_accidental_region")),
 pedestal_region(p.get<double>("pedestal_region")),
 pedestal_accidental_region(p.get<double>("pedestal_accidental_region")),
 accidental_threshold(p.get<double>("accidental_threshold")),
 nbins(p.get<int>("nbins")),
 low_bin(p.get<int>("low_bin")),
 high_bin(p.get<int>("high_bin")),
 //temp
 channels(p.get<std::vector<int> >("channels_tmp"))
{
  produces<darkart::LaserSpectrumMap, art::InRun>();
}


darkart::LaserSpectra::~LaserSpectra()
{
}

void darkart::LaserSpectra::produce(art::Event & e)
{
  art::Handle<darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_producer_tag_, chVecHandle);
  darkart::ChannelVec const& chVec(*chVecHandle);

  art::Handle<artdaq::Fragments> v1495_handle;
  e.getByLabel(v1495_tag_, v1495_handle);

  if(!v1495_handle.isValid () || ds50::V1495Fragment((*v1495_handle)[0]).trigger_type () != ds50::V1495Fragment::laser) 
    {
      throw cet::exception("ProductRetrieval")<<"v1495 retrieval problem in LaserSpectra_module";
    }

  for (auto ch : chVec) 
  {
    int chID = ch.channel_id();
    //darkart::Waveform const& wfm = util::getByChannelID<darkart::Waveform>(e, chVecHandle,
    //                                                                       chID,
    //                                                                       bs_wfm_producer_tag_);
    //darkart::Baseline const& bl = util::getByChannelID<darkart::Baseline>(e, chVecHandle,
    //                    chID,
    //                    bs_wfm_producer_tag_);

    darkart::ByChannel<darkart::Baseline> get_bl(chVecHandle, e, bs_wfm_producer_tag_);
    darkart::ManyByChannel<darkart::Roi> get_roi_vec(chVecHandle, e, roi_producer_tag_);
    auto const& bl  = *get_bl(chID);
    auto roivec = get_roi_vec(chID);
    // sort roi vector because getter doesn't always preserve order
    std::sort(roivec.begin(), roivec.end());
    
    if(bl.found_baseline)
    {
  
  int n_regions = roivec.size();
  if (n_regions <= laser_region ||
      n_regions <= laser_accidental_region ||
      n_regions <= pedestal_region ||
      n_regions <= pedestal_accidental_region)
  {
      std::cout<<"Number of defined regions are too few for laser spectra calculations"<<std::endl;
      return;
  }

  double laser_integral = roivec[laser_region]->integral;
  double laser_max_amp = -roivec[laser_region]->min;
  double laser_acc_integral = roivec[laser_accidental_region]->integral;
  
  double pedestal_integral = roivec[pedestal_region]->integral;
  double pedestal_max_amp = -roivec[pedestal_region]->min;
  double pedestal_acc_integral = roivec[pedestal_accidental_region]->integral;
  
  LaserSpectrum* ls = spectraMap->getOrThrow(key(chID));
      if (std::abs(laser_integral-laser_acc_integral) < accidental_threshold)
  {
      ls->laser_int_hist.Fill(-laser_integral);
      ls->laser_amp_hist.Fill(laser_max_amp);

      //if(-laser_integral>160) std::cout<<chID<<"/"<<-laser_integral;

      if(ds50::V1495Fragment((*v1495_handle)[0]).channel_in_trigger(chID)) 
                {
                  //std::cout<<chID<<"/"<<-laser_integral<<" ";
                  //std::cout<<chID<<" ";
                  ls->threshold_hist.Fill(-laser_integral);
  
                }
               // else{std::cout<<" ";}
  }
  if (std::abs(pedestal_integral-pedestal_acc_integral) < accidental_threshold)
  {
      ls->pedestal_int_hist.Fill(-pedestal_integral);
      ls->pedestal_amp_hist.Fill(pedestal_max_amp);
  }
    }
  }
  //std::cout<<ds50::V1495Fragment((*v1495_handle)[0]).trigger_pattern()<<std::endl;
}

void darkart::LaserSpectra::beginJob()
{
}

void darkart::LaserSpectra::endRun(art::Run & r)
{
    if (save_histograms)
    {
  tfs_->file().cd();
  for(LaserSpectrumMap::iterator it = spectraMap->begin(); it!=spectraMap->end();++it)
  {
      TString name = "channel_";
      name += it->first.asInt(); name += "_";
      
      TString histname;
      histname = name + it->second->laser_int_hist.GetName();
      it->second->laser_int_hist.Write(histname);
      
      histname = name + it->second->laser_amp_hist.GetName();
      it->second->laser_amp_hist.Write(histname);
      
      histname = name + it->second->pedestal_int_hist.GetName();
      it->second->pedestal_int_hist.Write(histname);
      
      histname = name + it->second->pedestal_amp_hist.GetName();
      it->second->pedestal_amp_hist.Write(histname);
      
      histname = name + "thresh";
      it->second->threshold_hist.Write(histname);

      std::cout<<"Wrote histograms for channel: "<<it->first.asInt()<<std::endl;
  }
    }
    r.put(std::move(spectraMap));

}

void darkart::LaserSpectra::beginRun(art::Run & )//r)
{
    /*
    art::Handle<darkart::PmtVec> pmtVecHandle;
    r.getByLabel(ch_producer_tag_, pmtVecHandle);
    darkart::PmtVec const& pmtVec(*pmtVecHandle);

    

    for (auto pmt : pmtVec) 
    {
      TString name = "channel";
    name += pmt.channel_id;

      spectraMap->push_back(std::make_pair(key(pmt.channel_id),LaserSpectrum(name,name,nbins,low_bin,high_bin)));
      std::cout<<"Creating spectrum for channel "<<pmt.channel_id<<std::endl;
    }
    */

    for (int chn : channels) 
    {
      (*spectraMap)[key(chn)]= new LaserSpectrum(nbins,low_bin,high_bin);
    }
    
    for(LaserSpectrumMap::iterator it = spectraMap->begin(); it!=spectraMap->end();++it)
    {
  //std::cout<<"map first: "<<it->first<<"   map second: "<<it->second.hist<<std::endl;
      std::cout<<"Created spectrum for channel: "<<it->first.asInt()<<std::endl;
    }
}



DEFINE_ART_MODULE(darkart::LaserSpectra)
