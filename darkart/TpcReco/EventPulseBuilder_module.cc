////////////////////////////////////////////////////////////////////////
// Class:       EventPulseBuilder
// Module Type: producer
// File:        EventPulseBuilder_module.cc
//
// Generated at Sat Nov  2 14:33:58 2013 by Alden Fan using artmod
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

#include "darkart/Products/Pulse.hh"
#include "darkart/Products/Waveform.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/Baseline.hh"
#include "darkart/Products/PulseParam.hh"
#include "darkart/Products/ProductAssns.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/ManyByChannel.hh"
#include "darkart/TpcReco/PmtHelper.hh"
#include "darkart/TpcReco/ByPulse.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/common_algs.hh"

#include <memory>
#include <algorithm>

//---------------------------------------------------------------------
// This module creates event-level objects relevant to physical pulses,
// including darkart::Pulse, darkart::PulseParam, and anything else
// associated with a physical pulse.
//-------------------------------------------------------------------





namespace darkart {
  class EventPulseBuilder;
}

class darkart::EventPulseBuilder : public art::EDProducer {
public:
  explicit EventPulseBuilder(fhicl::ParameterSet const & p);
  virtual ~EventPulseBuilder();

  void produce(art::Event & e) override;


private:

  // Declare member data here.
  std::vector<int> skip_channels_;

  // Input tags for all modules whose output we need.
  art::InputTag ch_tag_;
  art::InputTag pmt_tag_;
  art::InputTag pulse_tag_;
  art::InputTag pulse_param_tag_;
  art::InputTag sumch_tag_;
  art::InputTag sumch_pulse_tag_;

};


darkart::EventPulseBuilder::EventPulseBuilder(fhicl::ParameterSet const & p) :
  // Initialize member data here.
  skip_channels_(p.get<std::vector<int> >("skip_channels", std::vector<int>())),
  ch_tag_(p.get<std::string>("ch_tag")),
  pmt_tag_(p.get<std::string>("pmt_tag")),
  pulse_tag_(p.get<std::string>("pulse_tag")),
  pulse_param_tag_(p.get<std::string>("pulse_param_tag")),
  sumch_tag_(p.get<std::string>("sumch_tag")),
  sumch_pulse_tag_(p.get<std::string>("sumch_pulse_tag"))
{
  // Call appropriate Produces<>() functions here.
  produces<darkart::PulseVec>();
  produces<darkart::PulseParamVec>();
  produces<darkart::PulseParamAssns>();
}

