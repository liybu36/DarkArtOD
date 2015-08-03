////////////////////////////////////////////////////////////////////////
// Class:       InEventlist
// Module Type: filter
// File:        InEventlist_module.cc
//
// Generated at Wed Oct 16 13:32:11 2013 by Alden Fan using artmod
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

#include <memory>
#include <iostream>
#include <fstream>
#include <string>

//-----------------------------------------------------------------------
// This module reads an eventlist file with where the first entry of each
// line is an event ID. It tells art to process only events that are in
// the eventlist.
//
// NOTE: THIS RELIES ON THE FACT THAT art::Event::event() AND
// ds50::V1495Fragment::trigger_counter() RETURN THE SAME VALUE!
//-----------------------------------------------------------------------


namespace darkart {
  class InEventlist;
}

class darkart::InEventlist : public art::EDFilter {
public:
  explicit InEventlist(fhicl::ParameterSet const & p);
  virtual ~InEventlist();

  bool filter(art::Event & e) override;

  void beginJob() override;
  void endJob() override;

private:

  // Declare member data here.
  std::ifstream infile_;
  std::string eventlist_;
  int run_id_;
  int event_id_;

  std::string line_;
};


darkart::InEventlist::InEventlist(fhicl::ParameterSet const & p) :
  eventlist_(p.get<std::string>("eventlist"))
{ }

darkart::InEventlist::~InEventlist()
{ }

bool darkart::InEventlist::filter(art::Event & e)
{

  int art_event_id = e.event();
  int art_run_id = e.run();
  if (art_event_id != event_id_ || art_run_id != run_id_) {
    // we don't want to include this event in processing
    return false;
  }
  else {
    // we want to include this event for processing. but before returning true,
    // first find the next event we will want to include
    std::getline(infile_, line_);
    std::istringstream iss(line_);
    iss >> run_id_ >> event_id_;
    return true;
  }

}

void darkart::InEventlist::beginJob()
{
  infile_.open(eventlist_.c_str());
  if (!infile_.is_open() || !infile_.good())
    throw cet::exception("FileAccess") << "Could not open file " << eventlist_;

  std::getline(infile_, line_);
  std::istringstream iss(line_);
  iss >> run_id_ >> event_id_;
}


void darkart::InEventlist::endJob()
{
  infile_.close();
}




DEFINE_ART_MODULE(darkart::InEventlist)
