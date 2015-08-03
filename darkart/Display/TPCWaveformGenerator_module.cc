////////////////////////////////////////////////////////////////////////
// Class:       EventDisplay
// Module Type: producer
// File:        TPCWaveformGenerator_module.cc
//
// Created in May 2015 by Chris Stanford
// 
////////////////////////////////////////////////////////////////////////

/*
This module generates waveforms and other data structures
(pulses, spes) and saves them to a root file. The root file
can be read by a standalone GUI program that can plot the 
waveforms and color PMTs by integral in a 3d display
*/

#ifndef TPCWaveformGenerator_module_cc
#define TPCWaveformGenerator_module_cc

// Framework includes
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Optional/TFileService.h" 
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"

// Darkart TPC includes
#include "darkart/Products/EventInfo.hh"
#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/Baseline.hh"
#include "darkart/Products/Pulse.hh"
#include "darkart/Products/PulseParam.hh"
#include "darkart/Products/Spe.hh"
#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/ManyByChannel.hh"
#include "darkart/TpcReco/ByPulse.hh"
#include "darkart/TpcReco/utilities.hh"

// Root includes
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TRootCanvas.h"
#include "TPad.h"
#include "TObject.h"
#include "TAxis.h"
#include "TGaxis.h"
#include "TBox.h"
#include "TLine.h"
#include "TList.h"
#include "TStyle.h"
#include "TTree.h"
#include "TMath.h"

// C++ includes
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

namespace darkart {
  namespace display {
  class TPCWaveformGenerator;
  }
}


class darkart::display::TPCWaveformGenerator : public art::EDProducer {
public:
  explicit TPCWaveformGenerator(fhicl::ParameterSet const & p);
  virtual ~TPCWaveformGenerator();

  void produce(art::Event & e) override;
  void beginJob() override;
  void endJob() override;

private:
  //  void FillTPCWaveforms(art::Event & e);
  void FillData(art::Event & e);

  //               fhicl analysis parameters
  std::vector<int> skip_channels_;
  art::InputTag    event_info_tag_;
  art::InputTag    ch_tag_;
  art::InputTag    raw_wfm_tag_;
  art::InputTag    bs_wfm_tag_;
  art::InputTag    baseline_tag_;
  art::InputTag    int_wfm_tag_;
  art::InputTag    sumch_tag_;
  art::InputTag    sumch_raw_wfm_tag_;
  art::InputTag    sumch_bs_wfm_tag_;
  art::InputTag    sumch_baseline_tag_;
  art::InputTag    sumch_int_wfm_tag_;
  art::InputTag    pulse_tag_;
  art::InputTag    pulseparam_tag_;
  art::InputTag    sumch_pulse_tag_;
  art::InputTag    sumch_pulseparam_tag_;
  art::InputTag    event_pulse_tag_;
  art::InputTag    spe_tag_;

  //                fhicl display parameters
  bool             _tpc_enabled;
  bool             _tpc_geo_enabled;
  bool             _draw_baselinesubtracted;
  //  bool             _draw_interpolations;

  // Pulse Tree
  TTree*           tpc_pulse_tree;
  int              pulse_channel;
  double           pulse_start_us, pulse_end_us, pulse_peak_us, pulse_base, pulse_height;

  // SPE Tree
  TTree*           tpc_spe_tree;
  int              spe_channel;
  double           spe_start_us, spe_end_us, spe_base, spe_height;

  // Objects
  int              tpc_run_id;
  int              tpc_event_id;
  TTree*           tpc_display_tree;
  TTree*           tpc_settings_tree;
  TMultiGraph*     tpc_sum;
  TMultiGraph*     tpc_chan;
  TMultiGraph*     tpc_bl_sum;
  TMultiGraph*     tpc_bl_chan;

  art::ServiceHandle<art::TFileService> tfs;
};


