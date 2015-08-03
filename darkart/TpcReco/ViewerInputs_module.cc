////////////////////////////////////////////////////////////////////////
// Class:       ViewerInputs
// Module Type: filter
// File:        ViewerInputs_module.cc
//
// Generated at Tue Oct 29 00:39:31 2013 by Alden Fan using artmod
// from cetpkgsupport v1_02_01.
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
  class ViewerInputs;
}

class darkart::ViewerInputs : public art::EDFilter {
public:
  explicit ViewerInputs(fhicl::ParameterSet const & p);
  //explicit ViewerInputs();
  virtual ~ViewerInputs();

  bool filter(art::Event & e) override;


private:
  
  uint next_event_to_process;
};


darkart::ViewerInputs::ViewerInputs(fhicl::ParameterSet const & ) :
  next_event_to_process(0)
{ }

darkart::ViewerInputs::~ViewerInputs()
{ }

bool darkart::ViewerInputs::filter(art::Event & e)
{
  bool result = false;

  if (e.event() == next_event_to_process)
    result = true;
  else if (e.event() > next_event_to_process) {

    while (!result) {
      std::cout << "\nSpecify next event to view or press ENTER to view event "<<e.event()<<": ";
    
      std::string line;
      getline(std::cin, line);
      if( line == "" ) {
        result = true;
      }
      else if (line == "q" || line == "Q" || line == "-1")
        throw cet::exception("QuitViewer") << "This message is expected because of the clumsy way of quitting art that I have implemented right now."<<std::endl;
      else {
        next_event_to_process = atoi(line.c_str());
        if (next_event_to_process == e.event())
          result = true;
        else if (next_event_to_process > e.event()) {
          result = false;
          break;
        }
        else
          result = false;
      }
    }
  }

  return result;
}

DEFINE_ART_MODULE(darkart::ViewerInputs)
