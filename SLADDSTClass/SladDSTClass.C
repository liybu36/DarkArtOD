#include "SladDSTClass.h"

using namespace std;
ClassImp(SladDSTClass);

void SladDSTClass::Init()
{
  od_coinc_wind = 3930.; //[ns] obtained from validation fits

  tpc_s1_late_min = 3500;
  tpc_s1_late_max = 35000;
  tpc_s1_min = 60;
  tpc_s1_max = 1000;
  tpc_total_f90Nmin = 0.6;
  tpc_total_f90Nmax = 0.85;
  tpc_total_f90Gmin = 0.1;
  tpc_total_f90Gmax = 0.4;
  
}

bool SladDSTClass::load_tpctree(TChain* tpctree, TPCEvent & e)
{
  //read the branches of TPC SLAD files
  /*  tpctree->SetBranchStatus("*",0); //disable all
  tpctree->SetBranchStatus("events.run_id", 1);
  tpctree->SetBranchStatus("events.event_id", 1);
  tpctree->SetBranchStatus("gps.gps_coarse", 1);
  tpctree->SetBranchStatus("gps.gps_fine", 1);
  tpctree->SetBranchStatus("nchannel.nchannel", 1);
  tpctree->SetBranchStatus("baseline.SumChannelHasNoBaseline",1);
  tpctree->SetBranchStatus("long_lifetime.lifetime",1);
  tpctree->SetBranchStatus("long_lifetime.inhibittime",1);
  tpctree->SetBranchStatus("npulses.n_phys_pulses",1);
  tpctree->SetBranchStatus("npulses.has_s3",1);
  tpctree->SetBranchStatus("tdrift.tdrift",1);
  tpctree->SetBranchStatus("s1_time.s1_start_time", 1);
  tpctree->SetBranchStatus("s1.total_s1_corr", 1);
  tpctree->SetBranchStatus("s1_f90.total_f90", 1);
  tpctree->SetBranchStatus("s2.total_s2_corr", 1);
  tpctree->SetBranchStatus("s2_fraction.s2_chan", 1);
  tpctree->SetBranchStatus("s2_f90.total_s2_f90", 1);
  tpctree->SetBranchStatus("acqui_window.acqui_window", 1);
  tpctree->SetBranchStatus("s1.total_s1_top", 1);
  tpctree->SetBranchStatus("s1.total_s1_bottom", 1);
  tpctree->SetBranchStatus("s1_saturation.is_saturated_pulse0", 1);
  tpctree->SetBranchStatus("s1_time.s1_end_time", 1);
  tpctree->SetBranchStatus("s1_time.s2_start_time", 1);
  tpctree->SetBranchStatus("s1_time.s2_end_time", 1);
  tpctree->SetBranchStatus("s1_f90.total_f90_spe_mean", 1);
  tpctree->SetBranchStatus("s1_fracton.s1_max_chan", 1);
  tpctree->SetBranchStatus("masas_xy.masas_x", 1);
  tpctree->SetBranchStatus("masas_xy.masas_y", 1);
  tpctree->SetBranchStatus("xylocator_xy.xyl_best_x", 1);
  tpctree->SetBranchStatus("xylocator_xy.xyl_best_y", 1);
  */
  tpctree->SetBranchAddress("run_id", &e.run_id);
  tpctree->SetBranchAddress("event_id", &e.event_id);
  tpctree->SetBranchAddress("gps_coarse", &e.gps_coarse);
  tpctree->SetBranchAddress("gps_fine", &e.gps_fine);
  tpctree->SetBranchAddress("nchannel.nchannel", &e.nchannel);
  tpctree->SetBranchAddress("baseline.SumChannelHasNoBaseline", &e.baseline_not_found);
  tpctree->SetBranchAddress("long_lifetime.lifetime", &e.live_time);
  tpctree->SetBranchAddress("long_lifetime.inhibittime", &e.inhibit_time);
  tpctree->SetBranchAddress("acqui_window.acqui_window", &e.acqui_window);
  tpctree->SetBranchAddress("npulses.n_phys_pulses", &e.npulses);
  tpctree->SetBranchAddress("npulses.has_s3", &e.has_s3);
  tpctree->SetBranchAddress("tdrift.tdrift", &e.tdrift);
  tpctree->SetBranchAddress("s1_time.s1_start_time", &e.s1_start_time);
  tpctree->SetBranchAddress("s1_time.s1_end_time", &e.s1_end_time);
  tpctree->SetBranchAddress("s1.total_s1_corr", &e.s1);
  tpctree->SetBranchAddress("s1_f90.total_f90", &e.s1_total_f90);
  tpctree->SetBranchAddress("s1_f90.total_f90_spe_mean", &e.s1_total_f90_spe_mean);
  tpctree->SetBranchAddress("s1.total_s1_top", &e.total_s1_top);
  tpctree->SetBranchAddress("s1.total_s1_bottom", &e.total_s1_bottom);
  tpctree->SetBranchAddress("s1_saturation.is_saturated_pulse0", &e.is_saturated_pulse0);
  tpctree->SetBranchAddress("s1_fraction.s1_max_chan", &e.s1_max_chan);
  tpctree->SetBranchAddress("s1_fraction.s1_max_frac", &e.s1_max_frac);
  tpctree->SetBranchAddress("max_s1_frac_cut.max_s1_frac_cut_threshold99", &e.max_s1_frac_cut_threshold99);
  tpctree->SetBranchAddress("max_s1_frac_cut.max_s1_frac_cut_exceeds99", &e.max_s1_frac_cut_exceeds99);
  tpctree->SetBranchAddress("s2.total_s2_corr", &e.s2);
  tpctree->SetBranchAddress("s2_f90.total_s2_f90", &e.s2_total_f90);
  tpctree->SetBranchAddress("s2_f90.total_s2_f90_spe_mean", &e.s2_total_f90_spe_mean);
  tpctree->SetBranchAddress("s2.total_s2_top", &e.total_s2_top);
  tpctree->SetBranchAddress("s2.total_s2_bottom", &e.total_s2_bottom);
  tpctree->SetBranchAddress("s1_time.s2_start_time", &e.s2_start_time);
  tpctree->SetBranchAddress("s1_time.s2_end_time", &e.s2_end_time);
  tpctree->SetBranchAddress("s2_saturation.is_saturated_pulse1", &e.is_saturated_pulse1);
  tpctree->SetBranchAddress("s2_fraction.s2_chan", e.s2_ch_frac);
  tpctree->SetBranchAddress("s2_fraction.s2_max_chan", &e.s2_max_chan);
  tpctree->SetBranchAddress("s2_fraction.s2_max_frac", &e.s2_max_frac);  
  tpctree->SetBranchAddress("masas_xy.masas_x", &e.masa_x);
  tpctree->SetBranchAddress("masas_xy.masas_y", &e.masa_y);
  tpctree->SetBranchAddress("xylocator_xy.xyl_best_x", &e.jason_x);
  tpctree->SetBranchAddress("xylocator_xy.xyl_best_y", &e.jason_y);

  return true;
}

