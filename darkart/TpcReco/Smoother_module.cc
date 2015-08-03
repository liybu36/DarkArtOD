////////////////////////////////////////////////////////////////////////
// Class:       Smoother
// Module Type: producer
// File:        Smoother_module.cc
//
// Generated at Mon Sep 16 15:24:54 2013 by Arthur Kurlej using artmod
// from cetpkgsupport v1_02_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Common/Assns.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/Products/ProductAssns.hh"
#include "darkart/Products/Channel.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/smoother_algs.hh"

#include <memory>

namespace darkart {
  class Smoother;
}

class darkart::Smoother : public art::EDProducer {
public:
  explicit Smoother(fhicl::ParameterSet const & p);
  virtual ~Smoother();

  void produce(art::Event & e) override;

private:
  art::InputTag ch_producer_tag_;
  art::InputTag wfm_producer_tag_;
  int pre_samp_;
  int post_samp_;
  // Declare member data here.

};


darkart::Smoother::Smoother(fhicl::ParameterSet const & p) :
  ch_producer_tag_(p.get<std::string>("ch_producer_tag")),
  wfm_producer_tag_(p.get<std::string>("wfm_producer_tag")),
  pre_samp_(p.get<int>("pre_samp")),
  post_samp_(p.get<int>("post_samp"))
{
  produces<darkart::WaveformVec>();
  produces<darkart::WaveformAssns>();
  produces<darkart::WaveformInfoVec>();
  produces<darkart::WaveformInfoAssns>();
  // Call appropriate Produces<>() functions here.
}

darkart::Smoother::~Smoother()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::Smoother::produce(art::Event & e)
{
  //get channel info
  art::Handle<darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_producer_tag_, chVecHandle);
  darkart::ChannelVec const& chVec(*chVecHandle);

  //make empty products
  std::unique_ptr<darkart::WaveformVec> smoothedWfms(new darkart::WaveformVec);
  std::unique_ptr<darkart::WaveformAssns> smoothedWfmAssns(new darkart::WaveformAssns);
  std::unique_ptr<darkart::WaveformInfoVec> smoothedWfmInfos(new darkart::WaveformInfoVec);
  std::unique_ptr<darkart::WaveformInfoAssns> smoothedWfmInfoAssns(new darkart::WaveformInfoAssns);

  //begin smoothing
  darkart::ByChannel<darkart::Waveform> get_wfm(chVecHandle, e, wfm_producer_tag_);
  for(auto const& ch:chVec){
    darkart::Channel::ChannelID const chID = ch.channelID;
    //get some kind of (integrated, baseline-subtracted, etc) waveform
    auto const& wfm = get_wfm(chID); //Ptr.
    
    //initialize new waveform+data
    darkart::Waveform smoothedWfm;
    darkart::WaveformInfo smoothedData;

    if (!wfm->isValid()) {
      // save default constructed object
      LOG_DEBUG("DataState") << "Invalid state for input waveform for channel "
                                  << ch.channel_id();
    }
    else {
      //smooth like butter
      darkart::smoother(*wfm, smoothedWfm, smoothedData, pre_samp_, post_samp_);

      if (!smoothedWfm.isValid() || !smoothedData.isValid())
        LOG_WARNING("Algorithm")<<"Inserting object in non-valid state for channel "<<ch.channel_id();
    }
    
    //fill vectors with smooth info
    smoothedWfms->push_back(smoothedWfm);
    smoothedWfmInfos->push_back(smoothedData);
    //create associatiosn
    util::createAssn(*this, e,
		     chVecHandle, ch,
		     *smoothedWfms, smoothedWfm,
		     *smoothedWfmAssns);
    util::createAssn(*this, e,
		     chVecHandle, ch,
		     *smoothedWfmInfos, smoothedData,
		     *smoothedWfmInfoAssns);
  }//finish channel loop
  //place products into event
  e.put(std::move(smoothedWfms));
  e.put(std::move(smoothedWfmAssns));
  e.put(std::move(smoothedWfmInfoAssns));
  e.put(std::move(smoothedWfmInfos));

}
							  
DEFINE_ART_MODULE(darkart::Smoother)
