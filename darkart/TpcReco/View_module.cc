////////////////////////////////////////////////////////////////////////
// Class:       View
// Module Type: analyzer
// File:        View_module.cc
//
// Generated at Mon Oct  7 18:46:01 2013 by Jason Brodsky using artmod
// from cetpkgsupport v1_02_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"

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
#include "darkart/TpcReco/RootGraphics.hh"

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

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

namespace darkart {
  class View;
}



////////////////////////////////////////////////////////////////////////////////
// This is a hacked together event viewer. It should be replaced by a better, //
// more fully featured one for the long term.                                 //
////////////////////////////////////////////////////////////////////////////////



class darkart::View : public art::EDAnalyzer {
public:
  explicit View(fhicl::ParameterSet const & p);
  virtual ~View();

  void analyze(art::Event const & e) override;


private:
  std::vector<int> skip_channels_;

  art::InputTag event_info_tag_;
  art::InputTag ch_tag_;
  art::InputTag raw_wfm_tag_;
  art::InputTag bs_wfm_tag_;
  art::InputTag baseline_tag_;
  art::InputTag int_wfm_tag_;
  art::InputTag sumch_tag_;
  art::InputTag sumch_raw_wfm_tag_;
  art::InputTag sumch_bs_wfm_tag_;
  art::InputTag sumch_baseline_tag_;
  art::InputTag sumch_int_wfm_tag_;
  art::InputTag pulse_tag_;
  art::InputTag pulseparam_tag_;
  art::InputTag sumch_pulse_tag_;
  art::InputTag sumch_pulseparam_tag_;
  art::InputTag event_pulse_tag_;
  art::InputTag spe_tag_;

  bool draw_physical_;
  bool draw_sum_;
  bool draw_baseline_subtracted_;
  bool draw_integral_;
  bool draw_baseline_;
  bool draw_interpolations_;
  bool draw_pulses_;
  bool draw_single_pe_;


  // This function retrieves all the waveforms that we want for a given channel
  // and formats and plots them. We pass in the whole event because we need it to
  // retrieve the pulseParam objects
  void draw_processed(TMultiGraph* mg,
                      darkart::Channel const& channel,
                      darkart::Waveform const& raw,
                      darkart::Waveform const& bs_wfm,
                      darkart::Baseline const& bs_info,
                      darkart::Waveform const& integral,
                      darkart::WaveformInfo const& integral_info,
                      darkart::PulseVec const& pulseVec,
                      darkart::PulseParamVec const& pulseParamVec,
                      darkart::SpeVec const& speVec,
                      bool draw_baseline_subtracted,
                      bool draw_integral,
                      bool draw_baseline,
                      bool draw_interpolations,
                      bool draw_pulses,
                      bool draw_single_pe);


  darkart::RootGraphics gr;//graphics handler. Starts a graphics loop when initialized.
  TCanvas can;
  TCanvas sum_canvas;
};