bool SladDSTClass::load_odtree(TChain* odtree, ODEvent & d)
{
  //read the branches from SLADOD file   
  odtree->SetBranchStatus("*",0); //disable all
  odtree->SetBranchStatus("run_id",1);
  odtree->SetBranchStatus("event_id",1);
  odtree->SetBranchStatus("gps_fine",1);
  odtree->SetBranchStatus("gps_coarse",1);
  odtree->SetBranchStatus("gps_pps",1);
  odtree->SetBranchStatus("gps_timestamp_sec",1);
  odtree->SetBranchStatus("dt_usec",1);
  odtree->SetBranchStatus("trigger_type",1);
  odtree->SetBranchStatus("bad_time_alignment",1);
  odtree->SetBranchStatus("nclusters_lsv",1);
  odtree->SetBranchStatus("clusters_lsv_max_multiplicity",1);
  odtree->SetBranchStatus("clusters_lsv_charge",1);
  odtree->SetBranchStatus("clusters_lsv_height",1);
  odtree->SetBranchStatus("clusters_lsv_start_ns",1);
  odtree->SetBranchStatus("lsv_total_charge",1);
  odtree->SetBranchStatus("wt_total_charge",1);

  odtree->SetBranchStatus("roi_lsv_id_vec",1);
  odtree->SetBranchStatus("roi_lsv_charge_vec",1);
  odtree->SetBranchStatus("roi_lsv_max_multiplicity_vec",1);
  odtree->SetBranchStatus("slider_lsv_id_vec",1);
  odtree->SetBranchStatus("slider_lsv_charge_vec",1);
  odtree->SetBranchStatus("slider_lsv_time_ns_vec",1);
  odtree->SetBranchStatus("slider_lsv_max_multiplicity_vec",1);

  odtree->SetBranchAddress("run_id", &d.run_id);
  odtree->SetBranchAddress("event_id", &d.event_id);
  odtree->SetBranchAddress("gps_fine", &d.gps_fine);
  odtree->SetBranchAddress("gps_coarse", &d.gps_coarse);
  odtree->SetBranchAddress("gps_pps", &d.gps_pps);
  odtree->SetBranchAddress("gps_timestamp_sec", &d.gps_timestamp_sec);
  odtree->SetBranchAddress("dt_usec", &d.dt_usec);
  odtree->SetBranchAddress("trigger_type", &d.trigger_type);
  odtree->SetBranchAddress("bad_time_alignment", &d.bad_time_alignment);
  odtree->SetBranchAddress("nclusters_lsv", &d.nclusters_lsv);
  odtree->SetBranchAddress("clusters_lsv_max_multiplicity", &d.clusters_lsv_max_multiplicity);
  odtree->SetBranchAddress("clusters_lsv_charge", &d.clusters_lsv_charge);
  odtree->SetBranchAddress("clusters_lsv_height", &d.clusters_lsv_height);
  odtree->SetBranchAddress("clusters_lsv_start_ns", &d.clusters_lsv_start_ns);
  odtree->SetBranchAddress("lsv_total_charge", &d.lsv_total_charge);
  odtree->SetBranchAddress("wt_total_charge", &d.wt_total_charge);

  odtree->SetBranchAddress("roi_lsv_id_vec",&d.roi_lsv_id_vec);
  odtree->SetBranchAddress("roi_lsv_charge_vec",&d.roi_lsv_charge_vec);
  odtree->SetBranchAddress("roi_lsv_max_multiplicity_vec",&d.roi_lsv_max_multiplicity_vec);
  odtree->SetBranchAddress("slider_lsv_id_vec",&d.slider_lsv_id_vec);
  odtree->SetBranchAddress("slider_lsv_charge_vec",&d.slider_lsv_charge_vec);
  odtree->SetBranchAddress("slider_lsv_time_ns_vec",&d.slider_lsv_time_ns_vec);
  odtree->SetBranchAddress("slider_lsv_max_multiplicity_vec",&d.slider_lsv_max_multiplicity_vec);

  return true;
}

