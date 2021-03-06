////////////////////////////////////////////////////////////////////////
// Class:       ODViewer
// Module Type: producer
// File:        ODViewer_module.cc
//
// Generated at Thu Feb 19 15:46:37 2015 by Alden Fan using artmod
// from cetpkgsupport v1_07_00.
////////////////////////////////////////////////////////////////////////

// framework includes
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"

// c++ includes
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

// root includes
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

// darkart OD includes
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

// Use the same graphics handling as the TPC
#include "darkart/TpcReco/RootGraphics.hh"

namespace darkart {
  namespace od {
    class ODViewer;
  }
}

/////////////////////////////////////////////////////////////////////////
// This is the OD waveform viewer. It primarily shows sum waveforms    //
// but is also able to show individual channels in various specifiable //
// formats (to be implemented).                                        //
/////////////////////////////////////////////////////////////////////////


class darkart::od::ODViewer : public art::EDProducer {
public:
  explicit ODViewer(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  ODViewer(ODViewer const &) = delete;
  ODViewer(ODViewer &&) = delete;
  ODViewer & operator = (ODViewer const &) = delete;
  ODViewer & operator = (ODViewer &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;


private:

  std::string _event_info_tag;
  std::string _sum_wf_tag;
  std::string _cluster_finder_tag;
  std::string _roi_integrator_tag;
  std::string _input_pulses;

  std::vector<int> _skip_channels;
  int _channel_type; //0: default, 1: LSV, 2: WT, -1: disabled LSV, -2: disabled WT
  
  bool _print_clusters;
  bool _print_rois;
  bool _draw_cluster_boxes;
  bool _draw_roi_boxes;

  fhicl::ParameterSet _amplitudesumparams;
  fhicl::ParameterSet _discriminatorsumparams;
  AmplitudeWaveformGenerator *_amplitude_waveform_generator;
  DiscriminatorWaveformGenerator *_discriminator_waveform_generator;

  darkart::RootGraphics gr;//graphics handler. Starts a graphics loop when initialized.
  TCanvas can;
  
  art::ServiceHandle<darkart::od::Utilities> _utils;
};


darkart::od::ODViewer::ODViewer(fhicl::ParameterSet const & p)
  : _event_info_tag(p.get<std::string>("event_info_tag"))
  , _sum_wf_tag(p.get<std::string>("sum_wf_tag"))
  , _cluster_finder_tag(p.get<std::string>("cluster_finder_tag"))
  , _roi_integrator_tag(p.get<std::string>("roi_integrator_tag"))
  , _input_pulses(p.get<std::string>("input_pulses", "PulseCorrector"))
  , _skip_channels(p.get<std::vector<int> >("skip_channels", std::vector<int>()))
  , _channel_type(p.get<int>("channel_type", 0))
  , _print_clusters(p.get<bool>("print_clusters",true))
  , _print_rois(p.get<bool>("print_rois",true))
  , _draw_cluster_boxes(p.get<bool>("draw_cluster_boxes",true))
  , _draw_roi_boxes(p.get<bool>("draw_roi_boxes",true))
  , _amplitudesumparams(p.get<fhicl::ParameterSet>("amplitudesumparams")) // Default?
  , _discriminatorsumparams(p.get<fhicl::ParameterSet>("discriminatorsumparams")) //Default?
  , gr()
  , can("viewcanvas","",0,0,1800,900)
{
  //disables closing the window until the module ends.
  ((TRootCanvas*)can.GetCanvasImp ())->DontCallClose();

  //Begins actually processing graphics information. Before this point, graphics commands are saved for later, not processed.
  gr.Start();
}

void darkart::od::ODViewer::produce(art::Event & e)
{
  {//open scope for lock guard
    LockGuard lock(gr.mutex);//until this gets deleted, don't let the graphics thread update, to avoid memory conflicts

    // retrieve data products from art::Event
    // art::Handle is one of art's smart pointers. You can dereference it with -> or * like a regular pointer. 
    art::Handle<darkart::od::ODEventInfo> event_info;   e.getByLabel(_event_info_tag, event_info);
    art::Handle<darkart::od::SumWF>     sum_wf;       e.getByLabel(_sum_wf_tag, sum_wf);
    //  art::Handle<darkart::od::ClusterVec> cluster_vec; e.getByLabel(_cluster_finder_tag, cluster_vec);   

    // check that we have the expected event
    if (static_cast<uint>(event_info->event_id) != e.event())
      throw cet::exception("ODViewer") << "art event ID does not match OD event ID. Aborting."<<std::endl;

    std::cout << "Processing event "<<event_info->event_id<<std::endl;

    // Format the canvas

    can.cd();
    can.Clear();
    std::ostringstream canvas_title;
    canvas_title<<"Run "<<event_info->run_id<<" Event "<<event_info->event_id;
    can.SetTitle(canvas_title.str().c_str());

    // hard coding the canvas division for now. I leave it to you to make this configurable.
    TPad* pad_sum_ampl = new TPad("pad_sum_ampl","Sum Amplitude TPad",0,0.75,0.8,1,-1,-1,-2);
    TPad* pad_sum_disc = new TPad("pad_sum_disc","Sum Discriminator TPad",0,0.5,0.8,0.75,-1,-1,-2);
    TPad* pad_chan_ampl = new TPad("pad_chan_ampl","Channel Amplitude TPad",0,0.25,0.8,0.5,-1,-1,-2);
    TPad* pad_chan_disc = new TPad("pad_chan_disc","Channel Discriminator TPad",0,0,0.8,0.25,-1,-1,-2);
    pad_sum_ampl->Draw(); pad_sum_disc->Draw(); pad_chan_ampl->Draw(); pad_chan_disc->Draw();

    // Make the legend
    TLegend* leg = new TLegend(0.8,0,1,1);
    leg->SetNColumns(5);
    leg->SetBorderSize(0);
    leg->Draw();

    if (!sum_wf.isValid()) {
      std::cout << "Sum wf is invalid. Skipping."<<std::endl;
      //TODO implement skip.
    }

    ///////////////////////////////////////////////////////////////////
    /////////////////     PLOT SUM WAVEFORM     ///////////////////////
    ///////////////////////////////////////////////////////////////////
    
    // I am generating the waveform here. Feel free to wrap this section up into a member function.
    
    const int nsamps = sum_wf->size();
    std::ostringstream name_oss; name_oss << "r" << event_info->run_id<<"e"<<event_info->event_id<<"sum";

    // Draw the sum amplitude waveform
    double sum_ampl_x[nsamps];
    double sum_ampl_y[nsamps];
    for (int samp=0; samp<nsamps; ++samp) {
      sum_ampl_x[samp] = sum_wf->at(samp).sample_ns;
      sum_ampl_y[samp] = sum_wf->at(samp).amplitude;
    }
    TGraph* sum_ampl_wf_gr = new TGraph(nsamps, sum_ampl_x, sum_ampl_y);
    sum_ampl_wf_gr->SetName(name_oss.str().c_str());
    sum_ampl_wf_gr->SetTitle(name_oss.str().c_str());
    sum_ampl_wf_gr->GetXaxis()->SetTitle("ns");
    sum_ampl_wf_gr->GetYaxis()->SetTitle("amplitude");
    sum_ampl_wf_gr->SetLineColor(kBlue);
    pad_sum_ampl->cd();
    sum_ampl_wf_gr->Draw("al");



    // Draw the sum discriminator waveform
    double sum_disc_x[nsamps];
    double sum_disc_y[nsamps];
    for (int samp=0; samp<nsamps; ++samp) {
      sum_disc_x[samp] = sum_wf->at(samp).sample_ns;
      sum_disc_y[samp] = sum_wf->at(samp).amplitude_discr;
    }
    TGraph* sum_disc_wf_gr = new TGraph(nsamps, sum_disc_x, sum_disc_y);
    sum_disc_wf_gr->SetName(name_oss.str().c_str());
    sum_disc_wf_gr->SetTitle(name_oss.str().c_str());
    sum_disc_wf_gr->GetXaxis()->SetTitle("ns");
    sum_disc_wf_gr->GetYaxis()->SetTitle("discriminator");
    sum_disc_wf_gr->SetLineColor(kBlue);
    pad_sum_disc->cd();
    sum_disc_wf_gr->Draw("al");
    
    ///////////////////////////////////////////////////////////////////
    /////////////////     GET CHANNEL WAVEFORM     ////////////////////
    ///////////////////////////////////////////////////////////////////
    
    
    _amplitude_waveform_generator = new AmplitudeWaveformGenerator
      (_amplitudesumparams,_channel_type);
    _discriminator_waveform_generator = new DiscriminatorWaveformGenerator
      (_discriminatorsumparams,_channel_type);
    if(_discriminator_waveform_generator)
      _discriminator_waveform_generator->setUtils(_utils->getPointer());

    // Get maximum number of channels
    art::Handle<darkart::od::ChannelDataVec> channel_vec_handle;
    e.getByLabel(_input_pulses, channel_vec_handle);
    darkart::od::ChannelDataVec const& channel_vec(*channel_vec_handle);
    const int max_channels = channel_vec.size();

    auto const &channel_data_vec = e.getValidHandle<darkart::od::ChannelDataVec>(_input_pulses);
    auto const &channel_wfs_vec = e.getValidHandle<darkart::od::ChannelWFsVec>(_input_pulses);

    TMultiGraph* chan_ampl_wf_mg = new TMultiGraph;
    chan_ampl_wf_mg->SetName("chan_ampl_wf_mg");
    chan_ampl_wf_mg->SetTitle("Channel Amplitude;ns;amplitude");
    TMultiGraph* chan_disc_wf_mg = new TMultiGraph;
    chan_disc_wf_mg->SetName("chan_disc_wf_mg");
    chan_disc_wf_mg->SetTitle("Channel Discriminator;ns;discriminator");
    
    TGraph* chan_ampl_wf_gr[max_channels];
    TGraph* chan_disc_wf_gr[max_channels];
    TLine*  chan_leg_line[max_channels];

    // Define the colors we will be using
    int num_colors(119);
    TString color_arr[num_colors] =
      {"#991f1f", "#ff4d4d", "#e57373", "#994d4d", "#663d3d", "#ffcccc", "#ff3c1a", "#66180a", "#ff7d66", "#ffa899", "#b2766b", "#b3948f", "#cc4b14", "#994e2e", "#ff9466", "#664233", "#e5a58a", "#ff811a", "#99561f", "#663914", "#ffab66", "#e5b38a", "#7f634d", "#ffe3cc", "#ffa31a", "#99681f", "#664514", "#ffc266", "#b28f59", "#ffc61a", "#cca83d", "#7f6926", "#e5cf8a", "#7f734d", "#b3aa8f", "#fffacc", "#f4ff1a", "#93991f", "#dfe65c", "#54660a", "#5e663d", "#9acc3d", "#7e994d", "#c5e68a", "#99ff33", "#a1b38f", "#5c6652", "#deffcc", "#43992e", "#adff99", "#3f663d", "#80b37d", "#14cc27", "#0a6613", "#2ee677", "#99ffc2", "#2e9969", "#4d8069", "#b8e6d1", "#1affba", "#33ffe0", "#24b39d", "#0a6666", "#99ffff", "#7db3b3", "#4de4ff", "#2e8999", "#667c80", "#0a4a66", "#2488b3", "#4dc9ff", "#b3e8ff", "#1f6299", "#459de6", "#7d9ab3", "#1a75ff", "#1252b3", "#4585e6", "#1f3b66", "#b3d1ff", "#1a53ff", "#99b3ff", "#7d8ab3", "#474f66", "#1f2b99", "#6b72b3", "#2519ff", "#434080", "#1d0a66", "#c1b8e6", "#6229cc", "#7859b3", "#bd99ff", "#675980", "#570d80", "#b833ff", "#d280ff", "#a07db3", "#734080", "#920f99", "#ae59b3", "#ff1ae8", "#660a5d", "#ff99f5", "#e645bd", "#993d74", "#ffb3e0", "#b37d9d", "#990f4d", "#661439", "#e65c9a", "#663d50", "#806671", "#b21242", "#ff4d82", "#b35974", "#660a18", "#ff99a8", "#b37d85"};
    /*
      {
      "#000000", "#FFFF00", "#1CE6FF", "#FF34FF", "#FF4A46", "#008941", "#006FA6", "#A30059",
      "#FFDBE5", "#7A4900", "#0000A6", "#63FFAC", "#B79762", "#004D43", "#8FB0FF", "#997D87",
      "#5A0007", "#809693", "#FEFFE6", "#1B4400", "#4FC601", "#3B5DFF", "#4A3B53", "#FF2F80",
      "#61615A", "#BA0900", "#6B7900", "#00C2A0", "#FFAA92", "#FF90C9", "#B903AA", "#D16100",
      "#DDEFFF", "#000035", "#7B4F4B", "#A1C299", "#300018", "#0AA6D8", "#013349", "#00846F",
      "#372101", "#FFB500", "#C2FFED", "#A079BF", "#CC0744", "#C0B9B2", "#C2FF99", "#001E09",
      "#00489C", "#6F0062", "#0CBD66", "#EEC3FF", "#456D75", "#B77B68", "#7A87A1", "#788D66",
      "#885578", "#FAD09F", "#FF8A9A", "#D157A0", "#BEC459", "#456648", "#0086ED", "#886F4C",  
      "#34362D", "#B4A8BD", "#00A6AA", "#452C2C", "#636375", "#A3C8C9", "#FF913F", "#938A81",
      "#575329", "#00FECF", "#B05B6F", "#8CD0FF", "#3B9700", "#04F757", "#C8A1A1", "#1E6E00",
      "#7900D7", "#A77500", "#6367A9", "#A05837", "#6B002C", "#772600", "#D790FF", "#9B9700",
      "#549E79", "#FFF69F", "#201625", "#72418F", "#BC23FF", "#99ADC0", "#3A2465", "#922329",
      "#5B4534", "#FDE8DC", "#404E55", "#0089A3", "#CB7E98", "#A4E804", "#324E72", "#6A3A4C",
      "#83AB58", "#001C1E", "#D1F7CE", "#004B28", "#C8D0F6", "#A3A489", "#806C66", "#222800",
      "#BF5650", "#E83000", "#66796D", "#DA007C", "#FF1A59", "#8ADBB4", "#1E0200", "#5B4E51",
      "#C895C5", "#320033", "#FF6832", "#66E1D3", "#CFCDAC", "#D0AC94", "#7ED379", "#012C58"
      };
    */
    for(int ic=0; ic<max_channels; ic++) {

      darkart::od::ChannelData const& channel_data = channel_vec.at(ic);
      // Check the channel type (lsv = 1, wt = 2, disabled lsv = -1, disabled wt = -2)
      // and skip if it is not the desired type   
      if(channel_data.channel_type != _channel_type)
      	continue;
      //      if(!channel_data.is_enabled)
      //	continue;

      const int ch_id = channel_data.channel_id;
      // Skip channels that were excluded in the fcl file
      if (find(_skip_channels.begin(),_skip_channels.end(),ch_id)!=_skip_channels.end())
	continue;

      darkart::od::SumWF* chan_wf(new darkart::od::SumWF()); _utils->zeroSumWF(*chan_wf, _channel_type);    
      
      if(_amplitude_waveform_generator)
	_amplitude_waveform_generator->sum(*channel_data_vec, *channel_wfs_vec, *chan_wf, ic);
      if(_discriminator_waveform_generator)
	_discriminator_waveform_generator->sum(*channel_data_vec, *chan_wf, ic);

      ///////////////////////////////////////////////////////////////////
      /////////////////     DRAW CHANNEL WAVEFORM     ///////////////////
      ///////////////////////////////////////////////////////////////////
      
      const int nchansamps = chan_wf->size();
      const Color_t ch_color = TColor::GetColor(color_arr[ic % num_colors].Data());//my_palette[ic];
      // Draw the channel amplitude waveform
      double chan_ampl_x[nchansamps];
      double chan_ampl_y[nchansamps];
      for (int samp=0; samp<nchansamps; ++samp) {
	chan_ampl_x[samp] = chan_wf->at(samp).sample_ns;
	chan_ampl_y[samp] = chan_wf->at(samp).amplitude;
      }
      chan_ampl_wf_gr[ic] = new TGraph(nchansamps, chan_ampl_x, chan_ampl_y);
      chan_ampl_wf_gr[ic]->SetName(Form("chan_ampl_wf_gr[%d]",ic));
      chan_ampl_wf_gr[ic]->SetTitle(Form("Channel %d Amplitude",ic));
      chan_ampl_wf_gr[ic]->GetXaxis()->SetTitle("ns");
      chan_ampl_wf_gr[ic]->GetYaxis()->SetTitle("amplitude");
      chan_ampl_wf_gr[ic]->SetLineColor(ch_color);
      chan_ampl_wf_mg->Add(chan_ampl_wf_gr[ic]);
      // Draw the channel discriminator waveform
      double chan_disc_x[nchansamps];
      double chan_disc_y[nchansamps];
      for (int samp=0; samp<nchansamps; ++samp) {
	chan_disc_x[samp] = chan_wf->at(samp).sample_ns;
	chan_disc_y[samp] = chan_wf->at(samp).amplitude_discr;
      }
      chan_disc_wf_gr[ic] = new TGraph(nchansamps, chan_disc_x, chan_disc_y);
      chan_disc_wf_gr[ic]->SetName(Form("chan_disc_wf_gr[%d]",ic));
      chan_disc_wf_gr[ic]->SetTitle(Form("Channel %d Discriminator",ic));
      chan_disc_wf_gr[ic]->GetXaxis()->SetTitle("ns");
      chan_disc_wf_gr[ic]->GetYaxis()->SetTitle("discriminator");
      chan_disc_wf_gr[ic]->SetLineColor(ch_color);
      chan_disc_wf_mg->Add(chan_disc_wf_gr[ic]);

      // Add channel to legend
      chan_leg_line[ic]=new TLine();
      chan_leg_line[ic]->SetLineColor(ch_color);
      chan_leg_line[ic]->SetLineWidth(8);
      leg->AddEntry(chan_leg_line[ic],Form("%d",ch_id),"l");
    }// End loop over channels
    pad_chan_ampl->cd();
    chan_ampl_wf_mg->Draw("al");
    pad_chan_disc->cd();
    chan_disc_wf_mg->Draw("al");

    //////////////////////////////////////////////////////////////////
    /////////                   Clusters                     /////////
    //////////////////////////////////////////////////////////////////
    
    // Get Clusters
    art::Handle<darkart::od::ClusterVec> cluster_vec_handle;
    e.getByLabel(_cluster_finder_tag, cluster_vec_handle);
    darkart::od::ClusterVec const& cluster_vec(*cluster_vec_handle);

    const int max_clusters = cluster_vec.size(); 
    for(int cl=0; cl<max_clusters; cl++) {
      darkart::od::Cluster const& cluster = cluster_vec.at(cl);
      // Print Cluster Info
      if (_print_clusters) {
	std::cout<<"Cluster: " <<cl
		 <<"\tStart: " <<cluster.start_ns
		 <<"\tEnd: "   <<cluster.end_ns
		 <<"\tCharge: "<<cluster.charge
		 <<"\tMax Multiplicity: "<<cluster.charge
		 <<"\tHeight: "<<cluster.height
 		 <<std::endl;
      }
      // Draw boxes
      if (_draw_cluster_boxes) {
	// Sum amplitude boxes
	pad_sum_ampl->cd();
	TBox* box_cluster_sum_ampl = new TBox(cluster.start_ns, 0, cluster.end_ns, cluster.height);
	box_cluster_sum_ampl->SetBit(TObject::kCanDelete,true);
	box_cluster_sum_ampl->SetLineColor(kGreen);
	box_cluster_sum_ampl->SetFillStyle(0);
	box_cluster_sum_ampl->Draw();
	
	// Sum discriminator boxes
	pad_sum_disc->cd();
	TBox* box_cluster_sum_disc = new TBox(cluster.start_ns, 0, cluster.end_ns, cluster.max_multiplicity);
	box_cluster_sum_disc->SetBit(TObject::kCanDelete,true);
	box_cluster_sum_disc->SetLineColor(kGreen);
	box_cluster_sum_disc->SetFillStyle(0);
	box_cluster_sum_disc->Draw();

	// Chan amplitude boxes
	pad_chan_ampl->cd();
	TBox* box_cluster_chan_ampl = new TBox(cluster.start_ns, 0, cluster.end_ns, chan_ampl_wf_mg->GetHistogram()->GetMaximum());
	box_cluster_chan_ampl->SetBit(TObject::kCanDelete,true);
	box_cluster_chan_ampl->SetLineColor(kGreen);
	box_cluster_chan_ampl->SetFillStyle(0);
	box_cluster_chan_ampl->Draw();
	
	// Sum discriminator boxes
	pad_chan_disc->cd();
	TBox* box_cluster_chan_disc = new TBox(cluster.start_ns, 0, cluster.end_ns, chan_disc_wf_mg->GetHistogram()->GetMaximum());
	box_cluster_chan_disc->SetBit(TObject::kCanDelete,true);
	box_cluster_chan_disc->SetLineColor(kGreen);
	box_cluster_chan_disc->SetFillStyle(0);
	box_cluster_chan_disc->Draw();
      }
    }
    
    //////////////////////////////////////////////////////////////////
    /////////                       Rois                     /////////
    //////////////////////////////////////////////////////////////////

    // Get Rois
    art::Handle<darkart::od::ROI> roi_handle;
    e.getByLabel(_roi_integrator_tag, roi_handle);
    darkart::od::ROI const& roi(*roi_handle);

    art::Handle<darkart::od::ROICfg> roicfg_handle;
    e.getByLabel(_roi_integrator_tag, roicfg_handle);
    darkart::od::ROICfg const& roicfg(*roicfg_handle);


    const int num_rois = roicfg.id_vec.size();
    for(int ir=0; ir<num_rois; ir++) {
      // Print Roi Info
      double roi_start_ns = _utils->lsvPromptNsToTriggerNs(roicfg.start_ns_vec.at(ir));
      double roi_end_ns = _utils->lsvPromptNsToTriggerNs(roicfg.end_ns_vec.at(ir));
      if (_print_rois) {
	std::cout<<"Roi ID: " <<roicfg.id_vec.at(ir)
		 <<"\tStart: " <<roi_start_ns
		 <<"\tEnd: "   <<roi_end_ns
		 <<"\tCharge: "<<roi.charge_vec.at(ir)
		 <<"\tMax Multiplicity: "<<roi.max_multiplicity_vec.at(ir)
		 <<std::endl;
      }
      // Draw boxes
      if (_draw_roi_boxes) {
	pad_sum_ampl->cd();
	TBox* box_roi_sum_ampl = new TBox(roi_start_ns, 0, roi_end_ns, 
					  sum_ampl_wf_gr->GetHistogram()->GetMaximum());
	box_roi_sum_ampl->SetBit(TObject::kCanDelete,true);
	box_roi_sum_ampl->SetLineColor(kBlack);
	box_roi_sum_ampl->SetFillStyle(0);
	box_roi_sum_ampl->Draw();
	
	pad_sum_disc->cd();
	TBox* box_roi_sum_disc = new TBox(roi_start_ns, 0, roi_end_ns, 
					  sum_disc_wf_gr->GetHistogram()->GetMaximum());
	box_roi_sum_disc->SetBit(TObject::kCanDelete,true);
	box_roi_sum_disc->SetLineColor(kBlack);
	box_roi_sum_disc->SetFillStyle(0);
	box_roi_sum_disc->Draw();
	
	pad_chan_ampl->cd();
	TBox* box_roi_chan_ampl = new TBox(roi_start_ns, 0, roi_end_ns, 
					  chan_ampl_wf_mg->GetHistogram()->GetMaximum());
	box_roi_chan_ampl->SetBit(TObject::kCanDelete,true);
	box_roi_chan_ampl->SetLineColor(kBlack);
	box_roi_chan_ampl->SetFillStyle(0);
	box_roi_chan_ampl->Draw();
	
	pad_chan_disc->cd();
	TBox* box_roi_chan_disc = new TBox(roi_start_ns, 0, roi_end_ns, 
					  chan_disc_wf_mg->GetHistogram()->GetMaximum());
	box_roi_chan_disc->SetBit(TObject::kCanDelete,true);
	box_roi_chan_disc->SetLineColor(kBlack);
	box_roi_chan_disc->SetFillStyle(0);
	box_roi_chan_disc->Draw();
      }

    }

    //////////////////////////////////////////////////////////////////
    /////////     Print a few useful bits of information     /////////
    //////////////////////////////////////////////////////////////////

    std::cout << "--------------------------------------" << std::endl;
    std::cout << " Drawing run "<<event_info->run_id <<" event " << event_info->event_id<<". Please wait." << std::endl;

    
    // Final update of canvas
    can.cd(0);
    can.Update();    
  }//lock guard is deleted

  //////////////////////////////////////////////////////////////////
  //////////////     Decide what to do next       //////////////////
  //////////////////////////////////////////////////////////////////
  
  while (true) {
    std::cout << "\nOptions: " << std::endl
              << "  <ENTER> to finish viewing this event"<< std::endl;
    std::cout << "  q to quit" << std::endl;    
    std::string input;
    getline(std::cin, input);

    if (input=="")
      return;
    else if (input == 'q')
      throw cet::exception("QuitViewer") << "This message is expected because of the clumsy way "
                                         << "of quitting art that I have implemented right now."
                                         << std::endl;
    else
      std::cout << "Unrecognized input. Try again." << std::endl;
    
    return;
  }
}

DEFINE_ART_MODULE(darkart::od::ODViewer)
