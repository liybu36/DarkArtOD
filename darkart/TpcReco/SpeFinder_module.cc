////////////////////////////////////////////////////////////////////////
// Class:       SpeFinder
// Module Type: producer
// File:        SpeFinder_module.cc
//
// Generated at Fri Nov  8 11:48:56 2013 by Masayuki Wada using artmod
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
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/Products/Baseline.hh"
#include "darkart/Products/Pmt.hh"
#include "darkart/Products/ProductAssns.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/Pulse.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/ManyByChannel.hh"
#include "darkart/TpcReco/ByPulse.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/common_algs.hh"

#include "darkart/TpcReco/spefinder_algs.hh"

#include <memory>
#include <functional>   // std::binary_function
#include <algorithm>    // std::transform

namespace darkart {
  class SpeFinder;
}

    
class darkart::SpeFinder : public art::EDProducer {
public:
  explicit SpeFinder(fhicl::ParameterSet const & p);
  virtual ~SpeFinder();

  void produce(art::Event & e) override;

private:

  // Declare member data here.
  std::vector<int> skip_channels_;
  art::InputTag ch_tag_;
  art::InputTag raw_wfm_tag_;
  art::InputTag baseline_tag_;
  art::InputTag bs_wfm_tag_;
  art::InputTag int_wfm_tag_;
  art::InputTag pulse_tag_;
  art::InputTag pulse_param_tag_;
  art::InputTag event_pulse_tag_;
  SpeFinderParams speFinderParams_;

};


darkart::SpeFinder::SpeFinder(fhicl::ParameterSet const & p) :
  // Initialize member data here.
  skip_channels_(p.get<std::vector<int> >("skip_channels", std::vector<int>())),
  ch_tag_(p.get<std::string>("ch_tag")),
  raw_wfm_tag_(p.get<std::string>("raw_wfm_tag")),
  baseline_tag_(p.get<std::string>("baseline_tag")),
  bs_wfm_tag_(p.get<std::string>("bs_wfm_tag")),
  int_wfm_tag_(p.get<std::string>("int_wfm_tag")),
  pulse_tag_(p.get<std::string>("pulse_tag")),
  pulse_param_tag_(p.get<std::string>("pulse_param_tag")),
  event_pulse_tag_(p.get<std::string>("event_pulse_tag")),
  speFinderParams_(p.get<fhicl::ParameterSet>("speFinderParams"))
{
  // Call appropriate Produces<>() functions here.
  produces<darkart::SpeVec>();
  produces<darkart::SpeAssns>();
}