void SladDSTClass::create_tpcchain()
{
  tpc_chain = new TChain("events");
  s2_fraction = new TChain("s2_fraction");
  masaxy = new TChain("masas_xy");
  jasonxy = new TChain("xylocator_xy");
}

bool SladDSTClass::add_tpctree(int i)
{ 
  TString tpcfile;
  tpcfile.Form("Run%06d.root",i);
  tpcfile.Prepend(GetTPCPath());

  if(!VerifyDataFile(tpcfile))
    return false;

  TString s2file = tpcfile;
  s2file.Remove(s2file.Length()-5);
  s2file+="_s2.root";
  
  TString masaxyfile = tpcfile;
  masaxyfile.Remove(masaxyfile.Length()-5);
  masaxyfile+="_masas_xy.root";
  
  TString jasonxyfile = tpcfile;
  jasonxyfile.Remove(jasonxyfile.Length()-5);
  jasonxyfile+="_xylocator_xy.root";

  if(!VerifyDataFile(s2file) || !VerifyDataFile(masaxyfile) || !VerifyDataFile(jasonxyfile))
    return false;    
  
  tpc_chain->Add(tpcfile);
  s2_fraction->Add(s2file);
  masaxy->Add(masaxyfile);
  jasonxy->Add(jasonxyfile);
  
  return true;
}

