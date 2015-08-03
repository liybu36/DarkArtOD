////////////////////////////////////////////////////////////////////////
// Class:       EventDisplay
// Module Type: producer
// File:        ODWaveformGenerator_module.cc
//
// Created on Wed Mar 18 2015 by Chris Stanford
////////////////////////////////////////////////////////////////////////

/*
This module generates waveforms and other data structures
(pulses, spes) and saves them to a root file. The root file
can be read by a standalone GUI program that can plot the 
waveforms and color PMTs by integral in a 3d display
*/

#ifndef ODWaveformGenerator_module_cc
#define ODWaveformGenerator_module_cc

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

// C++ includes
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>

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
#include "TLegend.h"
#include "TColor.h"
#include "TString.h"
#include "TMath.h"
#include "TH1.h"
#include "TFile.h"
#include "TEnv.h"
#include "TFolder.h"
#include "TRootBrowser.h"
#include "TDirectory.h"
#include "TObjArray.h"
#include "TTree.h"

// Darkart OD includes
#include "darkart/ODProducts/RunInfo.hh"
#include "darkart/ODProducts/ODEventInfo.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"
#include "darkart/ODProducts/SumWF.hh"
#include "darkart/ODProducts/Cluster.hh"
#include "darkart/ODProducts/ROI.hh"
#include "darkart/ODProducts/ROICfg.hh"
#include "darkart/ODReco/Submodules/AmplitudeWaveformGenerator.hh"
#include "darkart/ODReco/Submodules/DiscriminatorWaveformGenerator.hh"
#include "darkart/ODReco/Tools/Utilities.hh"

namespace darkart {
  namespace display {
    class ODWaveformGenerator;
  }
}

class darkart::display::ODWaveformGenerator : public art::EDProducer {
public:
  explicit ODWaveformGenerator(fhicl::ParameterSet const & p);
  ODWaveformGenerator(ODWaveformGenerator const &)               = delete;
  ODWaveformGenerator(ODWaveformGenerator &&)                    = delete;
  ODWaveformGenerator & operator = (ODWaveformGenerator const &) = delete;
  ODWaveformGenerator & operator = (ODWaveformGenerator &&)      = delete;

  void          produce(art::Event & e) override;
  void          beginJob() override;
  void          endJob() override;

  void          FillData(art::Event & e);
private:
  void          FillODWaveforms(art::Event & e, const int ch_type);
  void          FillLSVClusters(art::Event & e);
  void          FillLSVROIs(art::Event & e);

  //                  fhicl display parameters
  bool                _lsv_enabled;
  bool                _wt_enabled;
  bool                _lsv_geo_enabled;
  bool                _wt_geo_enabled;

  //                  fhicl analysis parameters
  std::string         _event_info_tag;  
  std::string         _input_pulses;
  fhicl::ParameterSet _amplitudesumparams;
  fhicl::ParameterSet _discriminatorsumparams;
  std::string         _cluster_finder_tag;
  std::string         _roi_integrator_tag;

  int                 _single_event;

  //                  output objects
  TTree*              od_display_tree;
  TTree*              od_settings_tree;
  TTree*              lsv_cluster_tree;
  TTree*              lsv_roi_tree;

  int                 od_run_id;
  int                 od_event_id;
  TMultiGraph*        lsv_ampl_sum;
  TMultiGraph*        lsv_disc_sum;
  TMultiGraph*        wt_ampl_sum;
  TMultiGraph*        wt_disc_sum;
  
  TMultiGraph*        lsv_ampl_chan;
  TMultiGraph*        lsv_disc_chan;
  TMultiGraph*        wt_ampl_chan;
  TMultiGraph*        wt_disc_chan;


