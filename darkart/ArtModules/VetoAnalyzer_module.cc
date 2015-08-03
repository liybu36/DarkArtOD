////////////////////////////////////////////////////////////////////////
// Class:       VetoAnalyzer
// Module Type: analyzer
// File:        VetoAnalyzer_module.cc
//
// Generated at Tue May 21 15:18:44 2013 by Maria Elena Monzani using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "TH1F.h"
#include "darkart/Products/VetoTruth.hh"
#include <iostream>

namespace darkart {
  class VetoAnalyzer;
}

class darkart::VetoAnalyzer : public art::EDAnalyzer {
public:
  explicit VetoAnalyzer(fhicl::ParameterSet const & p);
  virtual ~VetoAnalyzer();

  void analyze(art::Event const & e) override;


private:
  TH1 * h_hits_;
  // Declare member data here.

};


darkart::VetoAnalyzer::VetoAnalyzer(fhicl::ParameterSet const & p)
:
  EDAnalyzer(p),
  h_hits_(0)
{
  art::ServiceHandle<art::TFileService> tfh;
  h_hits_ = tfh->make<TH1F>("VetoHitMultiplicity","Veto Hit Multiplicity", 50, -10., 1010.);
}

darkart::VetoAnalyzer::~VetoAnalyzer()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::VetoAnalyzer::analyze(art::Event const & e)
{
  auto veto = e.getValidHandle<darkart::VetoTruth>("vetomerge");
  std::cout << veto->hits.size() << std::endl;
  h_hits_->Fill(veto->hits.size());
}

DEFINE_ART_MODULE(darkart::VetoAnalyzer)
