////////////////////////////////////////////////////////////////////////
// Class:       TotalChargeIntegrator
// Module Type: producer
// File:        TotalChargeIntegrator_module.cc
//
// Generated at Tue Mar 24 11:28:39 2015 by Xin Xiang using artmod
// from cetpkgsupport v1_08_04.
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

#include "darkart/ODProducts/TotalCharge.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODReco/Tools/Utilities.hh"

#include <memory>
#include <iostream>

namespace darkart {
  namespace od {
    class TotalChargeIntegrator;
  }
}

class darkart::od::TotalChargeIntegrator : public art::EDProducer {
public:
  explicit TotalChargeIntegrator(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  TotalChargeIntegrator(TotalChargeIntegrator const &) = delete;
  TotalChargeIntegrator(TotalChargeIntegrator &&) = delete;
  TotalChargeIntegrator & operator = (TotalChargeIntegrator const &) = delete;
  TotalChargeIntegrator & operator = (TotalChargeIntegrator &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

  // Selected optional functions.
  // void beginRun(art::Run & r) override; commented to avoid warning


private:

  // Declare member data here.
  int _verbosity;
  std::string _input_pulses;
  bool _save_chan_charge_vec;
};


darkart::od::TotalChargeIntegrator::TotalChargeIntegrator(fhicl::ParameterSet const & p)
  : _verbosity(p.get<int>("verbosity", 0))
  , _input_pulses(p.get<std::string>("input_pulses", "none"))
  , _save_chan_charge_vec(p.get<bool>("save_chan_charge_vec", false))
{
  produces<darkart::od::TotalCharge>();
}

void darkart::od::TotalChargeIntegrator::produce(art::Event & e)
{

  std::unique_ptr<darkart::od::TotalCharge> total_charge(new darkart::od::TotalCharge());
  
  //get channel data
  auto const &channel_data_vec = e.getValidHandle<darkart::od::ChannelDataVec>(_input_pulses);
 
  double lsv_total_integral = 0;
  double wt_total_integral = 0;
 
  const size_t nchan = channel_data_vec->size();
  std::vector<float> chan_integral_vec (nchan, 0);
  for (size_t icd=0; icd<nchan; icd++){

    const darkart::od::ChannelData& channel_data = channel_data_vec->at(icd);
    const int channel_id = channel_data.channel_id;
    const int channel_type = channel_data.channel_type;
    
    //calculate total charge on each individual channel
    double chan_integral = 0;
    const int ch_npulse = channel_data.npulses; //npulses;
    for (int ip=0; ip<ch_npulse; ip++){
      darkart::od::ChannelData::Pulse const& pulse = channel_data.pulses[ip];
      chan_integral += pulse.integral;
    }
    if (channel_id != -1) // Skip the channel if it is blank
      chan_integral_vec[channel_id] = chan_integral;

    //lsv = 1, wt = 2, disabled lsv = -1, disabled wt = -2
    if (channel_type==1) {
      lsv_total_integral += chan_integral;
    }

    if (channel_type==2) {
      wt_total_integral += chan_integral;
    }
     
  } // end ChannelDataVec loop   
  
  if (_save_chan_charge_vec)
    total_charge->chan_charge_vec  = std::move(chan_integral_vec);
  else {
    total_charge->chan_charge_vec = {};
  }
  total_charge->lsv_total_charge = lsv_total_integral;
  total_charge->wt_total_charge  = wt_total_integral;

  e.put(std::move(total_charge));
  
}

//void darkart::od::TotalChargeIntegrator::beginRun(art::Run & r)
//{
  // Implementation of optional member function here.
//}


DEFINE_ART_MODULE(darkart::od::TotalChargeIntegrator)