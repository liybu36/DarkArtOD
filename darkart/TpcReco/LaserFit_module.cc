////////////////////////////////////////////////////////////////////////
// Class:       LaserFit
// Module Type: analyzer
// File:        LaserFit_module.cc
//
// Generated at Tue Aug 13 15:21:00 2013 by Jason Brodsky using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include "darkart/Products/LaserSpectrum.hh"

#include "darkart/TpcReco/RootGraphics.hh"
#include "darkart/TpcReco/laserfit_algs.hh"

#include "darksidecore/ArtServices/DBInterface.hh"

#include "TApplication.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TRootCanvas.h"
#include "TF1.h"
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <map>

namespace darkart {
  class LaserFit;
}

class darkart::LaserFit : public art::EDAnalyzer {
public:
  explicit LaserFit(fhicl::ParameterSet const & );
  virtual ~LaserFit();

  void analyze(art::Event const & ) override;

  void beginJob() override;
  void beginRun(art::Run const & ) override;
  void endJob() override;
  void endRun(art::Run const & ) override;

private:
  art::InputTag ch_producer_tag_;
  art::InputTag laserSpectra_tag_;
  
  art::ServiceHandle<art::TFileService> tfs_;
  TTree* tree_;
  LaserResultsData res_data;
  bool save_histograms;
  bool skip_interactive;
  bool autowrite;
  // Declare member data here.
  darkart::RootGraphics gr;
  TCanvas can;
  TCanvas can2;
  TCanvas can3;

  fhicl::ParameterSet fit_config;
  std::string version_db;
  fhicl::ParameterSetID fit_psetID;
  art::ServiceHandle<ds50::DBInterface> dbi;
};


darkart::LaserFit::LaserFit(fhicl::ParameterSet const & p)
  :EDAnalyzer(p),
  ch_producer_tag_(p.get<std::string>("ch_producer_tag")),
 laserSpectra_tag_(p.get<std::string>("laserSpectra_tag")), 
 tfs_(), tree_(nullptr),res_data(),
 save_histograms(p.get<bool>("save_histograms")), 
 skip_interactive(p.get<bool>("skip_interactive")), 
 autowrite(p.get<bool>("autowrite")),
 gr(p.get<bool>("draw_results")), 
 can("lasercanvas","Laser Integral Spectum",50,50,1800,900),
 can2("auxcanvas","Laser Integral Subtracted Spectrum",50,50,1800,900),
 can3("ampcanvas","Laser Amplitude Subtracted Spectrum",50,50,1800,900),
 fit_config(p.get<fhicl::ParameterSet>("laser_fit_params")), version_db(p.get<std::string>("version_db")),
 fit_psetID(p.id())
// Initialize member data here.
{
  ((TRootCanvas*)can.GetCanvasImp ())->DontCallClose();
  can.SetLogy(1);
  ((TRootCanvas*)can2.GetCanvasImp ())->DontCallClose();
  //can2.SetLogy(1);
  ((TRootCanvas*)can3.GetCanvasImp ())->DontCallClose();
  //can3.SetLogy(1);
}

darkart::LaserFit::~LaserFit()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::LaserFit::analyze(art::Event const & e)
{
  if(e.event() % 1000==0) dbi->latest("dark_art.laser_calibration","-4444");//dumy query to try and prvent timeout

}

void darkart::LaserFit::beginJob()
{
    tree_ = tfs_->make<TTree>("Events", "Events");
    tree_->Branch("LaserData", &res_data);
}

void darkart::LaserFit::beginRun(art::Run const & r)
{
    std::ostringstream title_ss1, title_ss2, title_ss3;
    title_ss1<<r.id()<<" Laser Integral Spectum";
    title_ss2<<r.id()<<" Laser Integral Subtracted Spectrum";
    title_ss3<<r.id()<<" Laser Amplitude Subtracted Spectrum";
    can.SetTitle(title_ss1.str().c_str());
    can2.SetTitle(title_ss2.str().c_str());
    can3.SetTitle(title_ss3.str().c_str());
  

  //std::cout<<"database working: "<<res.get<int>("run",0,0);
}

void darkart::LaserFit::endJob()
{
  // Implementation of optional member function here.
}

