////////////////////////////////////////////////////////////////////////
// Class:       PhysPulseFinder
// Module Type: producer
// File:        PhysPulseFinder_module.cc
//
// Generated at Wed Sep 11 18:38:27 2013 by Richard Saldanha using artmod
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

#include "darkart/Products/Pulse.hh"
#include "darkart/Products/Waveform.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/ProductAssns.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/PmtHelper.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/pulsefinder_algs.hh"

#include <memory>

//-----------------------------------------------------------------------
// Class PhysPulseFinder is an EDProducer module that creates Pulse objects
// The module searches for pulses on each channel independently 
//-----------------------------------------------------------------------

namespace darkart 
{
    class PhysPulseFinder;
}

class darkart::PhysPulseFinder : public art::EDProducer 
{
public:
    explicit PhysPulseFinder(fhicl::ParameterSet const & p);
    virtual ~PhysPulseFinder();

    void produce(art::Event & e) override;

private:

    // Declare member data here.

    //Input tags for all modules whose output we need
    art::InputTag ch_producer_tag_;
    art::InputTag bs_wfm_producer_tag_;
    art::InputTag int_wfm_producer_tag_;
    art::InputTag pmt_tag_;

    //Vector of channels to skip
    std::vector<int> skip_channels_;
  
    //Structure to hold parameters for pulse-finding algorithm
    CurvatureSearchParams curvatureSearchParams;
};


darkart::PhysPulseFinder::PhysPulseFinder(fhicl::ParameterSet const & p):
    ch_producer_tag_(p.get<std::string>("ch_producer_tag")),
    bs_wfm_producer_tag_(p.get<std::string>("bs_wfm_producer_tag")),
    int_wfm_producer_tag_(p.get<std::string>("int_wfm_producer_tag")),
    pmt_tag_(p.get<std::string>("pmt_tag")),   
    skip_channels_(p.get<std::vector<int> >("skip_channels", std::vector<int>())),
    curvatureSearchParams(p.get<fhicl::ParameterSet>("curvature_search_params"))
{
    // Call appropriate Produces<>() functions here.
    produces<darkart::PulseVec>();
    produces<darkart::PulseAssns>();
}

darkart::PhysPulseFinder::~PhysPulseFinder()
{
    // Clean up dynamic memory and other resources here.
}

void darkart::PhysPulseFinder::produce(art::Event & e)
{
    // Retrieve the ChannelVec data product for physical channels from the event.
    // The e.getValidHandle<>() method does not play well with making art::Assns,
    // so use explicit art::Handles.                                              
    art::Handle<darkart::ChannelVec> physChVecHandle;
    e.getByLabel(ch_producer_tag_, physChVecHandle);
    darkart::ChannelVec const& physChVec(*physChVecHandle);

    //Make the products, which begin empty
    std::unique_ptr<darkart::PulseVec> physPulseVec(new darkart::PulseVec);
    std::unique_ptr<darkart::PulseAssns> physPulseAssns(new darkart::PulseAssns);

    int idx = -1;
  

    darkart::ByChannel<darkart::Waveform> get_bs_wfm(physChVecHandle, e, bs_wfm_producer_tag_);
    darkart::ByChannel<darkart::Waveform> get_int_wfm(physChVecHandle, e, int_wfm_producer_tag_);
    darkart::PmtHelper get_pmt(e, pmt_tag_);

    //Loop over all physical channels
    for(auto const& ch:physChVec)
    {
	darkart::Channel::ChannelID const& chID = ch.channelID;
        auto const& bs_wfm = get_bs_wfm(chID);
        auto const& int_wfm = get_int_wfm(chID);

	//Get spe_mean for the channel
	auto const& pmt = get_pmt(chID); //util::getByChannelID <darkart::Pmt> (e, physChVecHandle, chID, pmt_tag_);
	double spe_mean = pmt.spe_mean;
									     
	//Create a temporary pulse vector for the search algorithm
	darkart::PulseVec tempPulseVec;

	// If told to skip this channel, then don't do anything
	if (util::skipChannel(ch, skip_channels_)) 
	{
	    //save default constructed object
	}
	else if (!bs_wfm->isValid()) 
	{
          //save default constructed object
	    LOG_DEBUG("DataState") << "Invalid state for input waveform for channel "
                                   << ch.channel_id();
	}
	else 
	{
	    //Call algorithm to find pulses on the sum channel
	  tempPulseVec = darkart::curvatureSearch(*bs_wfm, *int_wfm, spe_mean, ch.sample_rate, curvatureSearchParams);
	}
	//Create associations individually for each pulse
        int pulse_id = -1;
	for (auto & pulse:tempPulseVec)
	{
            pulse.pulseID.pulse_id = ++pulse_id;
	    pulse.pulseID.pulse_index = ++idx;
	    (*physPulseVec).push_back(pulse);
	    util::createAssn(*this, e,
			     physChVecHandle, ch,
			     *physPulseVec, pulse,
			     *physPulseAssns);
	}  
    }

    //Put complete products into the event
    e.put(std::move(physPulseVec));
    e.put(std::move(physPulseAssns));
}

DEFINE_ART_MODULE(darkart::PhysPulseFinder)