  art    ::ServiceHandle<art::TFileService> tfs;
  //  darkart::RootGraphics* gr; // graphics handler
  art    ::ServiceHandle<darkart::od::Utilities> utils;
};
//_______________________________________________________________________________________________
darkart::display::ODWaveformGenerator::ODWaveformGenerator(fhicl::ParameterSet const & p):
  // Constructor
  _lsv_enabled           (p.get<bool>("lsv_enabled",true)),
  _wt_enabled            (p.get<bool>("wt_enabled",true)),
  _lsv_geo_enabled       (p.get<bool>("lsv_geo_enabled",true)),
  _wt_geo_enabled        (p.get<bool>("wt_geo_enabled",true)),
  _event_info_tag        (p.get<std::string>("event_info_tag")),
  _input_pulses          (p.get<std::string>("input_pulses", "PulseCorrector")), 
  _amplitudesumparams    (p.get<fhicl::ParameterSet>("amplitudesumparams")), // Default?
  _discriminatorsumparams(p.get<fhicl::ParameterSet>("discriminatorsumparams")), //Default?
  _cluster_finder_tag    (p.get<std::string>("cluster_finder_tag")),
  _roi_integrator_tag    (p.get<std::string>("roi_integrator_tag")),
  _single_event          (p.get<int>("single_event",0)),
  tfs()
{
  // Settting environment variables (equivalent to putting these in .rootrc)
  //gEnv->SetValue("Gui.IconFont",  "-*-helvetica-medium-r-*-*-14-*-*-*-*-*-iso8859-1");
  //gEnv->SetValue("Gui.StatusFont","-*-helvetica-medium-r-*-*-14-*-*-*-*-*-iso8859-1");
  // Initializing graphics loop
  //  gr = new RootGraphics();  
}

//_______________________________________________________________________________________________
void darkart::display::ODWaveformGenerator::produce(art::Event & e)
{
  //std::cout<<_single_event<<" "<<(int)e.event()<<"\n";
  //  if ((int)e.event() != _single_event) return;

  //  gr->Start(); 
  //  {//open scope for lock guard
  //    LockGuard lock(gr->mutex); // lock graphics thread to avoid memory conflicts
    //    std::cout<<"Filling OD data\n";    
    FillData(e);

    std::cout<<"Filling display output tree\n";
    od_display_tree->Fill();

    std::cout<<"Complete!\n";
    //  }//lock guard is deleted

  //////////////////////////////////////////////////////////////////
  //////////////     Decide what to do next       //////////////////
  //////////////////////////////////////////////////////////////////
  /*  
  while (true) {
    std::cout << "\nOptions: " << std::endl
              << "  <ENTER> to finish viewing this event"<< std::endl;
    std::cout << "  q to quit" << std::endl;    
    std::string input;
    getline(std::cin, input);

    if (input=="") {
      return;
    }
    else if (input == 'q')
      throw cet::exception("QuitViewer") << "This message is expected because of the clumsy way "
                                         << "of quitting art that is implemented right now."
                                         << std::endl;
    else
      std::cout << "Unrecognized input. Try again." << std::endl;
    
    return;
  }
  */
}
//_______________________________________________________________________________________________
void darkart::display::ODWaveformGenerator::FillData(art::Event & e) {
  art::Handle<darkart::od::ODEventInfo> event_info; e.getByLabel(_event_info_tag, event_info);

  // Double check event id
  if (static_cast<uint>(event_info->event_id) != e.event()) {
    std::cout << "WARNING: art::Event::event() does not match darkart::EventInfo::event_id."<<std::endl
	      << "You may not actually be accessing event "<<event_info->event_id<<std::endl;
  }

  od_run_id = event_info->run_id;
  od_event_id = event_info->event_id;
  std::cout << "Processing OD run "<<od_run_id<<" event "<<od_event_id<<std::endl;

  //std::cout<<"a\n";
  if(_lsv_enabled) {
    // std::cout<<"b\n";
    FillODWaveforms(e,1/*ch_type*/);
    // std::cout<<"c\n";
    FillLSVClusters(e);
    //std::cout<<"d\n";
    FillLSVROIs(e);
    //std::cout<<"e\n";
  }
  if(_wt_enabled)  FillODWaveforms(e,2/*ch_type*/);
  //std::cout<<"f\n";
}
//_______________________________________________________________________________________________
/*void darkart::display::ODWaveformGenerator::FillTPCWaveforms(art::Event & e) {  
  //  auto const& evtInfo = e.getValidHandle<darkart::EventInfo>(event_info_tag_);
  }*/