darkart::display::TPCWaveformGenerator::TPCWaveformGenerator(fhicl::ParameterSet const & p) :
  skip_channels_(p.get<std::vector<int> >("skip_channels", std::vector<int>())),
  event_info_tag_(p.get<std::string>("event_info_tag")),
  ch_tag_(p.get<std::string>("ch_tag")),
  raw_wfm_tag_(p.get<std::string>("raw_wfm_tag")),
  bs_wfm_tag_(p.get<std::string>("bs_wfm_tag")),
  baseline_tag_(p.get<std::string>("baseline_tag")),
  //  int_wfm_tag_(p.get<std::string>("int_wfm_tag")),
  sumch_tag_(p.get<std::string>("sumch_tag")),
  sumch_raw_wfm_tag_(p.get<std::string>("sumch_raw_wfm_tag")),
  sumch_bs_wfm_tag_(p.get<std::string>("sumch_bs_wfm_tag")),
  sumch_baseline_tag_(p.get<std::string>("sumch_baseline_tag")),
  //  sumch_int_wfm_tag_(p.get<std::string>("sumch_int_wfm_tag")),
  pulse_tag_(p.get<std::string>("pulse_tag")),
  pulseparam_tag_(p.get<std::string>("pulseparam_tag")),
  sumch_pulse_tag_(p.get<std::string>("sumch_pulse_tag")),
  sumch_pulseparam_tag_(p.get<std::string>("sumch_pulseparam_tag")),
  event_pulse_tag_(p.get<std::string>("event_pulse_tag")),
  spe_tag_(p.get<std::string>("spe_tag")),

  _tpc_enabled(p.get<bool>("tpc_enabled",true)),
  _tpc_geo_enabled(p.get<bool>("tpc_geo_enabled",true)),
  _draw_baselinesubtracted(p.get<bool>("draw_baseline_subtracted")),
  //  _draw_baseline(p.get<bool>("draw_baseline")),
  //  _draw_interpolations(p.get<bool>("draw_interpolations")),
  tfs() {}