void darkart::LaserFit::endRun(art::Run const & r)
{

  art::Handle<darkart::LaserSpectrumMap> lsMapHandle;
  r.getByLabel(laserSpectra_tag_, lsMapHandle);
  darkart::LaserSpectrumMap laserSpectra(*lsMapHandle);
  
  std::map<int, int> ch_pad_map;

  for (auto spectrum_pair : laserSpectra)
  {
    std::cout << "fitting channel "<<spectrum_pair.first.asInt()<<std::endl;
      std::ostringstream this_channel;
      this_channel<<"channel"<<spectrum_pair.first.asInt();
      laserEvalParams config_this_channel(fit_config.get<fhicl::ParameterSet>(this_channel.str(),fit_config.get<fhicl::ParameterSet>("default_channel")));
      eval_laser_spectrum(*spectrum_pair.second, config_this_channel);
      res_data = spectrum_pair.second->results.data;
      res_data.run = r.run();
      res_data.channel = spectrum_pair.first.asInt();
      tree_->Fill();
      dbi->latest("dark_art.laser_calibration","-4444"); // more dummy queries because fits could take a while
  }
  print_laser_fit_results(laserSpectra);
  draw_laser_spectra(laserSpectra, can, gr,ch_pad_map);
  draw_subtracted_integral_spectra(laserSpectra, can2, gr);
  draw_subtracted_amplitude_spectra(laserSpectra, can3, gr);

  std::string response = "";
  gr.Start();

  while(response != "q" && response !="w")
  {
      if(autowrite)
      {
	  response="A";
	  autowrite=false;
      }
      else if (skip_interactive)
      {
	  response ="q";
      }
      else
      {
	  std::cout<<"'q' to quit, 'w' to write, 'c' to choose ch to redraw"
		   <<std::endl;
	  std::cin>>response;
      }
      
      if (response == "c") 
      {

	  std::cout << "Enter a channel to plot: "<<std::endl;
	  std::string input="";
	  std::cin>>input;
	  int ch = std::atoi(input.c_str());
	      
	  if (ch_pad_map.find(ch) == ch_pad_map.end()) 
	  {
	      std::cout << "That channel doesn't exist on the plots currently drawn." 
			<< std::endl;
	      continue;
	  }
	  else 
	  {
	      std::cout << "Redrawing channel "<<ch<<std::endl;
	      {//open scope for lock guard
		  LockGuard lock(gr.mutex);
		  std::ostringstream cname;
		  cname<<"ch"<<ch<<"_c";
		  TCanvas* c_new = new TCanvas(cname.str().c_str(), cname.str().c_str(), 100,100,1000,500);
		  c_new->SetLogy();
		  // Go to the pad we want to replicate, loop through all its
		  // primitives and draw each one on the new canvas
		  can.cd(ch_pad_map[ch]);
		  TIter next(gPad->GetListOfPrimitives());
		  while (TObject *obj = next()) 
		  {
		      c_new->cd();
		      obj->Draw(next.GetOption());
		      can.cd(ch_pad_map[ch]);
		  }
		  c_new->cd();
	      }//lock guard is deleted
	  }
      }
    
      else if(response=="w" || response=="A")
      {
        
        std::stringstream updatequery;
        updatequery<<"UPDATE dark_art.laser_calibration SET version = version || '-' || inserttime WHERE run = "<<r.run()
                   <<"AND version = '"<<version_db<<"';";
        dbi->query(updatequery.str(), true);
	      
	  ds50::db::result res = dbi->latest("dark_art.laser_calibration","-4444");
	  res.wipe();
	  res.set<int>("run",0,r.run());
	  res.set("version",0,version_db);
	  res.set("comment",0,"early insert test");
	      
	  char inserttime_str[20];
	  time_t currenttime;
	  time(&currenttime);
	  std::strftime(inserttime_str,20,"%Y-%m-%d %H:%M:%S",std::gmtime(&currenttime));
	      
	  char runtime_str[20];
	  time_t runtime(r.beginTime().timeHigh());
	  //std::stringstream runtime_ss;
	  //runtime_ss<<"to_timestamp("<<r.beginTime().timeHigh()<<")";
	  std::strftime(runtime_str,20,"%Y-%m-%d %H:%M:%S",std::gmtime(&runtime));
	  res.set("runtime",0,runtime_str);
	  //res.set("runtime",0,"1970-10-22 22:30:15.673233+02");
	  res.set("inserttime",0,inserttime_str);
	  //res.set("inserttime",0,"2013-10-22 22:30:15.673233+02");
	  res.set("illumination_setting",0,"test");
	      
	  const  art::Provenance* spectrum_provenance = lsMapHandle.provenance();
	  const std::set<fhicl::ParameterSetID> spectrum_psetID_set = spectrum_provenance->psetIDs();
	  if(!spectrum_psetID_set.empty())
	  {
	      res.set("spectrum_provenance_hash",0,spectrum_psetID_set.begin()->to_string());
	  }
	  else
	  {
	      res.set("spectrum_provenance_hash",0,"provenance not found");
	  }
	  res.set("fit_config_hash",0,fit_psetID.to_string());
	  res.set("use_me",0,true);
	      
      
	  int cell_n=0;
	  for(auto spair : laserSpectra)
	  {
		  
	      //	     TH1* h = &spair.second->hist;
	      LaserResultsData* resultsData= &spair.second->results.data;
              res.set("channel_id",0,cell_n, spair.first.asInt());
              res.set("entries",0,cell_n, spair.second->laser_int_hist.GetEntries());
	      res.set("lambda",0,cell_n, resultsData->occupancy);
	      res.set("ser_mean",0,cell_n, resultsData->ser_mean);
	      res.set("ser_mean_err",0,cell_n, sqrt(resultsData->ser_mean_unc2));
	      res.set("ser_sigma",0,cell_n, resultsData->ser_sigma);
	      res.set("gauss_center",0,cell_n, resultsData->gauss_center);
	      res.set("gauss_sigma",0,cell_n, resultsData->gauss_sigma);
	      res.set("slope_e",0,cell_n, resultsData->total_mean);
	      res.set("constant",0,cell_n, (int) resultsData->total_var);
              res.set("p_e",0,cell_n, resultsData->pedestal_mean);
	      res.set("ped_sigma",0,cell_n,resultsData->pedestal_var);
	      res.set("lambda_err_low",0,cell_n, sqrt(resultsData->occupancy_unc2));
	      res.set("lambda_err_high",0,cell_n, sqrt(resultsData->occupancy_unc2));
              res.set("gauss_center_err_low",0,cell_n, -8888);
	      res.set("gauss_center_err_high",0,cell_n, -8888);
	      res.set("gauss_sigma_err_low",0,cell_n,-8888);
	      res.set("gauss_sigma_err_high",0,cell_n,-8888);
	      res.set("slope_e_err_low",0,cell_n, sqrt(resultsData->total_mean_unc2));
	      res.set("slope_e_err_high",0,cell_n,  sqrt(resultsData->total_mean_unc2));
	      res.set("p_e_err_low",0,cell_n, sqrt(resultsData->pedestal_mean_unc2));
	      res.set("p_e_err_high",0,cell_n, sqrt(resultsData->pedestal_mean_unc2));
	      res.set("ped_sigma_err_low",0,cell_n,-8888);
	      res.set("ped_sigma_err_high",0,cell_n,-8888);
	      res.set("ser_mean_err_low",0,cell_n, resultsData->int_shift_correction);
	      res.set("ser_mean_err_high",0,cell_n, resultsData->int_shift_correction);
	      res.set("ser_sigma_err_low",0,cell_n, resultsData->amp_shift_correction);
	      res.set("ser_sigma_err_high",0,cell_n, resultsData->amp_shift_correction);
	      res.set("chi2",0,cell_n,resultsData->laser_amp_frac_corr);
	      res.set("peak_valley_ratio",0,cell_n, resultsData->amplitude_threshold);
	      ++cell_n;
	  }
	      
	  std::cout<<"About to insert"<<std::endl;
	      
	  dbi->insert(res,version_db);
          std::cout<<"Wrote to DB"<<std::endl;
      }
      gr.Stop();
  }


  if (save_histograms)
  {
      tfs_->file().cd();
      
      for(LaserSpectrumMap::iterator it = laserSpectra.begin(); it!=laserSpectra.end();++it)
      {
	  TString name = "channel_";
	  name += it->first.asInt(); name += "_";
	  
	  TString histname;
	  histname = name + it->second->laser_int_hist.GetName();
	  it->second->laser_int_hist.Write(histname);
	  
	  histname = name + it->second->laser_amp_hist.GetName();
	  it->second->laser_amp_hist.Write(histname);
	  
	  histname = name + it->second->pedestal_int_hist.GetName();
	  it->second->pedestal_int_hist.Write(histname);
	  
	  histname = name + it->second->pedestal_amp_hist.GetName();
	  it->second->pedestal_amp_hist.Write(histname);
	  
	  histname = name + it->second->results.laser_int_sub_hist.GetName();
	  it->second->results.laser_int_sub_hist.Write(histname);
	  
	  histname = name + it->second->results.laser_amp_sub_hist.GetName();
	  it->second->results.laser_amp_sub_hist.Write(histname);
	  
	  histname = name + it->second->results.ped_int_scaled_hist.GetName();
	  it->second->results.ped_int_scaled_hist.Write(histname);
	  
	  histname = name + it->second->results.ped_amp_scaled_hist.GetName();
	  it->second->results.ped_amp_scaled_hist.Write(histname);
	  
	  histname = name + it->second->results.int_shift_chi2_graph.GetName();
	  it->second->results.int_shift_chi2_graph.Write(histname);

	  histname = name + it->second->results.amp_shift_chi2_graph.GetName();
	  it->second->results.amp_shift_chi2_graph.Write(histname);

	  std::cout<<"Wrote histograms for channel: "<<it->first.asInt()<<std::endl;
      }
  }
  return;
}

DEFINE_ART_MODULE(darkart::LaserFit)