//_______________________________________________________________________________________________
void darkart::display::ODWaveformGenerator::FillODWaveforms(art::Event & e, const int ch_type) {  
  art::Handle<darkart::od::ODEventInfo> event_info; e.getByLabel(_event_info_tag, event_info);
  int run_id = event_info->run_id;
  int event_id = event_info->event_id;

  std::string det;
  std::string udet;
  if (ch_type==1) {det = "lsv"; udet = "LSV";}
  if (ch_type==2) {det = "wt";  udet = "WT";}
  
  // Get data  
  AmplitudeWaveformGenerator* _amplitude_waveform_generator 
    = new AmplitudeWaveformGenerator(_amplitudesumparams,ch_type);
  DiscriminatorWaveformGenerator* _discriminator_waveform_generator 
    = new DiscriminatorWaveformGenerator(_discriminatorsumparams,ch_type);
  if(_discriminator_waveform_generator)
    _discriminator_waveform_generator->setUtils(utils->getPointer());

  art::Handle<darkart::od::ChannelDataVec> channel_vec_handle;
  e.getByLabel(_input_pulses, channel_vec_handle);
  darkart::od::ChannelDataVec const& channel_vec(*channel_vec_handle);
  
  auto const &channel_data_vec = e.getValidHandle<darkart::od::ChannelDataVec>(_input_pulses);
  auto const &channel_wfs_vec = e.getValidHandle<darkart::od::ChannelWFsVec>(_input_pulses);

  // Sum
  darkart::od::SumWF* sum_wf = new darkart::od::SumWF(); utils->zeroSumWF(*sum_wf, ch_type);
  if(_amplitude_waveform_generator)
    _amplitude_waveform_generator->sum(*channel_data_vec, *channel_wfs_vec, *sum_wf, -1);
  if(_discriminator_waveform_generator)
    _discriminator_waveform_generator->sum(*channel_data_vec, *sum_wf, -1);
  const int n_samps_sum = sum_wf->size();
  //  std::cout<<"1\n";

  // Make the sum multigraphs
  TMultiGraph* mg_ampl_sum = new TMultiGraph();
  mg_ampl_sum->SetName(Form("mg_%s_ampl_sum",det.c_str()));
  mg_ampl_sum->SetTitle(Form("Run %d Event %d %s Sum Channels;ns;Amplitude",run_id,event_id,udet.c_str()));
  TMultiGraph* mg_disc_sum = new TMultiGraph();
  mg_disc_sum->SetName(Form("mg_%s_disc_sum",det.c_str()));
  mg_disc_sum->SetTitle(Form("Run %d Event %d %s Sum Channels;ns;Discriminator",run_id,event_id,udet.c_str()));
  // std::cout<<"2\n";

  // Make the sum amplitude waveform
  double ampl_sum_x[n_samps_sum];
  double ampl_sum_y[n_samps_sum];
  for (int samp=0; samp<n_samps_sum; ++samp) {
    ampl_sum_x[samp] = sum_wf->at(samp).sample_ns;
    ampl_sum_y[samp] = sum_wf->at(samp).amplitude;
  }
  TGraph* gr_ampl_sum = new TGraph(n_samps_sum, ampl_sum_x, ampl_sum_y);
  gr_ampl_sum->SetName(Form("gr_%s_ampl_sum",det.c_str()));
  gr_ampl_sum->SetTitle(Form("Run %d Event %d %s Sum Channels;ns;Amplitude",run_id,event_id,udet.c_str()));
  mg_ampl_sum->Add(gr_ampl_sum);

  // Make the sum discriminator waveform
  double disc_sum_x[n_samps_sum];
  double disc_sum_y[n_samps_sum];
  for (int samp=0; samp<n_samps_sum; ++samp) {
    disc_sum_x[samp] = sum_wf->at(samp).sample_ns;
    disc_sum_y[samp] = sum_wf->at(samp).amplitude_discr;
  }
  TGraph* gr_disc_sum = new TGraph(n_samps_sum, disc_sum_x, disc_sum_y);
  gr_disc_sum->SetName(Form("gr_%s_disc_sum",det.c_str()));
  gr_disc_sum->SetTitle(Form("Run %d Event %d %s Sum Channels;ns;Discriminator",run_id,event_id,udet.c_str()));
  mg_disc_sum->Add(gr_disc_sum);
  // std::cout<<"3\n";


  // Make the channel multigraphs
  TMultiGraph* mg_ampl_chan = new TMultiGraph();
  mg_ampl_chan->SetName(Form("mg_%s_ampl_chan",det.c_str()));
  mg_ampl_chan->SetTitle(Form("Run %d Event %d %s All Channels;ns;Amplitude",run_id,event_id,udet.c_str()));
  TMultiGraph* mg_disc_chan = new TMultiGraph();
  mg_disc_chan->SetName(Form("mg_%s_disc_chan",det.c_str()));
  mg_disc_chan->SetTitle(Form("Run %d Event %d %s All Channels;ns;Discriminator",run_id,event_id,udet.c_str()));

  // Initialize the channel multigraphs
  //  TMultiGraph* mg_ampl_chan;
  //  TMultiGraph* mg_disc_chan;
  //std::cout<<"4\n";
  //  TMultiGraph* mymg = new TMultiGraph();
  // Make the channel trees
  /*
  TTree* ampl_chan_tree = new TTree(Form("%s_ampl_chan_tree",det.c_str()),Form("%s Amplitude Channel Data",udet.c_str()));
  TTree* disc_chan_tree = new TTree(Form("%s_disc_chan_tree",det.c_str()),Form("%s Discriminator Channel Data",udet.c_str()));
  std::cout<<"4a\n";  
  ampl_chan_tree->Branch("rid"Form("%s_ampl_chan",det.c_str()),&mg_ampl_chan);
  //  disc_chan_tree->Branch("lsv_disc_chan"Form("%s_disc_chan",det.c_str()),&mg_disc_chan);*/
  //std::cout<<"5\n";

  // Channels  
  // Loop over channels
  for(size_t i = 0; i < channel_vec.size(); i++) {
    darkart::od::ChannelData const& channel_data = channel_vec.at(i);
    const int ch_id = channel_data.channel_id;
    if(channel_data.channel_type != ch_type) continue;
    //std::cout<<"5a\n";

    //std::cout<<"5b\n";

    darkart::od::SumWF* chan_wf = new darkart::od::SumWF(); utils->zeroSumWF(*chan_wf, ch_type);
    if(_amplitude_waveform_generator)
      _amplitude_waveform_generator->sum(*channel_data_vec, *channel_wfs_vec, *chan_wf, ch_id);
    if(_discriminator_waveform_generator)
      _discriminator_waveform_generator->sum(*channel_data_vec, *chan_wf, ch_id);

    const int n_samps = chan_wf->size();

    // Make the channel amplitude waveform
    double ampl_chan_x[n_samps];
    double ampl_chan_y[n_samps];
    for (int samp=0; samp<n_samps; ++samp) {
      ampl_chan_x[samp] = chan_wf->at(samp).sample_ns;
      ampl_chan_y[samp] = chan_wf->at(samp).amplitude;
    }
    //std::cout<<"5c\n";

    TGraph* gr_ampl_chan = new TGraph(n_samps, ampl_chan_x, ampl_chan_y);
    gr_ampl_chan->SetName(Form("gr_%s_ampl_chan_%zu",det.c_str(),i));
    gr_ampl_chan->SetTitle(Form("Run %d Event %d %s Channel %zu;ns;Amplitude",run_id,event_id,udet.c_str(),i));
    mg_ampl_chan->Add(gr_ampl_chan);
    //std::cout<<"5d\n";

    // Make the channel discriminator waveform
    double disc_chan_x[n_samps];
    double disc_chan_y[n_samps];
    for (int samp=0; samp<n_samps; ++samp) {
      disc_chan_x[samp] = chan_wf->at(samp).sample_ns;
      disc_chan_y[samp] = chan_wf->at(samp).amplitude_discr;
    }
    TGraph* gr_disc_chan = new TGraph(n_samps, disc_chan_x, disc_chan_y);
    gr_disc_chan->SetName(Form("gr_%s_disc_chan_%zu",det.c_str(),i));
    gr_disc_chan->SetTitle(Form("Run %d Event %d %s Channel %zu;ns;Discriminator",run_id,event_id,udet.c_str(),i));
    mg_disc_chan->Add(gr_disc_chan);
    //  std::cout<<"5e\n";

    // Fill trees
  //    ampl_chan_tree->Fill();
  //    disc_chan_tree->Fill();
  //  std::cout<<"5f\n";

  }// End loop over channels
  
  // Set globals to be written
  if (ch_type == 1) {
    lsv_ampl_sum = mg_ampl_sum; 
    lsv_disc_sum = mg_disc_sum; 
    lsv_ampl_chan = mg_ampl_chan;
    lsv_disc_chan = mg_ampl_chan;
  }
  if (ch_type == 2) {
    wt_ampl_sum = mg_ampl_sum; 
    wt_disc_sum = mg_disc_sum; 
    wt_ampl_chan = mg_ampl_chan;
    wt_disc_chan = mg_disc_chan;
  }
}
//_______________________________________________________________________________________________
void darkart::display::ODWaveformGenerator::FillLSVClusters(art::Event & e) {
  // Get Clusters
  art::Handle<darkart::od::ClusterVec> cluster_vec_handle;
  e.getByLabel(_cluster_finder_tag, cluster_vec_handle);
  darkart::od::ClusterVec const& cluster_vec(*cluster_vec_handle);

  double start_ns;
  double end_ns;
  double charge;
  double height;
  int max_multiplicity;

  lsv_cluster_tree = new TTree("lsv_cluster_tree","Cluster Data");
  lsv_cluster_tree->Branch("start_ns",&start_ns);
  lsv_cluster_tree->Branch("end_ns",&end_ns);
  lsv_cluster_tree->Branch("charge",&charge);
  lsv_cluster_tree->Branch("height",&height);
  lsv_cluster_tree->Branch("max_multiplicity",&max_multiplicity);
  std::cout<<"Number of clusters: " <<cluster_vec.size()<<std::endl;
  for(size_t i=0; i<cluster_vec.size(); i++) {
    darkart::od::Cluster const& cluster = cluster_vec.at(i);
    start_ns         = cluster.start_ns;
    end_ns           = cluster.end_ns;
    charge           = cluster.charge;
    height           = cluster.height;
    max_multiplicity = cluster.max_multiplicity;
    lsv_cluster_tree->Fill();
  }
}
//_______________________________________________________________________________________________
void darkart::display::ODWaveformGenerator::FillLSVROIs(art::Event & e) {
  // Get Rois                                                                                                                                                                        
  art::Handle<darkart::od::ROI> roi_handle;
  e.getByLabel(_roi_integrator_tag, roi_handle);
  darkart::od::ROI const& roi(*roi_handle);
  art::Handle<darkart::od::ROICfg> roicfg_handle;
  e.getByLabel(_roi_integrator_tag, roicfg_handle);
  darkart::od::ROICfg const& roicfg(*roicfg_handle);

  double start_ns;
  double end_ns;
  double charge;
  int max_multiplicity;

  lsv_roi_tree = new TTree("lsv_roi_tree","ROI Data");
  lsv_roi_tree->Branch("start_ns",&start_ns);
  lsv_roi_tree->Branch("end_ns",&end_ns);
  lsv_roi_tree->Branch("charge",&charge);
  lsv_roi_tree->Branch("max_multiplicity",&max_multiplicity);
  std::cout<<"Number of ROIs: " <<roicfg.id_vec.size()<<std::endl;
  for(size_t i=0; i<roicfg.id_vec.size(); i++) {
    start_ns         = utils->lsvPromptNsToTriggerNs(roicfg.start_ns_vec.at(i));
    end_ns           = utils->lsvPromptNsToTriggerNs(roicfg.end_ns_vec.at(i));
    charge           = roi.charge_vec.at(i);
    max_multiplicity = roi.max_multiplicity_vec.at(i);
  }
  lsv_roi_tree->Fill();
}
  