darkart::display::TPCWaveformGenerator::~TPCWaveformGenerator()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::display::TPCWaveformGenerator::FillData(art::Event & e) {
  auto const& evtInfo = e.getValidHandle<darkart::EventInfo>(event_info_tag_);

  // Double check event id
  if (static_cast<uint>(evtInfo->event_id) != e.event()) {
    std::cout << "WARNING: art::Event::event() does not match darkart::EventInfo::event_id."<<std::endl
	      << "You may not actually be accessing event "<<evtInfo->event_id<<std::endl;
  }

  tpc_run_id = evtInfo->run_id;
  tpc_event_id = evtInfo->event_id;  
  std::cout << "Processing TPC run "<<tpc_run_id<<" event "<<tpc_event_id<<std::endl;
    
  ///////////////////////////////////////////////////////////////////
  ///////////////            CHANNELS            ////////////////////
  ///////////////////////////////////////////////////////////////////
  {
    // The various waveforms to draw will be added to this TMultiGraph
    tpc_chan = new TMultiGraph();
    tpc_chan->SetName("mg_tpc_chan");
    tpc_chan->SetTitle(Form("Run %d Event %d TPC All Channels;us;Amplitude",tpc_run_id, tpc_event_id));

    art::Handle<darkart::ChannelVec> chVecHandle;
    e.getByLabel(ch_tag_, chVecHandle);

    darkart::ByChannel<darkart::Waveform>     get_raw           (chVecHandle, e, raw_wfm_tag_);
    darkart::ByChannel<darkart::Waveform>     get_bs_wfm        (chVecHandle, e, bs_wfm_tag_);
    //    darkart::ByChannel<darkart::Waveform>     get_integral      (chVecHandle, e, int_wfm_tag_);
    darkart::ByChannel<darkart::Baseline>     get_bs_info       (chVecHandle, e, baseline_tag_);
    //    darkart::ByChannel<darkart::WaveformInfo> get_integral_info (chVecHandle, e, int_wfm_tag_);
    darkart::ManyByChannel<darkart::Spe>      get_spe_vec       (chVecHandle, e, spe_tag_);
    
    // Loop through channels
    for (size_t ch=0; ch<chVecHandle->size(); ch++) {
      if (util::skipChannel(ch, skip_channels_))
	continue;
        
      auto const& channel       = util::getByChannelID(chVecHandle, ch);
      auto const& raw           = *get_raw(ch);
      auto const& bs_wfm        = *get_bs_wfm(ch);
      auto const& bs_info       = *get_bs_info(ch);

      //
      // SPEs
      //
      // Make SPE tree
      tpc_spe_tree = tfs->make<TTree>("tpc_spe_tree","TPC SPE Tree");
      tpc_spe_tree->Branch("channel",&spe_channel);
      tpc_spe_tree->Branch("start_us",&spe_start_us);
      tpc_spe_tree->Branch("end_us",&spe_end_us);
      tpc_spe_tree->Branch("base",&spe_base);
      tpc_spe_tree->Branch("height",&spe_height);

      auto spePtrVec = get_spe_vec(ch);
      // sort spePtrVec because getter doesn't always preserve order
      std::sort(spePtrVec.begin(), spePtrVec.end());
      darkart::SpeVec speVec;
      // manually strip off the Ptrs; this will eventually be done by a utility function
      for (size_t i=0; i<spePtrVec.size(); i++)
	speVec.push_back(*spePtrVec[i]);

      for (size_t i=0; i<speVec.size(); i++) {
	darkart::Spe const& spe = speVec[i];
	spe_channel = ch;
	spe_start_us = spe.start_time;
	spe_end_us = spe.start_time + spe.length;
	spe_base = (_draw_baselinesubtracted ? 0 : bs_info.mean);
	spe_height = spe_base-spe.amplitude;
	tpc_spe_tree->Fill();
      }
    
      //
      // Waveforms
      //
      // Generate the x axis
      const int nsamps = raw.wave.size();
      std::vector<double> x(nsamps);
      for(int i = 0; i<nsamps; i++)
	x[i] = raw.SampleToTime(i);

      // Draw a raw or baseline subtracted waveform first to set the scale of the pad
      TGraph* gr_tpc_chan;
      TGraph* gr_tpc_bl_chan;
      if (!_draw_baselinesubtracted) {
	if (!raw.isValid())
	  return;
	gr_tpc_chan = new TGraph(nsamps, &x[0], &raw.wave[0]);
	gr_tpc_chan->SetName(Form("gr_tpc_chan_%zu",ch));
	gr_tpc_chan->SetTitle(Form("Run %d Event %d TPC Channel %zu;ns;Amplitude",tpc_run_id,tpc_event_id,ch));

	// Must include baseline since it was not subtracted
	if (!bs_wfm.isValid())
	  return;
	std::vector<double> baseline(nsamps);
	for(int i = 0; i<nsamps; i++)
	  baseline[i] = raw.wave[i] - bs_wfm.wave[i];
      
	gr_tpc_bl_chan = new TGraph(nsamps, &x[0], &baseline[0]);
	gr_tpc_bl_chan->SetName(Form("gr_tpc_bl_chan_%zu",ch));
	gr_tpc_bl_chan->SetTitle(Form("Run %d Event %d TPC Baseline Channel %zu;ns;Amplitude",tpc_run_id,tpc_event_id,ch));
	gr_tpc_bl_chan->SetLineColor(kRed);
	gr_tpc_bl_chan->SetMarkerColor(kRed);
	tpc_bl_chan->Add(gr_tpc_bl_chan);
	tpc_chan->Add(gr_tpc_chan);
      }
      else {
	if (!bs_wfm.isValid())
	  return;
	gr_tpc_chan = new TGraph(nsamps, &x[0], &bs_wfm.wave[0]);
	gr_tpc_chan->SetName(Form("gr_tpc_chan_%zu",ch));
	gr_tpc_chan->SetTitle(Form("Run %d Event %d TPC Channel %zu;ns;Amplitude",tpc_run_id,tpc_event_id,ch));
	tpc_chan->Add(gr_tpc_chan);
      }
      if (!gr_tpc_chan) std::cout<<"Failed to retrieve run "<<tpc_run_id<<" event "<<tpc_event_id<<" chan "<<ch<<std::endl;    
    }
  } // End channels

  ///////////////////////////////////////////////////////////////////
  /////////////////        SUM CHANNEL        ///////////////////////
  ///////////////////////////////////////////////////////////////////
  {
    art::Handle<darkart::ChannelVec> sumchVecHandle;
    e.getByLabel(sumch_tag_, sumchVecHandle);
    art::Handle<darkart::PulseVec> pulseVecHandle;
    e.getByLabel(sumch_pulse_tag_, pulseVecHandle);

    darkart::ByChannel<darkart::Waveform>     get_raw           (sumchVecHandle, e, sumch_raw_wfm_tag_);
    darkart::ByChannel<darkart::Waveform>     get_bs_wfm        (sumchVecHandle, e, sumch_bs_wfm_tag_);
    darkart::ByChannel<darkart::Baseline>     get_bs_info       (sumchVecHandle, e, sumch_baseline_tag_);
    darkart::ManyByChannel<darkart::Pulse>    get_pulse_vec     (sumchVecHandle, e, sumch_pulse_tag_);
    darkart::ByPulse<darkart::PulseParam>     get_pulse_param   (pulseVecHandle, e, sumch_pulseparam_tag_);
      
    const int ch = darkart::Channel::SUMCH_ID;
    auto const& channel       = util::getByChannelID(sumchVecHandle, ch);
    auto const& raw           = *get_raw(ch);
    auto const& bs_wfm        = *get_bs_wfm(ch);
    auto const& bs_info       = *get_bs_info(ch);

    //
    // Waveforms
    //

    // The sum waveform will be added to this TMultiGraph
    tpc_sum = new TMultiGraph();
    tpc_sum->SetName("mg_tpc_sum");
    tpc_sum->SetTitle(Form("Run %d Event %d TPC Sum Channels;us;Amplitude",tpc_run_id, tpc_event_id));

    // Generate the x axis
    const int nsamps = raw.wave.size();
    std::vector<double> x(nsamps);
    for(int i = 0; i<nsamps; i++)
      x[i] = raw.SampleToTime(i);

    TGraph* gr_tpc_sum;
    TGraph* gr_tpc_bl_sum;
    if (!_draw_baselinesubtracted) {
      if (!raw.isValid())
	return;
      gr_tpc_sum = new TGraph(nsamps, &x[0], &raw.wave[0]);
      gr_tpc_sum->SetName("gr_tpc_sum");
      gr_tpc_sum->SetTitle(Form("Run %d Event %d TPC Sum;ns;Amplitude",tpc_run_id,tpc_event_id));
      tpc_sum->Add(gr_tpc_sum);

      // Must include baseline since it was not subtracted
      if (!bs_wfm.isValid())
	return;
      std::vector<double> baseline(nsamps);
      for(int i = 0; i<nsamps; i++)
	baseline[i] = raw.wave[i] - bs_wfm.wave[i];
      
      gr_tpc_bl_sum = new TGraph(nsamps, &x[0], &baseline[0]);
      gr_tpc_bl_sum->SetLineColor(kRed);
      gr_tpc_bl_sum->SetMarkerColor(kRed);
      tpc_bl_sum->Add(gr_tpc_bl_sum);
      tpc_sum->Add(gr_tpc_bl_sum);      

    }
    else {
      if (!bs_wfm.isValid())
	return;
      gr_tpc_sum = new TGraph(nsamps, &x[0], &bs_wfm.wave[0]);
      gr_tpc_sum->SetName("gr_tpc_sum");
      gr_tpc_sum->SetTitle(Form("Run %d Event %d TPC Sum;us;Amplitude",tpc_run_id,tpc_event_id));
      tpc_sum->Add(gr_tpc_sum);
    }
    if (!gr_tpc_sum) std::cout<<"Failed to retrieve run "<<tpc_run_id<<" event "<<tpc_event_id<<" sum "<<std::endl;

    //
    // Pulses
    //
    // Make Pulse tree 
    tpc_pulse_tree = tfs->make<TTree>("tpc_pulse_tree","TPC Pulse Tree");
    tpc_pulse_tree->Branch("start_us",&pulse_start_us);
    tpc_pulse_tree->Branch("end_us",&pulse_end_us);
    tpc_pulse_tree->Branch("peak_us",&pulse_peak_us);
    tpc_pulse_tree->Branch("base",&pulse_base);
    tpc_pulse_tree->Branch("height",&pulse_height);
    
    auto pulsePtrVec = get_pulse_vec(ch);
    // sort pulsePtrVec because getter doesn't always preserve order. use here for conistency
    std::sort(pulsePtrVec.begin(), pulsePtrVec.end());
    darkart::PulseVec pulseVec;
    darkart::PulseParamVec pulseParamVec;
    for (size_t i=0; i<pulsePtrVec.size(); i++) {
      pulseVec.emplace_back(*pulsePtrVec[i]);
      pulseParamVec.emplace_back(*get_pulse_param(pulsePtrVec[i]->pulseID));
    }
    for (size_t i=0; i<pulseVec.size(); i++) {
      darkart::Pulse const& pulse = pulseVec[i];
      darkart::PulseParam const& pulse_param = pulseParamVec[i];
      pulse_base = (_draw_baselinesubtracted ? 0 : bs_info.mean);
      pulse_height = pulse_base - pulse_param.peak_amplitude;
      pulse_start_us = x[pulse.start_index];
      pulse_end_us = x[pulse.end_index];
      pulse_peak_us = x[pulse_param.peak_index];
      tpc_pulse_tree->Fill();
    }    


  } // End sum

}