darkart::View::View(fhicl::ParameterSet const & p) :
  EDAnalyzer(p),
  skip_channels_(p.get<std::vector<int> >("skip_channels", std::vector<int>())),
  event_info_tag_(p.get<std::string>("event_info_tag")),
  ch_tag_(p.get<std::string>("ch_tag")),
  raw_wfm_tag_(p.get<std::string>("raw_wfm_tag")),
  bs_wfm_tag_(p.get<std::string>("bs_wfm_tag")),
  baseline_tag_(p.get<std::string>("baseline_tag")),
  int_wfm_tag_(p.get<std::string>("int_wfm_tag")),
  sumch_tag_(p.get<std::string>("sumch_tag")),
  sumch_raw_wfm_tag_(p.get<std::string>("sumch_raw_wfm_tag")),
  sumch_bs_wfm_tag_(p.get<std::string>("sumch_bs_wfm_tag")),
  sumch_baseline_tag_(p.get<std::string>("sumch_baseline_tag")),
  sumch_int_wfm_tag_(p.get<std::string>("sumch_int_wfm_tag")),
  pulse_tag_(p.get<std::string>("pulse_tag")),
  pulseparam_tag_(p.get<std::string>("pulseparam_tag")),
  sumch_pulse_tag_(p.get<std::string>("sumch_pulse_tag")),
  sumch_pulseparam_tag_(p.get<std::string>("sumch_pulseparam_tag")),
  event_pulse_tag_(p.get<std::string>("event_pulse_tag")),
  spe_tag_(p.get<std::string>("spe_tag")),

  draw_physical_(p.get<bool>("draw_physical")),
  draw_sum_(p.get<bool>("draw_sum")),
  draw_baseline_subtracted_(p.get<bool>("draw_baseline_subtracted")),
  draw_integral_(p.get<bool>("draw_integral")),
  draw_baseline_(p.get<bool>("draw_baseline")),
  draw_interpolations_(p.get<bool>("draw_interpolations")),
  draw_pulses_(p.get<bool>("draw_pulses")),
  draw_single_pe_(p.get<bool>("draw_single_pe")),
  gr(),
  can("viewcanvas","",0,0,1800,900),
  sum_canvas("sum_channel_canvas", "", 50, 50, 1000, 500)
{
  ((TRootCanvas*)can.GetCanvasImp ())->DontCallClose();//disables closing the window until the module ends.
  ((TRootCanvas*)sum_canvas.GetCanvasImp ())->DontCallClose();
  gr.Start();//Begins actually processing graphics information. Before this point, graphics commands are saved for later, not processed.
}

darkart::View::~View()
{
  // Clean up dynamic memory and other resources here.
}


