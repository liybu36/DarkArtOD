////////////////////////////////////////////////////////////////////////
// Class:       RoiFinder
// Module Type: producer
// File:        RoiFinder_module.cc
//
// Generated at Fri Aug  2 02:49:26 2013 by Arthur Kurlej using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "darkart/Products/Roi.hh"
#include "darkart/Products/Waveform.hh"
#include "darkart/Products/ProductAssns.hh"
#include "darkart/Products/Channel.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/roifinder_algs.hh"

#include <memory>
#include <iostream>

namespace darkart {
  class RoiFinder;
}

class darkart::RoiFinder : public art::EDProducer {
public:
  explicit RoiFinder(fhicl::ParameterSet const & p);
  virtual ~RoiFinder();

  void produce(art::Event & e) override;

private:
  art::InputTag wfm_producer_tag_;
  art::InputTag ch_producer_tag_;
  art::InputTag int_wfm_producer_tag_;
  RoiFinderParams params;
  

};

//get params
darkart::RoiFinder::RoiFinder(fhicl::ParameterSet const & p):
  wfm_producer_tag_(p.get<std::string>("wfm_producer_tag")),
  ch_producer_tag_(p.get<std::string>("ch_producer_tag")),
  int_wfm_producer_tag_(p.get<std::string>("int_wfm_producer_tag")),
  params(p.get<std::vector< std::vector<double> > >("roi_times"))
{
  produces< darkart::RoiVec >();
  produces< darkart::RoiAssns >();
}

darkart::RoiFinder::~RoiFinder()
{
  //Clean up dynamic memory and other resources here
}

void darkart::RoiFinder::produce(art::Event & e)
{
  //get channel info
  art::Handle<darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_producer_tag_, chVecHandle);
  darkart::ChannelVec const& chVec(*chVecHandle);

  //make products
  std::unique_ptr<darkart::RoiVec> roiVec(new darkart::RoiVec);
  std::unique_ptr<darkart::RoiAssns> roiAssns(new darkart::RoiAssns);
  
  //find roi here
  darkart::ByChannel<darkart::Waveform> get_bs_wfm(chVecHandle, e, wfm_producer_tag_);
  darkart::ByChannel<darkart::Waveform> get_int_wfm(chVecHandle, e, int_wfm_producer_tag_);
  for(auto ch:chVec){
    auto const& chID = ch.channelID;
    auto const& bsWfm = get_bs_wfm(chID);
    auto const& intWfm = get_int_wfm(chID);
    
    //create variables and find ROI here
    darkart::RoiVec tmpRoiVec;
    
    if (!bsWfm->isValid()) {
      // save default constructed object
      LOG_DEBUG("DataState") << "Invalid state for input waveform for channel "
                                << ch.channel_id();
    }
    else {
      darkart::findRoi(*bsWfm, *intWfm, tmpRoiVec, params);
    }
    
    // create associations individually for each roi
    for (auto const& roi : tmpRoiVec) {
      roiVec->push_back(roi);
      //create associations
      util::createAssn(*this, e,
                       chVecHandle,ch,
                       *roiVec, roi,
                       *roiAssns);
    }
  } // end loop over channels
  
  
  
  //completed products go into event
  e.put(std::move(roiAssns));
  e.put(std::move(roiVec));
  
}


DEFINE_ART_MODULE(darkart::RoiFinder)