void SladDSTClass::add_tpcfriend()
{
  tpc_chain->AddFriend(s2_fraction);
  tpc_chain->AddFriend(masaxy);
  tpc_chain->AddFriend(jasonxy); 
}

void SladDSTClass::create_odchain()
{
  od_chain = new TChain("odtree");
}

bool SladDSTClass::add_odtree(int i)
{ 
  TString odfile;
  odfile.Form("SLADOD_Run%06d.root",i);
  odfile.Prepend(GetODPath());
  if(!VerifyDataFile(odfile))
    return false;
  else
    od_chain->Add(odfile);
  return true;
}

bool SladDSTClass::VerifyDataFile(TString mainfile)
{
  ifstream NameCheck;
  NameCheck.open(mainfile.Data());
  if(!NameCheck.good())
    { NameCheck.close();
      return false;
    }
  else{
    TFile *f = new TFile(mainfile);
    if(f->IsZombie())
      {
	NameCheck.close();
	f->Close();
	return false;
      }
    else{
      cout<<"Processing the data file: "<<mainfile<<endl;
      NameCheck.close();
      f->Close();
      return true;
    }
  }
}

void SladDSTClass::ReadRunList(vector<int>& run)
{
  string runlist = "50days_runlist.txt";
  ifstream indata (runlist.c_str());
  if(indata.is_open())
    {
      while(!indata.eof())
	{
	  string s1;
	  indata>>s1;
	  run.push_back(atoi(s1.c_str()));
	}
    }
  indata.close();
}