//_______________________________________________________________________________________________
void darkart::display::ODWaveformGenerator::beginJob() {

  od_settings_tree = tfs->make<TTree>("od_settings_tree","OD Event Display Settings");
  od_settings_tree->Branch("lsv_geo_enabled",&_lsv_geo_enabled);
  od_settings_tree->Branch("wt_geo_enabled",&_wt_geo_enabled);
  od_settings_tree->Branch("lsv_enabled",&_lsv_enabled);
  od_settings_tree->Branch("wt_enabled",&_wt_enabled);
  od_settings_tree->Fill();

  od_display_tree = tfs->make<TTree>("od_display_tree","OD Event Display Data");
  od_display_tree->Branch("od_run_id",&od_run_id);
  od_display_tree->Branch("od_event_id",&od_event_id);
  od_display_tree->Branch("lsv_ampl_sum",&lsv_ampl_sum);
  od_display_tree->Branch("lsv_disc_sum",&lsv_disc_sum);
  od_display_tree->Branch("lsv_ampl_chan",&lsv_ampl_chan);
  od_display_tree->Branch("lsv_disc_chan",&lsv_disc_chan);
  od_display_tree->Branch("wt_ampl_sum",&wt_ampl_sum);
  od_display_tree->Branch("wt_disc_sum",&wt_disc_sum);
  od_display_tree->Branch("wt_ampl_chan",&wt_ampl_chan);
  od_display_tree->Branch("wt_disc_chan",&wt_disc_chan);
  od_display_tree->Branch("lsv_cluster_tree",&lsv_cluster_tree);
  od_display_tree->Branch("lsv_roi_tree",&lsv_roi_tree);

}
//_______________________________________________________________________________________________
void darkart::display::ODWaveformGenerator::endJob() {

}
DEFINE_ART_MODULE(darkart::display::ODWaveformGenerator)
#endif
