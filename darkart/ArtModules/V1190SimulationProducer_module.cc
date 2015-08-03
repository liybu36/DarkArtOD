////////////////////////////////////////////////////////////////////////
// Class:       V1190SimulationProducer
// Module Type: producer
// File:        V1190SimulationProducer_module.cc
//
// Generated at Tue May 21 16:56:08 2013 by Maria Elena Monzani using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "darkart/Products/VetoTruth.hh"
#include "artdaq-core/Data/Fragments.hh"
#include "darksidecore/Data/V1190FragmentWriter.hh"

#include <memory>

namespace darkart {
  class V1190SimulationProducer;
}

class darkart::V1190SimulationProducer : public art::EDProducer {
public:
  explicit V1190SimulationProducer(fhicl::ParameterSet const & p);
  virtual ~V1190SimulationProducer();

  void produce(art::Event & e) override;


private:

  // Declare member data here.

};


darkart::V1190SimulationProducer::V1190SimulationProducer(fhicl::ParameterSet const & )
// :
// Initialize member data here.
{
  // Call appropriate Produces<>() functions here.
  produces<artdaq::Fragments>();
}

darkart::V1190SimulationProducer::~V1190SimulationProducer()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::V1190SimulationProducer::produce(art::Event & e)
{
  auto veto = e.getValidHandle<darkart::VetoTruth>("vetomerge");
  std::unique_ptr<artdaq::Fragments> prod(new artdaq::Fragments(1));
  auto & frag = prod->back();

  ds50::V1190FragmentWriter vfw(frag);

  vfw.size_for(veto->hits.size()); // Allocate beforehand to avoid resize.

  vfw.add_header(veto->sim_event);
  for (auto const & vHit : veto->hits){
    vfw.add_hit(vHit.pmt_index, vHit.leading_ns, vHit.trailing_ns);
  }
  vfw.add_trailer();

  e.put(std::move(prod));
}

DEFINE_ART_MODULE(darkart::V1190SimulationProducer)