void SladDSTClass::SetOutTPCTree()
{
  ////////////////////////////////////////////////////////////////////
  /////////////////     Declare SLAD DSTtree Variables   /////////////
  ////////////////////////////////////////////////////////////////////
  //***************************SLAD DSTtree TPC Variables************************
  DSTtree->Branch("runID",             &t.runID,              "runID/I");
  DSTtree->Branch("tpc_eventID",       &t.tpc_eventID,        "tpc_eventID/I");
  DSTtree->Branch("tpc_timestamp",     &t.tpc_timestamp,      "tpc_timestamp/D");
  DSTtree->Branch("tpc_event_type",    &t.tpc_event_type,     "tpc_event_type/I");
  /*
  DSTtree->Branch("tpc_gps_fine",      &t.tpc_gps_fine,       "tpc_gps_fine/D");
  DSTtree->Branch("tpc_gps_coarse",    &t.tpc_gps_coarse,     "tpc_gps_coarse/D");
  DSTtree->Branch("tpc_npulses",       &t.tpc_npulses,        "tpc_npulses/I");
  DSTtree->Branch("tpc_live_time",     &t.tpc_live_time,      "tpc_live_time/D");
  DSTtree->Branch("tpc_inhibit_time",  &t.tpc_inhibit_time,   "tpc_inhibit_time/D");
  DSTtree->Branch("tpc_has_s3",        &t.tpc_has_s3,         "tpc_has_s3/I");
  DSTtree->Branch("tpc_acqui_window",  &t.tpc_acqui_window,   "tpc_acqui_window/D");
  DSTtree->Branch("tpc_t_drift",       &t.tpc_t_drift,        "tpc_t_drift/D");
  DSTtree->Branch("tpc_total_s1",      &t.tpc_total_s1,       "tpc_total_s1/D");
  DSTtree->Branch("tpc_total_s1_f90",  &t.tpc_total_s1_f90,   "tpc_total_s1_f90/D");
  DSTtree->Branch("tpc_s1_total_f90_spe_mean", &t.tpc_s1_total_f90_spe_mean, "tpc_s1_total_f90_spe_mean/D");
  DSTtree->Branch("tpc_s1_start_time", &t.tpc_s1_start_time,  "tpc_s1_start_time/D");
  DSTtree->Branch("tpc_s1_end_time",    &t.tpc_s1_end_time,    "tpc_s1_end_time/D");
  DSTtree->Branch("tpc_total_s1_top",  &t.tpc_total_s1_top,   "tpc_total_s1_top/D");
  DSTtree->Branch("tpc_total_s1_bottom", &t.tpc_total_s1_bottom,"tpc_total_s1_bottom/D");
  DSTtree->Branch("tpc_is_saturated_pulse0", &t.tpc_is_saturated_pulse0,"tpc_is_saturated_pulse0/I");
  DSTtree->Branch("tpc_s1_late",       &t.tpc_s1_late,        "tpc_s1_late/D");
  DSTtree->Branch("tpc_s1_max_chan", &t.tpc_s1_max_chan, "tpc_s1_max_chan/I");
  DSTtree->Branch("tpc_s1_max_frac", &t.tpc_s1_max_frac, "tpc_s1_max_frac/D");
  DSTtree->Branch("tpc_total_s2",      &t.tpc_total_s2,       "tpc_total_s2/D");
  DSTtree->Branch("tpc_total_s2_f90",  &t.tpc_total_s2_f90,   "tpc_total_s2_f90/D");
  DSTtree->Branch("tpc_s2_start_time",  &t.tpc_s2_start_time,  "tpc_s2_start_time/D");
  DSTtree->Branch("tpc_s2_end_time",    &t.tpc_s2_end_time,    "tpc_s2_end_time/D");
  DSTtree->Branch("tpc_is_saturated_pulse1", &t.tpc_is_saturated_pulse1,"tpc_is_saturated_pulse1/I");
  DSTtree->Branch("tpc_s2_total_f90_spe_mean", &t.tpc_s2_total_f90_spe_mean, "tpc_s2_total_f90_spe_mean/D");
  DSTtree->Branch("tpc_s2_max_chan", &t.tpc_s2_max_chan, "tpc_s2_max_chan/I");
  DSTtree->Branch("tpc_s2_max_frac", &t.tpc_s2_max_frac, "tpc_s2_max_frac/D");
  DSTtree->Branch("tpc_total_s2_top",  &t.tpc_total_s2_top,   "tpc_total_s2_top/D");
  DSTtree->Branch("tpc_total_s2_bottom", &t.tpc_total_s2_bottom,"tpc_total_s2_bottom/D");
  DSTtree->Branch("tpc_masa_x",        &t.tpc_masa_x,         "tpc_masa_x/F");
  DSTtree->Branch("tpc_masa_y",        &t.tpc_masa_y,         "tpc_masa_y/F");
  DSTtree->Branch("tpc_jason_x",       &t.tpc_jason_x,        "tpc_jason_x/F");
  DSTtree->Branch("tpc_jaosn_y",       &t.tpc_jason_y,        "tpc_jason_x/F");
  */
}

