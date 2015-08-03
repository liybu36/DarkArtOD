////////////////////////////////////////////////////////////////////////
// Class:       PositionRecon
// Module Type: producer
// File:        PositionRecon_module.cc
//
// Generated at Mon Nov 11 00:22:07 2013 by Alden Fan using artmod
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
#include "darkart/Products/Channel.hh"
#include "darkart/Products/PulseParam.hh"
#include "darkart/Products/PositionParam.hh"
#include "darkart/Products/ProductAssns.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/ManyByChannel.hh"
#include "darkart/TpcReco/ByPulse.hh"
#include "darkart/TpcReco/PmtHelper.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/common_algs.hh"


#include <algorithm>
#include <memory>

//-------------------------------------------------------------------
// Compute 3D position reconstruction variables. Idea is to compute
// the variables for all pulses, then later decide which pulse is S2
// and use the associated position recon variables to reconstruct
// the event.
//
// Currently, using barycenter, which is definitely wrong.
//-------------------------------------------------------------------

namespace darkart {
  class PositionRecon;
}

class darkart::PositionRecon : public art::EDProducer {
public:
  explicit PositionRecon(fhicl::ParameterSet const & p);
  virtual ~PositionRecon();

  void produce(art::Event & e) override;


private:

  // Declare member data here.
  art::InputTag ch_tag_;
  art::InputTag pmt_tag_;
  art::InputTag pulse_tag_;
  art::InputTag pulseparam_tag_;
  art::InputTag event_pulse_tag_;

  
};


darkart::PositionRecon::PositionRecon(fhicl::ParameterSet const & p) :
  ch_tag_(p.get<std::string>("ch_tag")),
  pmt_tag_(p.get<std::string>("pmt_tag")),
  pulse_tag_(p.get<std::string>("pulse_tag")),
  pulseparam_tag_(p.get<std::string>("pulseparam_tag")),
  event_pulse_tag_(p.get<std::string>("event_pulse_tag"))
{
  produces<darkart::PositionParamVec>();
  produces<darkart::PositionParamAssns>();
}

darkart::PositionRecon::~PositionRecon()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::PositionRecon::produce(art::Event & e)
{
  art::Handle <darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_tag_, chVecHandle);

  art::Handle<darkart::PulseVec> plsVecHandle;
  e.getByLabel(pulse_tag_, plsVecHandle);

  art::Handle<darkart::PulseVec> evtPlsVecHandle;
  e.getByLabel(event_pulse_tag_, evtPlsVecHandle);

  // Make the products, which begin empty.
  std::unique_ptr<darkart::PositionParamVec> posParamVec(new darkart::PositionParamVec);
  std::unique_ptr<darkart::PositionParamAssns> posParamAssns(new darkart::PositionParamAssns);

  darkart::ManyByChannel<darkart::Pulse> get_pulse_vec(chVecHandle, e, pulse_tag_);
  darkart::ByPulse<darkart::PulseParam> get_pulse_param(plsVecHandle, e, pulseparam_tag_);
  darkart::PmtHelper get_pmt(e, pmt_tag_);


  for (auto const& pulse : *evtPlsVecHandle) {
    posParamVec->emplace_back();
    auto & pos_param = posParamVec->back();
    double bary_x=0, bary_y=0;
    double top_fixed_int2 = 0;
    for (auto const& ch : *chVecHandle) {
      auto const& pmt = get_pmt(ch.channelID);
      if (pmt.photocathode_z > 0) {
        //Retrieve the corresponding PulseParam object for this channel
        auto ch_pulse_vec = get_pulse_vec(ch.channelID);
        // sort pulse vector because getter doesn't always preserve order
        std::sort(ch_pulse_vec.begin(), ch_pulse_vec.end());
        darkart::Pulse const& ch_pulse = *(ch_pulse_vec.at(pulse.pulse_id()));
        auto const& ch_pulse_param = *get_pulse_param(ch_pulse.pulseID);
        
        
        top_fixed_int2 += -ch_pulse_param.fixed_int2/pmt.spe_mean;
        bary_x += pmt.photocathode_x * -ch_pulse_param.fixed_int2/pmt.spe_mean;
        bary_y += pmt.photocathode_y * -ch_pulse_param.fixed_int2/pmt.spe_mean;
      }
    }// end loop over channels

    bary_x /= top_fixed_int2;
    bary_y /= top_fixed_int2;


    pos_param.bary_x = bary_x;
    pos_param.bary_y = bary_y;

    util::createAssn(*this, e,
                     evtPlsVecHandle, pulse,
                     *posParamVec, pos_param,
                     *posParamAssns);
  }// end loop over pulses




  
  // Put complete products into the event
  e.put(std::move(posParamVec));
  e.put(std::move(posParamAssns));
}

DEFINE_ART_MODULE(darkart::PositionRecon)
