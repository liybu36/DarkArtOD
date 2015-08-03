////////////////////////////////////////////////////////////////////////
// Class:       DisplayInputs
// Module Type: filter
// File:        DisplayInputs_module.cc
//
// Created in May 2015 by Chris Stanford
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "cetlib/exception.h"

#include "darkart/Products/EventInfo.hh"

#include <memory>
#include <iostream>
#include <string>
#include <sstream>

namespace darkart {
  class DisplayInputs;
}

class darkart::DisplayInputs : public art::EDFilter {
public:
  explicit DisplayInputs(fhicl::ParameterSet const & p);
  //explicit DisplayInputs();
  virtual ~DisplayInputs();

  bool filter(art::Event & e) override;


private:
  
  uint next_event_to_process; // Desired event id
  uint consecutive_events;    // Number of consecutive events desired, starting at next_event_to_process
};


darkart::DisplayInputs::DisplayInputs(fhicl::ParameterSet const & p) :
  next_event_to_process(p.get<int>("single_event",0)),
  consecutive_events   (p.get<int>("consecutive_events",1))
{ }

darkart::DisplayInputs::~DisplayInputs()
{ }

bool darkart::DisplayInputs::filter(art::Event & e)
{
  uint event_id = e.event();
  if (event_id % 1000 == 0) std::cout<<"Scanning event "<<event_id<<std::endl;
  if (event_id == next_event_to_process) {
    if (consecutive_events>1) {
      next_event_to_process++;
      consecutive_events--;
    }
    return true;
  }
  if (event_id > next_event_to_process) {
    throw cet::exception("QuitViewer") << "This message is expected because of the clumsy way of quitting art that I have implemented right now."<<std::endl;
  }
  return false;
}

DEFINE_ART_MODULE(darkart::DisplayInputs)