void SladDSTClass::SetOutODTree()
{
  //*************************SLAD DSTtree Veto Variables******************************
  DSTtree->Branch("od_eventMatched",&t.od_eventMatched,"od_eventMatched/O");
  DSTtree->Branch("od_eventID",&t.od_eventID,"od_eventID/I");
  DSTtree->Branch("od_gps_fine",&t.od_gps_fine,"od_gps_fine/D");
  DSTtree->Branch("od_gps_coarse",&t.od_gps_coarse,"od_gps_coarse/D");
  DSTtree->Branch("od_timestamp", &t.od_timestamp,"od_timestamp/D"); //time in ns
  DSTtree->Branch("od_nclusters", &t.od_nclusters,"od_nclusters/I");
  DSTtree->Branch("od_lsv_charge", &t.od_lsv_charge,"od_lsv_charge/D");
  DSTtree->Branch("od_wt_charge", &t.od_wt_charge,"od_wt_charge/D");
  DSTtree->Branch("od_cluster_charge", &t.od_cluster_charge);
  DSTtree->Branch("od_cluster_start", &t.od_cluster_start); // time in ns
  DSTtree->Branch("od_cluster_height", &t.od_cluster_height);
  DSTtree->Branch("od_cluster_multiplicity", &t.od_cluster_multiplicity);
  DSTtree->Branch("od_cluster_pass_multcut", &t.od_cluster_pass_multcut);
  DSTtree->Branch("od_cluster_dtprompt", &t.od_cluster_dtprompt);
  // DSTtree->Branch("od_cluster_width", &t.od_cluster_width);
  DSTtree->Branch("od_roi_lsv_id",&t.od_roi_lsv_id);
  DSTtree->Branch("od_roi_lsv_charge",&t.od_roi_lsv_charge);
  DSTtree->Branch("od_roi_lsv_max_multiplicity",&t.od_roi_lsv_max_multiplicity);
  DSTtree->Branch("od_slider_lsv_id",&t.od_slider_lsv_id);
  DSTtree->Branch("od_slider_lsv_charge",&t.od_slider_lsv_charge);
  DSTtree->Branch("od_slider_lsv_time",&t.od_slider_lsv_time);
  DSTtree->Branch("od_slider_lsv_max_multiplicity",&t.od_slider_lsv_max_multiplicity);

  //*********************End of SLAD DSTtree Variables*********************************  
  //  DSTtree->SetDirectory(fFile);
  //  DSTtree->AutoSave();
}

//Old multiplicity cut
bool SladDSTClass::multiplicity_cut(Float_t height, Float_t multiplicity, Float_t charge){
  return height/multiplicity < (2.563e7 + TMath::Sqrt(1.574e14+1.390e12*(charge-14.40)*(charge-14.40)));
}

bool SladDSTClass::Cut_Gamma(double tpc_s1_late,double tpc_total_s1_f90){
  //cut to select 4.4 gammas
  return (tpc_s1_late>tpc_s1_late_min && tpc_s1_late<tpc_s1_late_max 
	  && tpc_total_s1_f90>tpc_total_f90Gmin && tpc_total_s1_f90<tpc_total_f90Gmax);
}

bool SladDSTClass::Cut_Neutron(double tpc_total_s1,double tpc_total_s1_f90){
  //cut to select neutrons NOTE: we could implement a cut on f90 at 2sigma of the distribution (95%)
  return (tpc_total_s1>tpc_s1_min && tpc_total_s1<tpc_s1_max 
	  && tpc_total_s1_f90>tpc_total_f90Nmin && tpc_total_s1_f90<tpc_total_f90Nmax);
}

void SladDSTClass::PreLoop()
{
  Init();
  //  TPCEvent e;
  if(!load_tpctree(tpc_chain, e))
    cout<<"Cannot load tpc tree!"<<endl;

  //  ODEvent d;
  if(!load_odtree(od_chain,d))
    cout<<"Cannot load od tree!"<<endl;
  
  f = new TFile(out_file.Data(), "RECREATE");
  f->cd();
  //Create DSTtree
  DSTtree = new TTree ("DSTtree", "tree of selected events");
  SetOutTPCTree();
  SetOutODTree();
}