darkart::SpeFinder::~SpeFinder()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::SpeFinder::produce(art::Event & e)
{
  // Retrieve the ChannelVec data product from the event.
  // The e.getValidHandle<>() method does not play well with making art::Assns,
  // so use explicit art::Handles.
  art::Handle <darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_tag_, chVecHandle);
  darkart::ChannelVec const& chVec(*chVecHandle);

  // Create a pulseVec handle to make associations to pulses.
  art::Handle <darkart::PulseVec> plsVecHandle;
  e.getByLabel(pulse_tag_, plsVecHandle);

  // Retrieve the event-level pulse objects
  art::Handle<darkart::PulseVec> eventPulseVecHandle;
  e.getByLabel(event_pulse_tag_, eventPulseVecHandle);
  //auto const& evtPlsVec(*eventPulseVecHandle);
  

  // Make our products, which begin empty
  std::unique_ptr <darkart::SpeVec> speVec(new darkart::SpeVec);
  std::unique_ptr <darkart::SpeAssns> speAssns(new darkart::SpeAssns);


  // Fill the products. Call the algorithm to find and subtract the baseline.
  // Loop through the channels here, rather than in a separate algorithms file
  // to better ensure that products get mapped to the correct channel. For each
  // new object, add an entry to the corresponding Assns object.
  darkart::ByChannel<darkart::Waveform> get_wfm(chVecHandle, e, raw_wfm_tag_);
  darkart::ByChannel<darkart::Baseline> get_baseline(chVecHandle, e, baseline_tag_);
  darkart::ByChannel<darkart::Waveform> get_bs_wfm(chVecHandle, e, bs_wfm_tag_);
  darkart::ByChannel<darkart::Waveform> get_int_wfm(chVecHandle, e, int_wfm_tag_);
  darkart::ManyByChannel<darkart::Pulse> get_pulse_vec(chVecHandle, e, pulse_tag_);
  darkart::ByPulse<darkart::PulseParam> get_pulse_param(plsVecHandle, e, pulse_param_tag_);
  darkart::ByPulse<darkart::PulseParam> get_evt_pulse_param(eventPulseVecHandle, e, event_pulse_tag_);
  for (auto const& chInfo : chVec) {

    //Create a temporary pulse vector for the search algorithm
    darkart::PulseVec tempPulseVec;

    // If told to skip this channel, then don't do anything
    if (util::skipChannel(chInfo, skip_channels_))
      continue;


    // only process real channels
    if(chInfo.channel_id() < 0 ) {
      LOG_DEBUG("SpeFinder") << "channel_id < 0";
      continue;
    }

    // Retrieve the input raw waveform from the event
    darkart::Channel::ChannelID const chID = chInfo.channelID;
    //auto const& wfm = get_wfm(chID); //Ptr.
    auto const& bsData = get_baseline(chID); //Ptr.
    auto const& bsWfm = get_bs_wfm(chID); //Ptr.
    auto const& intWfm = get_int_wfm(chID); //Ptr.
    auto plsVecPtr = get_pulse_vec(chID); //vector< Ptr<Pulse> >
    // sort plsVec because getter doesn't always preserve order
    std::sort(plsVecPtr.begin(), plsVecPtr.end());

    //need a good baseline
    if(!bsData->found_baseline) {
      LOG_DEBUG("DataState") << "Baseline is no good";
      continue;
    }
    //make sure there is a valid pulse found
    if(plsVecPtr.size() <= 0) {
      LOG_DEBUG("DataState") << "No valid pulse";
      continue;
    }
    //exclude high energy events
    auto const& evtPlsParam = *get_evt_pulse_param(eventPulseVecHandle->at(0).pulseID);
    if (evtPlsParam.fixed_int1 > speFinderParams_.max_npe)
      continue;

    //art::FindOne<darkart::PulseParam> fa(plsVecHandle, e, pulse_param_tag_); // Get pulseParam for each pulse
    //int pulse_id = plsVecPtr[0]->pulse_id();
    //cet::maybe_ref<darkart::PulseParam const> result(fa.at(pulse_id));
    //darkart::PulseParam plsParam = result.ref();
    darkart::PulseParam const& plsParam = *get_pulse_param(plsVecPtr[0]->pulseID);
    double npeOfFirstPls = plsParam.npe;
      
    // manually strip off the art::Ptr part of the plsVec. This will eventually be done with a utility function.
    darkart::PulseVec plsVec;
    plsVec.reserve(plsVec.size());
    for (auto const& pls : plsVecPtr) {
      // pls is Ptr
      plsVec.push_back(*pls);
    }
    
    auto const& tempSpeVec = darkart::findSPEs(chInfo, *bsWfm, *intWfm, plsVec, npeOfFirstPls, speFinderParams_);
    //auto const& tempSpeVec = darkart::findSPEs(chInfo, *bsWfm, *intWfm, plsVecPtr, npeOfFirstPls, speFinderParams_);
      
    //Create associations individually for each pulse
    for (auto const& spe:tempSpeVec)
      {
	(*speVec).push_back(spe);
	util::createAssn(*this, e,
			 chVecHandle, chInfo,
			 *speVec, spe,
			 *speAssns);
      }

  }


  // Put our complete products into the Event
  e.put(std::move(speVec));
  e.put(std::move(speAssns));

}


DEFINE_ART_MODULE(darkart::SpeFinder)