void darkart::View::draw_processed(TMultiGraph* mg,
                                   darkart::Channel const& channel,
                                   darkart::Waveform const& raw,
                                   darkart::Waveform const& bs_wfm,
                                   darkart::Baseline const& bs_info,
                                   darkart::Waveform const& integral,
                                   darkart::WaveformInfo const& integral_info,
                                   darkart::PulseVec const& pulseVec,
                                   darkart::PulseParamVec const& pulseParamVec,
                                   darkart::SpeVec const& speVec,
                                   bool draw_baseline_subtracted,
                                   bool draw_integral,
                                   bool draw_baseline,
                                   bool draw_interpolations,
                                   bool draw_pulses,
                                   bool draw_single_pe)
{


  
  // generate the x axis
  const int nsamps = raw.wave.size();
  std::vector<double> x(nsamps);
  for(int i = 0; i<nsamps; i++)
    x[i] = raw.SampleToTime(i);

  
  // Draw a raw or baseline subtracted waveform first to set the scale of the pad
  if (!draw_baseline_subtracted) {
    if (!raw.isValid())
      return;
    TGraph* raw_gr = new TGraph(nsamps, &x[0], &raw.wave[0]);
    raw_gr->SetLineColor(kBlack);
    raw_gr->SetMarkerColor(kBlack);
    mg->Add(raw_gr);
  }
  else {
    if (!bs_wfm.isValid())
      return;
    TGraph* bs_wfm_gr = new TGraph(nsamps, &x[0], &bs_wfm.wave[0]);
    bs_wfm_gr->SetLineColor(kBlack);
    bs_wfm_gr->SetMarkerColor(kBlack);
    mg->Add(bs_wfm_gr);
  }
  mg->Draw("ALP");


      
  // If told to draw baseline, need to compute it
  if (draw_baseline) {
    if (draw_baseline_subtracted)
      LOG_WARNING("View") << "Configured to draw baseline with baseline-subtracted waveform. Baseline will likely be way off scale. ";
    if (!raw.isValid() || !bs_wfm.isValid())
      return;
    std::vector<double> baseline(nsamps);
    for(int i = 0; i<nsamps; i++)
      baseline[i] = raw.wave[i] - bs_wfm.wave[i];

    TGraph* baseline_gr = new TGraph(nsamps, &x[0], &baseline[0]);
    baseline_gr->SetLineColor(kRed);
    baseline_gr->SetMarkerColor(kRed);
    mg->Add(baseline_gr);

    if (draw_interpolations && bs_info.interpolations.size()>0) {
      // draw baseline interpolation points
      std::vector<double> interpolations_time(bs_info.interpolations.size()*2);
      std::vector<double> interpolations_value(bs_info.interpolations.size()*2);
      for (size_t i=0; i<bs_info.interpolations.size(); i++) {
        double t1 = bs_info.interpolations[i].first;
        double t2 = bs_info.interpolations[i].second;
        interpolations_time[i*2] = t1;
        interpolations_time[i*2+1] = t2;
        interpolations_value[i*2] = baseline_gr->Eval(t1);
        interpolations_value[i*2+1] = baseline_gr->Eval(t2);
      }
      TGraph* interpolations_gr = new TGraph(bs_info.interpolations.size()*2,
                                             &interpolations_time[0],
                                             &interpolations_value[0]);
      interpolations_gr->SetMarkerStyle(8);
      interpolations_gr->SetMarkerSize(1);
      interpolations_gr->SetMarkerColor(kGreen);

      mg->Add(interpolations_gr,"P");
    }
  }


  // If told to draw integral, need to scale and offset it so it overlays nicely onto the existing waveform
  if (draw_integral) {
    if (!integral.isValid())
      return;
    std::vector<double> adjusted_integral(nsamps); 
    double integral_offset = (draw_baseline_subtracted ? 0 : bs_info.mean);
    double x1,x2,y1,y2;
    gPad->Update();
    gPad->GetRangeAxis(x1,y1,x2,y2);
    double raw_ratio = (y2 - integral_offset) / (integral_offset - y1);
    double integral_ratio = std::abs(integral_info.maximum) / std::abs(integral_info.minimum);
    double integral_scale;
    if (raw_ratio < integral_ratio)
      integral_scale = (y2 - integral_offset) / std::abs(integral_info.maximum) * 0.9;
    else
      integral_scale = (integral_offset - y1) / std::abs(integral_info.minimum) * 0.9;


    for(int i = 0; i<nsamps; i++)
      adjusted_integral[i] = integral_scale*integral.wave[i] + integral_offset;

    int integral_color = kBlue;
    
    TGraph* integral_gr = new TGraph(nsamps, &x[0], &adjusted_integral[0]);
    integral_gr->SetLineColor(integral_color);
    integral_gr->SetMarkerColor(integral_color);
    mg->Add(integral_gr);
      
    // draw a separate axis for the integral
    if (gPad) {
      double x1,x2,y1,y2;
      gPad->GetRangeAxis(x1,y1,x2,y2);
      gPad->SetBit(TObject::kCanDelete, true);
      TGaxis* gaxis = new TGaxis(x2,y1,x2,y2,
                                 (y1-integral_offset)/integral_scale,
                                 (y2-integral_offset)/integral_scale,
                                 510,"L+");
      gaxis->SetName("integral_axis");
      if(channel.channel_id() == darkart::Channel::SUMCH_ID)
        gaxis->SetTitle("Integral [photoelectrons]"); 
      else
        gaxis->SetTitle("Integral [counts*samples]");
      gaxis->SetLineColor(integral_color);
      gaxis->SetTextColor(integral_color);
      gaxis->SetTitleColor(integral_color);
      gaxis->SetLabelColor(integral_color);
      gaxis->SetTitleOffset(1.2);
      gaxis->Draw();
    }
  }

  

  // Done formatting all the waveforms, so draw them.
  mg->Draw("ALP");



  // Now overlay boxes and lines
  if (draw_pulses) {
    for (size_t i=0; i<pulseVec.size(); i++) {
      darkart::Pulse const& pulse = pulseVec[i];
      darkart::PulseParam const& pulse_param = pulseParamVec[i];
      double base = (draw_baseline_subtracted ? 0 : bs_info.mean);
      double peak_y = base - pulse_param.peak_amplitude;
      TBox* pbox = new TBox( x[pulse.start_index], base,
                             x[pulse.end_index], peak_y );

      pbox->SetBit(TObject::kCanDelete,true);
      pbox->SetLineColor(kGreen);
      pbox->SetFillStyle(0);
      pbox->Draw();
      TLine* pline = new TLine( x[pulse_param.peak_index], base,
                                 x[pulse_param.peak_index], peak_y);
      pline->SetBit(TObject::kCanDelete,true);
      pline->SetLineColor(kMagenta);
      pline->Draw();


    }//end loop over pulses
  }

  if (draw_single_pe) {
    for (size_t i=0; i<speVec.size(); i++) {
      darkart::Spe const& spe = speVec[i];
      TBox* spebox = new TBox(spe.start_time, bs_info.mean,
                              spe.start_time+spe.length,
                              bs_info.mean-spe.amplitude);
      spebox->SetBit(TObject::kCanDelete, true);
      spebox->SetLineColor(kBlue);
      spebox->SetFillStyle(0);
      spebox->Draw();
    }// end loop over spes
  }

}