//_______________________________________________________________________________________________
void darkart::display::TPCWaveformGenerator::produce(art::Event & e) { //called for each event 
  // Generate waveforms and other data structures 
  FillData(e);
  std::cout<<"Filling display output tree\n";
  tpc_display_tree->Fill();
  std::cout<<"Complete!\n"; 
}

void darkart::display::TPCWaveformGenerator::beginJob() {
  // Create output trees
  // Settings tree contains bookkeeping variables
  // Only contains 1 entry
  tpc_settings_tree = tfs->make<TTree>("tpc_settings_tree","TPC Event Display Settings");
  tpc_settings_tree->Branch("tpc_geo_enabled",&_tpc_geo_enabled);
  tpc_settings_tree->Branch("tpc_enabled",&_tpc_enabled);
  tpc_settings_tree->Fill();
  // Diplay tree contains waveforms and other data structures
  // Contains as many entries as events
  tpc_display_tree = tfs->make<TTree>("tpc_display_tree","TPC Event Display Data");
  tpc_display_tree->Branch("tpc_run_id",&tpc_run_id);
  tpc_display_tree->Branch("tpc_event_id",&tpc_event_id);
  tpc_display_tree->Branch("tpc_sum",&tpc_sum);
  tpc_display_tree->Branch("tpc_chan",&tpc_chan);
  tpc_display_tree->Branch("tpc_bl_sum",&tpc_bl_sum);
  tpc_display_tree->Branch("tpc_bl_chan",&tpc_bl_chan);
  tpc_display_tree->Branch("tpc_pulse_tree",&tpc_pulse_tree);
  tpc_display_tree->Branch("tpc_spe_tree",&tpc_spe_tree);

}
void darkart::display::TPCWaveformGenerator::endJob() { }



DEFINE_ART_MODULE(darkart::display::TPCWaveformGenerator)
#endif
