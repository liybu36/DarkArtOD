////////////////////////////////////////////////////////////////////////
// Class:       PeChargeMaker
// Module Type: producer
// File:        PeChargeMaker_module.cc
//
// Generated at Wed Oct 22 16:49:37 2014 by Alden Fan using artmod
// from cetpkgsupport v1_07_00.
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

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>

#include "TRandom3.h"

#include "darkart/Products/PeHit.hh"
#include "darkart/Products/PeCharge.hh"
#include "darkart/Products/Waveform.hh"
#include "darkart/TpcMC/PeChargeGenerator.hh"
#include "darkart/TpcMC/SignalGenerator.hh"

namespace darkart {
  namespace MC {
    class PeChargeMaker;
  }
}

////////////////////////////////////////////////////////////////////////
// 2014-10-22 AFan 
//
// This is a modularization of the existing electronics MC.
//
// Given PMT hits from MC::PeHitMaker, assign charge to each hit (PE).
// Draw charge from variety of spectra. Generate signal portions of
// waveforms. Overlay on baseline later.
//
////////////////////////////////////////////////////////////////////////



class darkart::MC::PeChargeMaker : public art::EDProducer {
public:
  explicit PeChargeMaker(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  PeChargeMaker(PeChargeMaker const &) = delete;
  PeChargeMaker(PeChargeMaker &&) = delete;
  PeChargeMaker & operator = (PeChargeMaker const &) = delete;
  PeChargeMaker & operator = (PeChargeMaker &&) = delete;

  // Required functions.
  void beginJob() override;
  void beginRun(art::Run & r) override;
  void produce(art::Event & e) override;


private:

  // Declare member data here.
  std::vector<std::string> _hit_tags;

  int _nchannels;

  int _seed;
  TRandom3* _ran;

  // Helper class to generate PE charge
  std::string _mode;
  PeChargeGenerator _pcg;

  // Helper class to generate simulated signal waveform (not including baseline)
  SignalGenerator _sg;
};


darkart::MC::PeChargeMaker::PeChargeMaker(fhicl::ParameterSet const & p)
  : _hit_tags(p.get<std::vector<std::string> >("hitTags"))
  , _nchannels(p.get<int>("nchannels", 38))
  , _seed(p.get<int>("seed"))
  , _ran(nullptr)
  , _mode(p.get<std::string>("mode"))
  , _pcg(p.get<fhicl::ParameterSet>("chargeParams"), _mode)
  , _sg(p.get<fhicl::ParameterSet>("signalParams"))
{
  produces<std::vector<PeChargeVec> >();
  produces<darkart::WaveformVec>();
  produces<double, art::InRun>("SPEmean");
  produces<double, art::InRun>("SPEsigma");
}

void darkart::MC::PeChargeMaker::beginJob()
{
  _ran = new TRandom3(_seed);
  gRandom->SetSeed(_ran->Integer(UINT_MAX));
}

void darkart::MC::PeChargeMaker::beginRun(art::Run & r)
{
  // Store the SPE mean and sigma from the charge distribution.
  std::unique_ptr<double> spe_mean(new double(_pcg.mean()));
  std::unique_ptr<double> spe_sigma(new double(_pcg.sigma()));

  r.put(std::move(spe_mean), "SPEmean");
  r.put(std::move(spe_sigma), "SPEsigma");
}

void darkart::MC::PeChargeMaker::produce(art::Event & e)
{

  std::unique_ptr<std::vector<PeChargeVec> > charges(new std::vector<PeChargeVec>);
  // resize the vector for number of channels
  charges->resize(_nchannels);

  for (auto const& tag : _hit_tags) {
    auto const& peVec = e.getValidHandle<PeHitVec>(tag);
    // Sort hits by channel and fill charges
    for (darkart::MC::PeHit hit : *peVec)
      {
        darkart::MC::PeCharge myCharge;
        myCharge.time = hit.time;
        myCharge.charge = _pcg.fire(_ran);
        charges->at(hit.pmt).push_back(std::move(myCharge));
      }

  }

  std::unique_ptr<darkart::WaveformVec> sim_wfms(new darkart::WaveformVec);
  sim_wfms->resize(_nchannels);
  // Generate signal waveform for each channel.
  for (int ch=0; ch<_nchannels; ++ch) {
    darkart::MC::PeChargeVec & qVec = charges->at(ch);
    std::sort(qVec.begin(), qVec.end(), [](PeCharge a, PeCharge b) {return a.time < b.time; });
    sim_wfms->at(ch) = std::move(_sg.generate(qVec));
  }

  
  e.put(std::move(charges));
  e.put(std::move(sim_wfms));
  
}

DEFINE_ART_MODULE(darkart::MC::PeChargeMaker)