bool SladDSTClass::LoopOverChain()
{
  PreLoop();
  Int_t tpc_events = tpc_chain->GetEntries();
  cout << "Total number of TPC events: "<<tpc_events << endl;
  //////////////////////////////////////////////////////////////////////////
  /////////////////     BEGIN LOOP OVER EVENTS       ///////////////////////
  //////////////////////////////////////////////////////////////////////////
  //  tpc_events -= 50; // Skip last few events because end of some (very few) runs are problematic.

  for(Int_t n_tpc = 0; n_tpc < tpc_events ; n_tpc++){
    Double_t DT =-100000;
    tpc_chain->GetEntry(n_tpc);
    if(n_tpc%10000==0)
      cout << "Processing event: " << n_tpc << "/" << tpc_events << ", RunID: " << e.run_id << '\n';
    // Generate cuts.
    bool cx1 = CX1(e);
    bool cx2 = CX2(e);
    bool cx3 = CX3(e);
    bool cx4 = CX4(e);
    bool cx8 = CX8(e);
    bool cx9 = CX9(e);
    bool basic_cuts = cx1 && cx2 && cx3 && cx4;

    if(!(basic_cuts && cx8 && cx9))
      //      continue;
      t.od_eventMatched = false;
    else
      {
	t.runID               = e.run_id;
	t.tpc_eventID         = e.event_id;
	t.tpc_timestamp       = e.gps_coarse*1.e+6 + e.gps_fine*20.e-3; // [us]	

	/*
	t.tpc_gps_fine        = e.gps_fine;
	t.tpc_gps_coarse      = e.gps_coarse;
	t.tpc_s1_start_time   = e.s1_start_time;
	t.tpc_s1_end_time     = e.s1_end_time;
	t.tpc_total_s1        = e.s1;
	t.tpc_total_s1_f90    = e.s1_total_f90;
	t.tpc_total_s2        = e.s2;
	t.tpc_total_s2_f90    = e.s2_total_f90;
	t.tpc_t_drift         = e.tdrift;
	t.tpc_s1_late         = (1-t.tpc_total_s1_f90)*t.tpc_total_s1;
	t.tpc_acqui_window    = e.acqui_window;
	t.tpc_total_s1_top    = e.total_s1_top;
	t.tpc_total_s1_bottom = e.total_s1_bottom;
	t.tpc_is_saturated_pulse0 = e.is_saturated_pulse0;
	t.tpc_is_saturated_pulse1 = e.is_saturated_pulse1;
	t.tpc_s2_start_time   = e.s2_start_time;
	t.tpc_s2_end_time     = e.s2_end_time;
	t.tpc_s1_total_f90_spe_mean = e.s1_total_f90_spe_mean;
	t.tpc_s2_total_f90_spe_mean = e.s2_total_f90_spe_mean;
	t.tpc_total_s2_top    = e.total_s2_top;
	t.tpc_total_s2_bottom = e.total_s2_bottom;
	t.tpc_max_s1_frac_cut_threshold99 = e.max_s1_frac_cut_threshold99;
	t.tpc_max_s1_frac_cut_exceeds99 = e.max_s1_frac_cut_exceeds99;       
	t.tpc_npulses         = e.npulses;
	t.tpc_timestamp       = t.tpc_gps_coarse*1.e+6 + t.tpc_gps_fine*20.e-3; // [us]
	t.tpc_live_time       = e.live_time;
	t.tpc_inhibit_time    = e.inhibit_time;
	t.tpc_has_s3          = e.has_s3;
	t.tpc_masa_x          = e.masa_x;
	t.tpc_masa_y          = e.masa_y;
	t.tpc_jason_x         = e.jason_x;
	t.tpc_jason_y         = e.jason_y;	
	*/

	double tpc_s1_late = (1-e.s1_total_f90)*e.s1;
	bool cut_gamma   = Cut_Gamma(tpc_s1_late,e.s1_total_f90);
	bool cut_neutron = Cut_Neutron(e.s1,e.s1_total_f90); 
	//if(!cut_gamma && !cut_neutron ) continue;
	if(cut_gamma)    t.tpc_event_type = 0;
	if(cut_neutron)  t.tpc_event_type = 1;
       
	// in slave mode 1:1 corrispondence tpc event od event (maybe just a little offset)
	const Int_t n_od_begin = (n_tpc - 10 < 0 ) ? 0 : (n_tpc - 10);
	const Int_t n_od_end = (n_tpc + 10 > tpc_events ) ? tpc_events : (n_tpc + 10);
	
	for(Int_t n_od = n_od_begin; n_od < n_od_end; n_od++){
	  od_chain->GetEntry(n_od);
	  if(d.bad_time_alignment>0){
	    cout << "failing od_time alignment" << '\n';
	    continue;
	  }

	  t.od_eventID           = d.event_id;
	  t.od_nclusters         = d.nclusters_lsv;
	  t.od_gps_fine          = d.gps_fine;
	  t.od_gps_coarse        = d.gps_pps;
	  t.od_wt_charge         = d.wt_total_charge;
	  t.od_lsv_charge        = d.lsv_total_charge;
	  t.od_timestamp         = t.od_gps_coarse*1.e+6 + t.od_gps_fine*20.e-3 - 54.*20.e-3; // [us]      
	  
	  DT = abs(t.od_timestamp - t.tpc_timestamp);
	  if (DT>0.1) continue; //select only od events in coincidence with the tpc +-100ns 

	  if(t.od_cluster_charge.size())
	    {
	      t.od_cluster_charge.clear();
	      t.od_cluster_start.clear();
	      t.od_cluster_height.clear();
	      t.od_cluster_multiplicity.clear();
	      t.od_cluster_pass_multcut.clear();
	      t.od_cluster_dtprompt.clear();
	    }
	  for(size_t j=0; j<d.clusters_lsv_charge->size(); ++j){
	    t.od_cluster_charge.push_back(d.clusters_lsv_charge->at(j));
	    t.od_cluster_start.push_back(d.clusters_lsv_start_ns->at(j)*1.e-3);// [us]
	    t.od_cluster_height.push_back(d.clusters_lsv_height->at(j));
	    t.od_cluster_multiplicity.push_back(d.clusters_lsv_max_multiplicity->at(j));
	    // t.od_cluster_width.push_back(d.clusters_lsv_width->at(j));

	    if(multiplicity_cut(d.clusters_lsv_height->at(j), d.clusters_lsv_max_multiplicity->at(j), d.clusters_lsv_charge->at(j)))
	      t.od_cluster_pass_multcut.push_back(1);
	    else t.od_cluster_pass_multcut.push_back(0);
	    t.od_cluster_dtprompt.push_back((d.clusters_lsv_start_ns->at(j) - od_coinc_wind)*1.e-3);
	  }
	  
	  if(t.od_roi_lsv_charge.size())
	    {
	      t.od_roi_lsv_id.clear();
	      t.od_roi_lsv_charge.clear();
	      t.od_roi_lsv_max_multiplicity.clear();
	    }
	  for(size_t j=0; j<d.roi_lsv_charge_vec->size(); ++j)
	    {
	      t.od_roi_lsv_id.push_back(d.roi_lsv_id_vec->at(j));
	      t.od_roi_lsv_charge.push_back(d.roi_lsv_charge_vec->at(j));
	      t.od_roi_lsv_max_multiplicity.push_back(d.roi_lsv_max_multiplicity_vec->at(j));
	    }

	  if(t.od_slider_lsv_charge.size())
	    {
	      t.od_slider_lsv_id.clear();
	      t.od_slider_lsv_charge.clear();
	      t.od_slider_lsv_time.clear();
	      t.od_slider_lsv_max_multiplicity.clear();
	    }
	  for(size_t j=0; j<d.slider_lsv_charge_vec->size(); ++j)
	    {
	      //t.od_slider_lsv_id.push_back(d.slider_lsv_id_vec->at(j));
	      t.od_slider_lsv_charge.push_back(d.slider_lsv_charge_vec->at(j));
	      t.od_slider_lsv_time.push_back(d.slider_lsv_time_ns_vec->at(j)*1.e-3); //[us]
	      t.od_slider_lsv_max_multiplicity.push_back(d.slider_lsv_max_multiplicity_vec->at(j));
	    }
	  t.od_eventMatched = true;
	  break; // found a match between TPC-OD, then break the loop on OD events
	}//end of OD Loop
      }//end of basic cuts
    DSTtree->Fill();
  }//End loop over events

  //  DSTtree->Write();
  f->Write();
  f->Close();
}  




	  