void darkart::View::analyze(art::Event const & e)//called for each event
{ 

  std::map<int, int> ch_pad_map;


  {//open scope for lock guard
    LockGuard lock(gr.mutex);//until this gets deleted, don't let the graphics thread update, to avoid memory conflicts

    auto const& evtInfo = e.getValidHandle<darkart::EventInfo>(event_info_tag_);


    if (static_cast<uint>(evtInfo->event_id) != e.event()) {
      std::cout << "WARNING: art::Event::event() does not match darkart::EventInfo::event_id."<<std::endl
                << "You may not actually be viewing event "<<evtInfo->event_id<<std::endl;
    }
    
    std::cout << "Processing event "<<evtInfo->event_id<<std::endl;
    
    int padn=0;

    ///////////////////////////////////////////////////////////////////
    ///////////////    PLOT PHYSICAL CHANNELS      ////////////////////
    ///////////////////////////////////////////////////////////////////

    if (draw_physical_) {
      art::Handle<darkart::ChannelVec> chVecHandle;
      e.getByLabel(ch_tag_, chVecHandle);
      art::Handle<darkart::PulseVec> pulseVecHandle;
      e.getByLabel(pulse_tag_, pulseVecHandle);
    
      // determine how many channels we will plot based on how many we're told to skip
      int nchans = 0;
      for (size_t ch=0; ch<chVecHandle->size(); ch++) {
        if (!util::skipChannel(ch, skip_channels_))
          nchans++;
      }

      // Format the canvas
      can.cd();
      can.Clear();
      std::ostringstream canvas_title;
      canvas_title << "Run "<<evtInfo->run_id<<" Event "<<evtInfo->event_id;
      can.SetTitle(canvas_title.str().c_str());
      divide_canvas(can, nchans); 

      darkart::ByChannel<darkart::Waveform>     get_raw           (chVecHandle, e, raw_wfm_tag_);
      darkart::ByChannel<darkart::Waveform>     get_bs_wfm        (chVecHandle, e, bs_wfm_tag_);
      darkart::ByChannel<darkart::Waveform>     get_integral      (chVecHandle, e, int_wfm_tag_);
      darkart::ByChannel<darkart::Baseline>     get_bs_info       (chVecHandle, e, baseline_tag_);
      darkart::ByChannel<darkart::WaveformInfo> get_integral_info (chVecHandle, e, int_wfm_tag_);
      darkart::ManyByChannel<darkart::Pulse>    get_pulse_vec     (chVecHandle, e, pulse_tag_);
      darkart::ByPulse<darkart::PulseParam>     get_pulse_param   (pulseVecHandle, e, pulseparam_tag_);
      darkart::ManyByChannel<darkart::Spe>      get_spe_vec       (chVecHandle, e, spe_tag_);


      // Loop through channel IDs in order so that they'll be plotted in order
      for (size_t ch=0; ch<chVecHandle->size(); ch++) {
        if (util::skipChannel(ch, skip_channels_))
          continue;

        ++padn;
        can.cd(padn);
        
        auto const& channel       = util::getByChannelID(chVecHandle, ch);
        auto const& raw           = *get_raw(ch);
        auto const& bs_wfm        = *get_bs_wfm(ch);
        auto const& integral      = *get_integral(ch);
        auto const& bs_info       = *get_bs_info(ch);
        auto const& integral_info = *get_integral_info(ch);

        auto pulsePtrVec = get_pulse_vec(ch);
        // sort pulsePtrVec because getter doesn't always preserve order
        std::sort(pulsePtrVec.begin(), pulsePtrVec.end());
        darkart::PulseVec pulseVec;
        darkart::PulseParamVec pulseParamVec;
        for (size_t i=0; i<pulsePtrVec.size(); i++) {
          pulseVec.emplace_back(*pulsePtrVec[i]);
          pulseParamVec.emplace_back(*get_pulse_param(pulsePtrVec[i]->pulseID));
        }

        auto spePtrVec = get_spe_vec(ch); //vector< Ptr<Spe> >
        // sort spePtrVec because getter doesn't always preserve order
        std::sort(spePtrVec.begin(), spePtrVec.end());
        darkart::SpeVec speVec;
        // manually strip off the Ptrs; this will eventually be done by a utility function
        for (size_t i=0; i<spePtrVec.size(); i++)
          speVec.push_back(*spePtrVec[i]);

        // The various waveforms to draw will be added to this TMultiGraph
        TMultiGraph* mg = new TMultiGraph();
        std::ostringstream pad_name;
        pad_name << "r"<<evtInfo->run_id<<"e"<<evtInfo->event_id<<"ch"<<ch;
        mg->SetName(pad_name.str().c_str());
        mg->SetTitle(pad_name.str().c_str());

        gPad->SetName(pad_name.str().c_str());
        gStyle->SetTitleSize(1);
        ch_pad_map.insert( std::pair<int, int>(ch, padn) );

        
        // Format and draw everything
        draw_processed(mg, channel, raw, bs_wfm, bs_info, integral, integral_info, pulseVec, pulseParamVec,speVec,
                       draw_baseline_subtracted_, draw_integral_, draw_baseline_, draw_interpolations_, draw_pulses_, draw_single_pe_);

        can.Update();



      }
      can.cd(0);
      can.Update();
    }
    else //!draw_physical_
      can.Close();


    ///////////////////////////////////////////////////////////////////
    /////////////////     PLOT SUM CHANNEL      ///////////////////////
    ///////////////////////////////////////////////////////////////////
    
    if (draw_sum_) {
      art::Handle<darkart::ChannelVec> sumchVecHandle;
      e.getByLabel(sumch_tag_, sumchVecHandle);
      art::Handle<darkart::PulseVec> pulseVecHandle;
      e.getByLabel(sumch_pulse_tag_, pulseVecHandle);
      sum_canvas.cd();
      sum_canvas.Clear();
      std::ostringstream sum_canvas_title;
      sum_canvas_title << "Run "<<evtInfo->run_id<<" Event "<<evtInfo->event_id<<" SUM";
      sum_canvas.SetTitle(sum_canvas_title.str().c_str());

      darkart::ByChannel<darkart::Waveform>     get_raw           (sumchVecHandle, e, sumch_raw_wfm_tag_);
      darkart::ByChannel<darkart::Waveform>     get_bs_wfm        (sumchVecHandle, e, sumch_bs_wfm_tag_);
      darkart::ByChannel<darkart::Waveform>     get_integral      (sumchVecHandle, e, sumch_int_wfm_tag_);
      darkart::ByChannel<darkart::Baseline>     get_bs_info       (sumchVecHandle, e, sumch_baseline_tag_);
      darkart::ByChannel<darkart::WaveformInfo> get_integral_info (sumchVecHandle, e, sumch_int_wfm_tag_);
      darkart::ManyByChannel<darkart::Pulse>    get_pulse_vec     (sumchVecHandle, e, sumch_pulse_tag_);
      darkart::ByPulse<darkart::PulseParam>     get_pulse_param   (pulseVecHandle, e, sumch_pulseparam_tag_);
      
      
      const int ch = darkart::Channel::SUMCH_ID;
      auto const& channel       = util::getByChannelID(sumchVecHandle, ch);
      auto const& raw           = *get_raw(ch);
      auto const& bs_wfm        = *get_bs_wfm(ch);
      auto const& integral      = *get_integral(ch);
      auto const& bs_info       = *get_bs_info(ch);
      auto const& integral_info = *get_integral_info(ch);

      auto pulsePtrVec = get_pulse_vec(ch);
      // sort pulsePtrVec because getter doesn't always preserve order. use here for conistency
      std::sort(pulsePtrVec.begin(), pulsePtrVec.end());
      darkart::PulseVec pulseVec;
      darkart::PulseParamVec pulseParamVec;
      for (size_t i=0; i<pulsePtrVec.size(); i++) {
        pulseVec.emplace_back(*pulsePtrVec[i]);
        pulseParamVec.emplace_back(*get_pulse_param(pulsePtrVec[i]->pulseID));
      }

      // create empty SpeVec to pass into draw function. won't be plotted.
      darkart::SpeVec speVec;
      
      // The various waveforms to draw will be added to this TMultiGraph
      TMultiGraph* mg = new TMultiGraph();
      std::ostringstream pad_name;
      pad_name << "r"<<evtInfo->run_id<<"e"<<evtInfo->event_id<<"chSUM";
      mg->SetName(pad_name.str().c_str());
      mg->SetTitle(pad_name.str().c_str());

      gPad->SetName(pad_name.str().c_str());
      gStyle->SetTitleSize(1);
      // Format and draw everything; draw_single_pe_ must be false for sum channel
      draw_processed(mg, channel, raw, bs_wfm, bs_info, integral, integral_info, pulseVec, pulseParamVec, speVec,
                     draw_baseline_subtracted_, draw_integral_, draw_baseline_, draw_interpolations_, draw_pulses_,false);

      sum_canvas.Update();
    }
    else //!draw_sum_
      sum_canvas.Close();

    //////////////////////////////////////////////////////////////////
    /////////     Print a few useful bits of information     /////////
    //////////////////////////////////////////////////////////////////

    art::Handle<darkart::PulseVec> eventPulseVecHandle;
    e.getByLabel(event_pulse_tag_, eventPulseVecHandle);
    auto const& evtPlsVec(*eventPulseVecHandle);
    darkart::ByPulse<darkart::PulseParam> get_evt_pulse_param(eventPulseVecHandle, e, event_pulse_tag_);


    evtInfo->print();
    
    for (auto const& pulse : evtPlsVec) {
      pulse.print(1);
      get_evt_pulse_param(pulse.pulseID)->print();
    }

    std::cout << "----------------------------------------"<<std::endl;

    
    
  }//lock guard is deleted


  
  //////////////////////////////////////////////////////////////////
  //////////////     Decide what to do next       //////////////////
  //////////////////////////////////////////////////////////////////
  
  while (true) {
    std::cout << "\nOptions: " << std::endl
              << "  <ENTER> to finish viewing this event"<< std::endl;
    if (draw_physical_) {std::cout << "  c to plot a specific channel on a separate canvas"<<std::endl;}
    std::cout << "  q to quit" << std::endl;
    std::string input;
    getline(std::cin, input);
    if (input == "")
      return;
    else if (input == "c") {

      std::cout << "Enter a channel to plot: ";
      getline(std::cin, input);
      int ch = std::atoi(input.c_str());

      if (ch==darkart::Channel::SUMCH_ID) {
        std::cout << "Sum channel already on its own canvas!" << std::endl;
        continue;
      }
      else if (ch_pad_map.find(ch) == ch_pad_map.end()) {
        std::cout << "That channel doesn't exist on the plots currently drawn." << std::endl;
        continue;
      }
      else {
        std::cout << "Redrawing channel "<<ch<<std::endl;
        {//open scope for lock guard
          LockGuard lock(gr.mutex);
          std::ostringstream cname;
          cname<<"r"<<e.run()<<"e"<<e.event()<<"ch"<<ch<<"_c";
          TCanvas* c_new = new TCanvas(cname.str().c_str(), cname.str().c_str(), 100,100,1000,500);

          // Go to the pad we want to replicate, loop through all its
          // primitives and draw each one on the new canvas
          can.cd(ch_pad_map[ch]);
          TIter next(gPad->GetListOfPrimitives());
          while (TObject *obj = next()) {
            c_new->cd();
            obj->Draw(next.GetOption());
            can.cd(ch_pad_map[ch]);
          }
          c_new->cd();
        }//lock guard is deleted
      }
        
        
        
    }
        
    else if (input == 'q')
      throw cet::exception("QuitViewer") << "This message is expected because of the clumsy way of quitting art that I have implemented right now."<<std::endl;
    else
      std::cout << "Unrecognized input. Try again." << std::endl;
  }


}



DEFINE_ART_MODULE(darkart::View)