darkart::EventPulseBuilder::~EventPulseBuilder()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::EventPulseBuilder::produce(art::Event & e)
{
  // Check if SumPulseFinder was run.
  // TODO move this check to after making the empty products
  if (pulse_tag_ != sumch_pulse_tag_)
    {
      throw cet::exception("Configuration") << "SumPulseFinder was not run!" << std::endl;
    }

  // Make the products, which begin empty.
  std::unique_ptr<darkart::PulseVec> pulseVec(new darkart::PulseVec);
  std::unique_ptr<darkart::PulseParamVec> pulseParamVec(new darkart::PulseParamVec);
  std::unique_ptr<darkart::PulseParamAssns> plsParamAssns(new darkart::PulseParamAssns);

  
  art::Handle <darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_tag_, chVecHandle);
  darkart::ChannelVec const& chVec(*chVecHandle);

  art::Handle <darkart::ChannelVec> sumChVecHandle;
  e.getByLabel(sumch_tag_, sumChVecHandle);

  // Create a pulseVec handle to make associations to pulses.
  art::Handle <darkart::PulseVec> plsVecHandle;
  e.getByLabel(pulse_tag_, plsVecHandle);
  

  darkart::ManyByChannel<darkart::Pulse> get_sum_pulse_vec(sumChVecHandle, e, sumch_pulse_tag_);
  darkart::ManyByChannel<darkart::Pulse> get_pulse_vec(chVecHandle, e, pulse_tag_);
  darkart::ByPulse<darkart::PulseParam> get_pulse_param(plsVecHandle, e, pulse_param_tag_);
  darkart::PmtHelper get_pmt(e, pmt_tag_);

  auto sumPlsPtrVec = get_sum_pulse_vec(darkart::Channel::SUMCH_ID);  //vector< Ptr<Pulse> >
  // for consistency, sort the sum channel pulse vector, even though length is usually only 1
  std::sort(sumPlsPtrVec.begin(), sumPlsPtrVec.end());

  // 2013-11-2 -- For the current analysis, the event-level darkart::Pulse objects are
  // just the same as the sum channel Pulse objects, which are also the Pulse objects
  // in the physical channels. The darkart::PulseParam and similar objects are different
  // at the sum channel, physical channel, and event levels.

  int pulse_idx = -1;
  for (auto const& sumPulse : sumPlsPtrVec)
    {
      // sumPulse is Ptr.

      ++pulse_idx;
      
      // Create new Pulse object
      (*pulseVec).emplace_back(*sumPulse);
      auto & pulse = (*pulseVec).back();
      // Copy constructor dictates pulseID.pulse_id, but we set pulseID.pulse_index
      // explicitly here to ensure we know where the pulses are.
      pulse.pulseID.pulse_index = pulse_idx;


      // Build up the variables in the pulse-related objects by looping over the physical
      // channels. This might seem inefficient but with the new product retrieval method, it
      // should be pretty fast. TODO: do a more detailed study to verify this.

      double max_ch_npe = 0;
      
      // Create new PulseParam object
      (*pulseParamVec).emplace_back();
      auto & pulseParam = (*pulseParamVec).back();
      bool found_peak = true;
      bool peak_saturated = false;
      bool fixed_int1_valid = true;
      bool fixed_int2_valid = true;
      for (auto const& ch : chVec)
        {
          // If told to skip this channel, then don't do anything
          if (util::skipChannel(ch, skip_channels_)) continue;

          auto const& pmt = get_pmt(ch.channelID);
          
          // Retrieve the corresponding Pulse object for this channel
          auto chPulseVec = get_pulse_vec(ch.channelID);
          // Must sort the pulse vector because order is not always preserved by getter.
          std::sort(chPulseVec.begin(), chPulseVec.end());
          darkart::Pulse const& chPulse = *(chPulseVec.at(pulse_idx));
          if (chPulse.pulse_id() != pulse.pulse_id()) {
            LOG_ERROR("Product Retrieval") << "PulseParam object is not the one that was expected."
                                           << std::endl;
          }

          // Retrieve the associated PulseParam object.
          auto const& chPulseParam = *get_pulse_param(chPulse.pulseID);
          found_peak *= chPulseParam.found_peak;
          // leave peak_index field empty
          // leave peak_time field empty
          // leave peak_ampltidue field empty
          pulseParam.integral += -chPulseParam.integral/pmt.spe_mean;
          peak_saturated += chPulseParam.peak_saturated;
          pulseParam.f_param.resize(chPulseParam.f_param.size());
          for (size_t i=0; i<chPulseParam.f_param.size(); i++) {
            pulseParam.f_param[i] += chPulseParam.f_param[i] * chPulseParam.npe;
            pulseParam.f_param_fixed[i] += chPulseParam.f_param_fixed[i] * chPulseParam.npe_fixed;
          }
          pulseParam.f90 += chPulseParam.f90 * chPulseParam.npe;
          pulseParam.f90_fixed += chPulseParam.f90_fixed * chPulseParam.npe_fixed;
          // leave tXX fields empty
          pulseParam.fixed_int1 += -chPulseParam.fixed_int1/pmt.spe_mean;
          pulseParam.fixed_int2 += -chPulseParam.fixed_int2/pmt.spe_mean;
          fixed_int1_valid *= chPulseParam.fixed_int1_valid;
          fixed_int2_valid *= chPulseParam.fixed_int2_valid;
          pulseParam.npe += chPulseParam.npe;
          pulseParam.npe_fixed += chPulseParam.npe_fixed;

          if (chPulseParam.npe > max_ch_npe) {
            pulseParam.max_ch = ch.channel_id();
            max_ch_npe = chPulseParam.npe;
          }

          // build up pulse ROIs
          pulseParam.pulse_rois.resize(chPulseParam.pulse_rois.size());
          for (size_t i=0; i<chPulseParam.pulse_rois.size(); i++) {
            pulseParam.pulse_rois[i] += (chPulseParam.pulse_rois[i] == 999 ? -1000 : -chPulseParam.pulse_rois[i]/pmt.spe_mean);
          }

        }// end loop over channels

      for (size_t i=0; i<pulseParam.f_param.size(); i++) {
        pulseParam.f_param[i] /= pulseParam.npe;
        pulseParam.f_param_fixed[i] /= pulseParam.npe_fixed;
      }
      pulseParam.f90 /= pulseParam.npe;
      pulseParam.f90_fixed /= pulseParam.npe_fixed;
      pulseParam.found_peak = found_peak;
      pulseParam.peak_saturated = peak_saturated;
      pulseParam.fixed_int1_valid = fixed_int1_valid;
      pulseParam.fixed_int2_valid = fixed_int2_valid;

      // Create association between pulse and pulseParam
      util::createAssn(*this, e,
                       *pulseVec, pulse,
                       *pulseParamVec, pulseParam,
                       *plsParamAssns);
      
    } // end loop over pulses
    
  
  

  // Put the complete products into the event.
  e.put(std::move(pulseVec));
  e.put(std::move(pulseParamVec));
  e.put(std::move(plsParamAssns));


  
}

DEFINE_ART_MODULE(darkart::EventPulseBuilder)
