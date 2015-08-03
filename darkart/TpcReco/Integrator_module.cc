////////////////////////////////////////////////////////////////////////
// Class:       Integrator
// Module Type: producer
// File:        Integrator_module.cc
//
// Generated at Mon Sep  2 10:44:10 2013 by Arthur Kurlej using artmod
// from cetpkgsupport v1_02_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "art/Persistency/Common/Assns.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/Products/ProductAssns.hh"
#include "darkart/Products/Channel.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/integrator_algs.hh"


#include <memory>

namespace darkart {
  class Integrator;
}

class darkart::Integrator : public art::EDProducer {
public:
  explicit Integrator(fhicl::ParameterSet const & p);
  virtual ~Integrator();

  void produce(art::Event & e) override;
private:
  art::InputTag wfm_producer_tag_;
  art::InputTag ch_producer_tag_;
  double threshold;
  // Declare member data here.

};


darkart::Integrator::Integrator(fhicl::ParameterSet const & p) :
  wfm_producer_tag_(p.get<std::string>("wfm_producer_tag")),
  ch_producer_tag_(p.get<std::string>("ch_producer_tag")),
  threshold(p.get<double>("threshold"))
{
  produces<darkart::WaveformVec>();
  produces<darkart::WaveformAssns>();
  produces<darkart::WaveformInfoVec>();
  produces<darkart::WaveformInfoAssns>();
}

darkart::Integrator::~Integrator()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::Integrator::produce(art::Event & e)
{
  //get channel info
  art::Handle<darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_producer_tag_ , chVecHandle);
  darkart::ChannelVec const& chVec(*chVecHandle);

  //make products
  std::unique_ptr<darkart::WaveformVec> integratedWfms(new darkart::WaveformVec);
  std::unique_ptr<darkart::WaveformAssns> integratedWfmAssns(new darkart::WaveformAssns);
  std::unique_ptr<darkart::WaveformInfoVec> integratedWfmInfos(new darkart::WaveformInfoVec);
  std::unique_ptr<darkart::WaveformInfoAssns> integratedWfmInfoAssns(new darkart::WaveformInfoAssns);

  //find integral info
  darkart::ByChannel<darkart::Waveform> get_wfm(chVecHandle, e, wfm_producer_tag_);
  for(auto const& ch:chVec){
    darkart::Channel::ChannelID const chID = ch.channelID;
    auto const& wfm = get_wfm(chID); //Ptr.
    
    //initialize variables
    darkart::Waveform integratedWfm;
    darkart::WaveformInfo integratorData;

    if (!wfm->isValid()) {
      // save default constructed object
      LOG_DEBUG("DataState") << "Invalid state for input waveform for channel "
                             << ch.channel_id();
    }
    else {
      //integrate waveform, and create waveform data
      darkart::integrator(*wfm, integratedWfm, integratorData, threshold);
      if (!integratedWfm.isValid() || !integratorData.isValid())
        LOG_WARNING("Algorithm")<<"Inserting object in non-valid state for channel "<<ch.channel_id();
    }
    
    integratedWfms->push_back(integratedWfm);
    integratedWfmInfos->push_back(integratorData);

    //create associations
    util::createAssn(*this, e,
		      chVecHandle, ch,
		      *integratedWfms, integratedWfm,
		      *integratedWfmAssns);

    util::createAssn(*this, e,
		      chVecHandle,ch,
		      *integratedWfmInfos, integratorData,
		      *integratedWfmInfoAssns);
    
  }//finish loop of channels
  //put into event
  e.put(std::move(integratedWfms));
  e.put(std::move(integratedWfmAssns));
  e.put(std::move(integratedWfmInfoAssns));
  e.put(std::move(integratedWfmInfos));

}

DEFINE_ART_MODULE(darkart::Integrator)
