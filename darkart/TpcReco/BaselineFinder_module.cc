////////////////////////////////////////////////////////////////////////
// Class:       BaselineFinder
// Module Type: producer
// File:        BaselineFinder_module.cc
//
// Generated at Tue Jul  2 19:34:44 2013 by Alden Fan using artmod
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
#include "cetlib/exception.h"

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/Products/Baseline.hh"
#include "darkart/Products/Pmt.hh"
#include "darkart/Products/ProductAssns.hh"
#include "darkart/Products/Channel.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/baselinefinder_algs.hh"
#include "darkart/TpcReco/common_algs.hh"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iostream>
#include <memory>


//----------------------------------------------------------------------------
// Class BaselineFinder is an EDProducer that creates 3 objects for each channel:
// 1. Waveform object to hold the baseline subtracted waveform
// 2. WaveformInfo object to hold some generic info about the Waveform
// 3. BaselineData object to hold info about the baseline
//----------------------------------------------------------------------------


namespace darkart {
  class BaselineFinder;
}

class darkart::BaselineFinder : public art::EDProducer {
public:
  explicit BaselineFinder(fhicl::ParameterSet const & p);
  virtual ~BaselineFinder();

  void produce(art::Event & e) override;

  
private:

  art::InputTag ch_tag_;
  art::InputTag raw_wfm_tag_;
  std::vector<int> skip_channels_;
  //bool fixed_baseline_;
  std::string mode_;
  DriftingBaselineParams driftingBaselineParams;
  FixedBaselineParams fixedBaselineParams;

};


darkart::BaselineFinder::BaselineFinder(fhicl::ParameterSet const & p) :
  ch_tag_(p.get<std::string>("ch_tag")),
  raw_wfm_tag_(p.get<std::string>("raw_wfm_tag")),
  skip_channels_(p.get<std::vector<int> >("skip_channels", std::vector<int>())),
  //fixed_baseline_(p.get<bool>("fixed_baseline")),
  mode_(p.get<std::string>("mode")),
  driftingBaselineParams(p.get<fhicl::ParameterSet>("drifting_baseline_params")),
  fixedBaselineParams(p.get<fhicl::ParameterSet>("fixed_baseline_params"))
  
{
  produces<darkart::WaveformVec>();
  produces<darkart::BaselineVec>();
  produces<darkart::BaselineAssns>();
  produces<darkart::WaveformAssns>();
  produces<darkart::WaveformInfoVec>();
  produces<darkart::WaveformInfoAssns>();
}

darkart::BaselineFinder::~BaselineFinder()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::BaselineFinder::produce(art::Event & e)
{
  // Retrieve the ChannelVec data product from the event.
  // The e.getValidHandle<>() method does not play well with making art::Assns,
  // so use explicit art::Handles.
  art::Handle<darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_tag_, chVecHandle);
  darkart::ChannelVec const& chVec(*chVecHandle);

  
  // Make our products, which begin empty
  std::unique_ptr<darkart::WaveformVec> bsWfmVec(new darkart::WaveformVec);
  std::unique_ptr<darkart::WaveformAssns> bsWfmAssns(new darkart::WaveformAssns);
  std::unique_ptr<darkart::BaselineVec> baselineVec(new darkart::BaselineVec);
  std::unique_ptr<darkart::BaselineAssns> baselineAssns(new darkart::BaselineAssns);
  std::unique_ptr<darkart::WaveformInfoVec> bsWfmInfoVec(new darkart::WaveformInfoVec);
  std::unique_ptr<darkart::WaveformInfoAssns> bsWfmInfoAssns(new darkart::WaveformInfoAssns);


  // Fill the products. Call the algorithm to find and subtract the baseline.
  // Loop through the channels here, rather than in a separate algorithms file
  // to better ensure that products get mapped to the correct channel. For each
  // new object, add an entry to the corresponding Assns object.
  darkart::ByChannel<darkart::Waveform> getwfm(chVecHandle, e, raw_wfm_tag_);
  for (auto const& ch : chVec) {

    // Retrieve the input raw waveform from the event
    darkart::Channel::ChannelID const chID = ch.channelID;
    auto const& wfm = getwfm(chID); //Ptr.
    // Make the channel-level objects, which begin empty
    darkart::Waveform bsWfm;
    darkart::Baseline baseline;
    darkart::WaveformInfo bsWfmInfo;

    if (util::skipChannel(ch, skip_channels_)) {
      //save default-constructed objects
    }
    else if (!wfm->isValid()) {
      //save default-constructed objets
      LOG_DEBUG("DataState") << "Invalid state for input waveform for channel "
                             << ch.channel_id();
    }
    else {
      if (mode_ == "ZERO")
        darkart::zeroBaseline(*wfm, bsWfm, baseline);
      else if (mode_ == "FIXED")
        darkart::fixedBaseline(ch, *wfm, bsWfm, baseline, fixedBaselineParams);
      else if (mode_ == "DRIFTING")
        darkart::driftingBaseline(ch, *wfm, bsWfm, baseline, driftingBaselineParams);
      else
        throw cet::exception("Configuration") << "Could not recognize mode for baseline finder!";
      // Fill the WaveformInfo object
      bsWfmInfo = darkart::fillWaveformInfo(bsWfm);

      if (!bsWfm.isValid() || !baseline.isValid() || !bsWfmInfo.isValid())
        LOG_WARNING("Algorithm") << "Inserting object in non-valid state for channel "<<ch.channel_id();
    }



    // Add the objects to the corresponding vectors
    bsWfmVec->push_back(bsWfm);
    baselineVec->push_back(baseline);
    bsWfmInfoVec->push_back(bsWfmInfo);
    
    // Fill the Assns
    util::createAssn( *this, e,
                      chVecHandle, ch,
                      *bsWfmVec, bsWfm,
                      *bsWfmAssns );

    util::createAssn( *this, e,
                      chVecHandle, ch,
                      *baselineVec, baseline,
                      *baselineAssns );

    util::createAssn( *this, e,
                      chVecHandle, ch,
                      *bsWfmInfoVec, bsWfmInfo,
                      *bsWfmInfoAssns );

  }


  
  // Put our complete products into the Event
  e.put(std::move(bsWfmVec));
  e.put(std::move(baselineVec));
  e.put(std::move(baselineAssns));
  e.put(std::move(bsWfmAssns));
  e.put(std::move(bsWfmInfoVec));
  e.put(std::move(bsWfmInfoAssns));
  
}

DEFINE_ART_MODULE(darkart::BaselineFinder)
