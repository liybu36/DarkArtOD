////////////////////////////////////////////////////////////////////////
// Class:       SumPulseFinder
// Module Type: producer
// File:        SumPulseFinder_module.cc
//
// Generated at Sun Sep  8 18:31:54 2013 by Richard Saldanha using artmod
// from cetpkgsupport v1_02_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include "darkart/Products/Pulse.hh"
#include "darkart/Products/Waveform.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/ProductAssns.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/pulsefinder_algs.hh"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

//-----------------------------------------------------------------------
// Class SumPulseFinder is an EDProducer module that creates Pulse objects
// The module searches for pulses on the sum channel and copies over the
// pulses on to all other physical channels 
//-----------------------------------------------------------------------

namespace darkart 
{
    class SumPulseFinder;
}

class darkart::SumPulseFinder : public art::EDProducer 
{
public:
    explicit SumPulseFinder(fhicl::ParameterSet const & p);
    virtual ~SumPulseFinder();
    
    void produce(art::Event & e) override;
    
private:
    
    // Declare member data here.
    
    //Input tags for all modules whose output we need
    art::InputTag ch_producer_tag_;
    art::InputTag sum_ch_producer_tag_;
    art::InputTag sum_bs_wfm_producer_tag_;
    art::InputTag sum_int_wfm_producer_tag_;

    //Vector of channels to skip
    std::vector<int> skip_channels_;
    
    //Structure to hold parameters for pulse-finding algorithm
    CurvatureSearchParams curvatureSearchParams;
};


darkart::SumPulseFinder::SumPulseFinder(fhicl::ParameterSet const & p):
    ch_producer_tag_(p.get<std::string>("ch_producer_tag")),
    sum_ch_producer_tag_(p.get<std::string>("sum_ch_producer_tag")),
    sum_bs_wfm_producer_tag_(p.get<std::string>("sum_bs_wfm_producer_tag")),
    sum_int_wfm_producer_tag_(p.get<std::string>("sum_int_wfm_producer_tag")),
    skip_channels_(p.get<std::vector<int> >("skip_channels", std::vector<int>())),
    curvatureSearchParams(p.get<fhicl::ParameterSet>("curvature_search_params"))
{
    // Call appropriate Produces<>() functions here.
    produces<darkart::PulseVec>();
    produces<darkart::PulseAssns>();
}

darkart::SumPulseFinder::~SumPulseFinder()
{
    // Clean up dynamic memory and other resources here.
}

void darkart::SumPulseFinder::produce(art::Event & e)
{
    // Retrieve the ChannelVec data product for the sum channel from the event.
    // The e.getValidHandle<>() method does not play well with making art::Assns,
    // so use explicit art::Handles.                                              
    art::Handle<darkart::ChannelVec> sumChVecHandle;
    e.getByLabel(sum_ch_producer_tag_, sumChVecHandle);
    
    // Retrieve the Channel object for the sum channel from the ChannelVec
    darkart::Channel const& sumChannel = util::getByChannelID(sumChVecHandle, darkart::Channel::SUMCH_ID);

    
    // Retrieve the baseline-subtracted waveform for the sum channel from the ChannelVec
    darkart::ByChannel<darkart::Waveform> get_wfm(sumChVecHandle, e, sum_bs_wfm_producer_tag_);
    auto const& sum_wfm = get_wfm(darkart::Channel::SUMCH_ID); //Ptr.

    
    // Retrieve the integrated waveform for the sum channel from the ChannelInfoVec
    darkart::ByChannel<darkart::Waveform> get_int_wfm(sumChVecHandle, e, sum_int_wfm_producer_tag_);
    auto const& sum_int_wfm = get_int_wfm(darkart::Channel::SUMCH_ID); //Ptr.


    
    // Retrieve the ChannelVec data product for physical channels from the event.
    // The e.getValidHandle<>() method does not play well with making art::Assns,
    // so use explicit art::Handles.
    art::Handle<darkart::ChannelVec> physChVecHandle;
    e.getByLabel(ch_producer_tag_, physChVecHandle);
    darkart::ChannelVec const& physChVec(*physChVecHandle);

    //Make the products, which begin empty
    std::unique_ptr<darkart::PulseVec> sumPulseVec(new darkart::PulseVec);
    std::unique_ptr<darkart::PulseAssns> sumPulseAssns(new darkart::PulseAssns);

    //SPE mean for the sum channel = 1
    double sum_spe_mean = 1;

    int idx = -1;
  
    if (!sum_wfm->isValid() || !sum_int_wfm->isValid()) 
    {
      // save default constructed objects
	LOG_DEBUG("DataState") << "Invalid state for input sum channel waveform.";
    }
    else 
    {
	//Call algorithm to find pulses on the sum channel

	//Create a temporary pulse vector for the search algorithm
	darkart::PulseVec tempPulseVec;
	tempPulseVec = darkart::curvatureSearch(*sum_wfm, *sum_int_wfm, sum_spe_mean,
                                                sumChannel.sample_rate, curvatureSearchParams);

	//Copy over pulses on to all physical channels
	for(auto const& ch:physChVec)
	{
	    // If told to skip this channel, then don't do anything
	    if (util::skipChannel(ch, skip_channels_))
		continue;
        
	    //COPY over pulses and create associations to physical channels individually for
            //each pulse. Hence we do NOT pass by reference.
            int pulse_id = -1;
	    for (auto & pulse:tempPulseVec)
	    {
                pulse.pulseID.pulse_id = ++pulse_id;
		pulse.pulseID.pulse_index = ++idx;
		(*sumPulseVec).push_back(pulse);
		util::createAssn(*this, e,
				 physChVecHandle, ch,
				 *sumPulseVec, pulse,
				 *sumPulseAssns);
	    }  
	}

        //Create association of pulses directly to sum channel, so pass by reference.
        int pulse_id = -1;
	for (auto & pulse:tempPulseVec)
	{
            pulse.pulseID.pulse_id = ++pulse_id;
	    pulse.pulseID.pulse_index = ++idx;
	    (*sumPulseVec).push_back(pulse);
	    util::createAssn(*this, e,
			     sumChVecHandle, sumChannel,
			     *sumPulseVec, pulse,
			     *sumPulseAssns);
	}
    }
    //Put complete products into the event
    e.put(std::move(sumPulseVec));
    e.put(std::move(sumPulseAssns));
}

DEFINE_ART_MODULE(darkart::SumPulseFinder)
