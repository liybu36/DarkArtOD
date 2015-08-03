////////////////////////////////////////////////////////////////////////
// Author: Jim Kowalkowski
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/InputTag.h"

#include "darkart/Products/VetoTruth.hh"
#include "darkart/ArtModules/VetoPMT.hh"

#include <utility>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <string>

using namespace std;

// -------
// helpers

namespace {

  struct Hit
  {
    Hit(float low, float high): leading_(low),trailing_(high) { }
    Hit(): leading_(0), trailing_(0) { }
    
    float leading_;
    float trailing_;
  };
  
  istream& operator>>(istream& is, Hit& v)
  {
    char c;
    is >> v.leading_;
    is >> c;
    is >> v.trailing_;
    is >> c;
    return is;
  }

  // 01-Oct-2014, KAB - commenting out this function since it is defined
  // but not used, and the 4.9.1 compiler is complaining about that
#if 0
  ostream& operator<<(ostream& os, Hit const& v)
  {
    os << v.leading_ << " " << v.trailing_;
    return os;
  }
#endif
  
  class VetoReader
  {
  public:
    VetoReader(std::string const& fname);
    bool nextVetoEvent(darkart::VetoTruth& out);
  private:
    ifstream is_;
    art::ServiceHandle<darkart::VetoPMT> pmts_;
    int curr_event_;
    std::string line_;
  };

  VetoReader::VetoReader(std::string const& fname):
    is_(fname.c_str()),
    pmts_(art::ServiceHandle<darkart::VetoPMT>()),
    curr_event_(-1)
  {
    if(!is_)
      {
	throw cet::exception("FileOpen")
	  << "cannot open veto sim file named " << fname;
      }

    // FOR THE FUTURE:
    // prime the system by reading in the first line.
    // we assume that there may be more than one line per event.
  }

  bool VetoReader::nextVetoEvent(darkart::VetoTruth& out)
  {
    line_.clear();
    getline(is_,line_);
    if(is_.eof())
      {
	throw cet::exception("FileRead")
	  << "should have gotten veto sim data from input file and read EOF";
      }

    istringstream istr(line_.c_str());
    int event;
    char comma;
    istr >> event >> comma;
    if(istr.eof())
      {
	throw cet::exception("FileRead")
	  << "should have gotten veto sim data event number from input file and read EOF";
      }
    out.sim_event=event;

    // only deals with the file that has one event per line and also one PE per line
    Hit cur;
    for(int t=0;1;++t)
      {
	istr >> cur;
	if(istr.eof()) break;

	if(cur.leading_ > 0.0)
	  {
	    out.hits.push_back( { t, 0, 
		  cur.leading_, cur.trailing_, 
		  pmts_->theta(t), pmts_->phi(t) } );
	  }
      }

    return (is_.eof()) ? false : true;
  }

// ----------
// the module

class ProtoVetoMerger : public art::EDProducer 
{
public:
  explicit ProtoVetoMerger(fhicl::ParameterSet const & p);
  virtual ~ProtoVetoMerger();

  void produce(art::Event & e) override;
  
private:
  std::string sim_file_;
  VetoReader reader_;
};


//----------------------------------------------------------------------------
// The implementation of the module member functions begins here.

ProtoVetoMerger::ProtoVetoMerger(fhicl::ParameterSet const & ps ) :
  sim_file_(ps.get<std::string>("sim_file")),
  reader_(sim_file_)
{
  produces<darkart::VetoTruth>();
}

ProtoVetoMerger::~ProtoVetoMerger()
{ }

void ProtoVetoMerger::produce(art::Event & e)
{
  // Make our products, which begin empty.
  std::unique_ptr<darkart::VetoTruth> veto(new darkart::VetoTruth);

  bool rc = reader_.nextVetoEvent(*veto);

  if(rc==false)
    throw cet::exception("eof") << "got eof from " << sim_file_ << " too early";

  // Put our complete products into the Event.
  e.put(std::move(veto));
}
}


DEFINE_ART_MODULE(ProtoVetoMerger)
