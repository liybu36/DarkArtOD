#ifndef SladDSTClass_H
#define SladDSTClass_H

#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TString.h"
#include "TRint.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMath.h"

using namespace std;

struct TPCEvent
{
  //This is the TPC EventStructure from SLAD
  // variables from main file
TPCEvent():
  run_id(0),event_id(0),gps_coarse(0),gps_fine(0),live_time(0),inhibit_time(0),
    nchannel(0),baseline_not_found(0),npulses(0),has_s3(0),tdrift(0),s1(0),
    s1_start_time(0),s1_total_f90(0),s2(0),s2_total_f90(0),masa_x(0),masa_y(0),
    jason_x(0),jason_y(0),acqui_window(0),total_s1_top(0),total_s1_bottom(0),
    is_saturated_pulse0(0),is_saturated_pulse1(0),s1_end_time(0),s2_start_time(0),
    s2_end_time(0),s1_total_f90_spe_mean(0),s1_max_chan(0),s1_max_frac(0),
    max_s1_frac_cut_threshold99(0),max_s1_frac_cut_exceeds99(0),total_s2_top(0),
    s2_total_f90_spe_mean(0),s2_max_chan(0),s2_max_frac(0),total_s2_bottom(0)
  {}

  int    run_id;
  int    event_id;
  int    gps_coarse;
  int    gps_fine;
  double live_time;
  double inhibit_time;
  float  acqui_window;//[us]
  int    nchannel;
  short  baseline_not_found;
  int    npulses;
  int    has_s3;
  double tdrift;
  float  s1;
  float  total_s1_top;
  float  total_s1_bottom;
  int    is_saturated_pulse0;
  float  s1_start_time;
  float  s1_end_time;
  float  s1_total_f90;
  float  s1_total_f90_spe_mean;
  int    s1_max_chan;
  float  s1_max_frac;
  float  max_s1_frac_cut_threshold99;
  int    max_s1_frac_cut_exceeds99;

  float  s2;
  float  total_s2_top;
  float  total_s2_bottom;
  int    is_saturated_pulse1;
  float  s2_start_time;
  float  s2_end_time;
  float  s2_total_f90;
  float  s2_total_f90_spe_mean;
  int    s2_max_chan;
  float  s2_max_frac;
  // variables from s2 file
  float  s2_ch_frac[38];

  // variables from xy file
  float  masa_x;
  float  masa_y;
  float  jason_x;
  float  jason_y;

};

struct ODEvent{
  //This is the Veto EventStructure from SLADOD, not DarkArtOD
ODEvent():
  run_id(0),event_id(0),gps_fine(0),gps_coarse(0),gps_pps(0),gps_timestamp_sec(0),
    dt_usec(0),trigger_type(0),bad_time_alignment(0),nclusters_lsv(0),
    clusters_lsv_max_multiplicity(0),clusters_lsv_charge(0),clusters_lsv_height(0),
    clusters_lsv_start_ns(0),lsv_total_charge(0),wt_total_charge(0),
    roi_lsv_id_vec(0),roi_lsv_charge_vec(0),roi_lsv_max_multiplicity_vec(0),
    slider_lsv_id_vec(0),slider_lsv_charge_vec(0),slider_lsv_time_ns_vec(0),slider_lsv_max_multiplicity_vec(0)
  {}
  Int_t           run_id;
  Int_t           event_id;
  UInt_t          gps_fine;
  UInt_t          gps_coarse;
  UShort_t        gps_pps;
  Double_t        gps_timestamp_sec;
  Double_t        dt_usec;
  UShort_t        trigger_type;
  Int_t           bad_time_alignment;
  Int_t           nclusters_lsv;

  std::vector<int>     *clusters_lsv_max_multiplicity;
  std::vector<float>   *clusters_lsv_charge;
  std::vector<float>   *clusters_lsv_height;
  std::vector<float>   *clusters_lsv_start_ns;

  Float_t         lsv_total_charge;
  Float_t         wt_total_charge;

  //ROI LSV
  std::vector<int>    *roi_lsv_id_vec;
  std::vector<float>  *roi_lsv_charge_vec;
  std::vector<int>    *roi_lsv_max_multiplicity_vec;

  //Slider LSV
  std::vector<int>    *slider_lsv_id_vec;
  std::vector<float>  *slider_lsv_charge_vec;
  std::vector<float>  *slider_lsv_time_ns_vec;
  std::vector<int>    *slider_lsv_max_multiplicity_vec;

};

struct DSTEvent{
DSTEvent():
  runID(0),tpc_eventID(0),tpc_timestamp(0),tpc_event_type(-1),
  /* tpc_gps_fine(0),tpc_gps_coarse(0),tpc_s1_start_time(0),
    tpc_total_s1(0),tpc_total_s1_f90(0),tpc_total_s2(0),tpc_total_s2_f90(0),tpc_t_drift(0),tpc_s1_late(0),
    tpc_npulses(0),tpc_live_time(0),tpc_inhibit_time(0),tpc_has_s3(0),
    tpc_masa_x(0),tpc_masa_y(0),tpc_jason_x(0),tpc_jason_y(0),tpc_acqui_window(0),tpc_total_s1_top(0),
    tpc_total_s1_bottom(0),tpc_is_saturated_pulse0(0),tpc_s1_end_time(0),tpc_s1_total_f90_spe_mean(0),
    tpc_s2_total_f90_spe_mean(0),tpc_s1_max_chan(0),tpc_s1_max_frac(0),tpc_max_s1_frac_cut_threshold99(0),
    tpc_max_s1_frac_cut_exceeds99(0),tpc_s2_start_time(0),tpc_s2_end_time(0),
    tpc_total_s2_top(0),tpc_total_s2_bottom(0),tpc_is_saturated_pulse1(0),tpc_s2_max_chan(0),tpc_s2_max_frac(0),
  */
    od_eventMatched(0),od_eventID(0),od_nclusters(0),od_gps_fine(0),od_gps_coarse(0),od_timestamp(0),od_wt_charge(0),od_lsv_charge(0)
  {}
  
  //***************************SLAD DSTtree TPC Variables************************
  Int_t    runID;
  Int_t    tpc_eventID;
  Double_t tpc_timestamp;  //[us]
  Int_t    tpc_event_type; // 0 gamma, 1 neutron, -1 else
  /*
  Double_t tpc_gps_fine; //clock cycles
  Double_t tpc_gps_coarse; //[s]
  Double_t tpc_live_time;
  Double_t tpc_inhibit_time;
  int      tpc_has_s3;
  Double_t tpc_t_drift; //[us]
  Double_t tpc_s1_late; //[PE]
  Double_t tpc_acqui_window;
  Double_t tpc_s1_start_time; //[us]
  Double_t tpc_total_s1; //[PE]
  Double_t tpc_total_s1_top;
  Double_t tpc_total_s1_bottom;
  int      tpc_is_saturated_pulse0;
  Double_t tpc_s1_end_time;
  Double_t tpc_total_s1_f90;
  Double_t tpc_s1_total_f90_spe_mean;
  int      tpc_s1_max_chan;
  Double_t tpc_s1_max_frac;
  Double_t tpc_max_s1_frac_cut_threshold99;
  int      tpc_max_s1_frac_cut_exceeds99;

  Double_t tpc_total_s2; //[PE]
  Double_t tpc_s2_start_time;
  Double_t tpc_s2_end_time;  
  Double_t tpc_total_s2_f90;
  Double_t tpc_s2_total_f90_spe_mean;
  Double_t tpc_total_s2_top;
  Double_t tpc_total_s2_bottom;
  int      tpc_is_saturated_pulse1;
  int      tpc_s2_max_chan;
  Double_t tpc_s2_max_frac;

  Int_t    tpc_npulses;
  float    tpc_masa_x; 
  float    tpc_masa_y;
  float    tpc_jason_x;
  float    tpc_jason_y;
  */
  //*************************SLAD DSTtree Veto Variables******************************
  Bool_t od_eventMatched;
  Int_t od_eventID;
  Int_t od_nclusters;
  Double_t od_gps_fine;
  Double_t od_gps_coarse;
  Double_t od_timestamp;
  Double_t od_wt_charge;
  Double_t od_lsv_charge;

  std::vector<Double_t> od_cluster_charge;
  std::vector<Double_t> od_cluster_start;
  std::vector<Double_t> od_cluster_height;
  std::vector<Double_t> od_cluster_multiplicity;
  std::vector<Int_t>    od_cluster_pass_multcut;
  std::vector<Double_t> od_cluster_dtprompt;
  //  std::vector<Double_t> od_cluster_width;

  //ROI LSV
  std::vector<int>    od_roi_lsv_id;
  std::vector<float>  od_roi_lsv_charge;
  std::vector<int>    od_roi_lsv_max_multiplicity;

  //Slider LSV
  std::vector<int>    od_slider_lsv_id;
  std::vector<float>  od_slider_lsv_charge;
  std::vector<float>  od_slider_lsv_time;
  std::vector<int>    od_slider_lsv_max_multiplicity;

};

class SladDSTClass:public TObject{
 public:
  TPCEvent e;
  ODEvent d;
  DSTEvent t;
  
 SladDSTClass(): 
  tpc_path(""),od_path(""),out_path(""),out_file(""),
    tpc_chain(0),s2_fraction(0),masaxy(0),jasonxy(0),
    od_chain(0),DSTtree(0)
      { }
  virtual ~SladDSTClass() { }
  
  virtual void Init();
  virtual bool add_tpctree(int);
  virtual bool add_odtree(int);
  virtual bool load_tpctree(TChain* , TPCEvent& e);
  virtual bool load_odtree(TChain* , ODEvent& d);
  void create_tpcchain();
  void create_odchain();
  void add_tpcfriend();

  bool multiplicity_cut(Float_t,Float_t,Float_t);
  void SetOutTPCTree();
  virtual void SetOutODTree();
  void PreLoop();
  virtual bool LoopOverChain();
  bool VerifyDataFile(TString);
  void ReadRunList(vector<int>&);

  void SetTPCPath(TString var){ tpc_path = var; }
  TString GetTPCPath() { return tpc_path; }
  void SetODPath(TString var) { od_path = var; }
  TString GetODPath() { return od_path; }
  void SetOutPath(TString var) { out_path = var; }
  TString GetOutPath() { return out_path; }
  void SetOutFile(TString var) { out_file = var; }
  TString GetOutFile() { return out_file; }
  
  //--------------------------------------------------------------------------------------------------------
  // Define cuts. These definitions are taken from the 2014 analysis memo v11.
  bool CX1(TPCEvent const& e) { return e.nchannel==38 ; }
  bool CX2(TPCEvent const& e) { return e.baseline_not_found == false; }
  bool CX3(TPCEvent const& e) { return (e.live_time+e.inhibit_time)>=1.35e-3; }
  bool CX4(TPCEvent const& e) { return e.live_time>0.4e-3; }// < 1.; }
  bool CX8(TPCEvent const& e) { return e.npulses==2 || (e.npulses==3 && e.has_s3); }
  bool CX9(TPCEvent const& e) { return
      ((e.run_id >= -999 && e.run_id < 7344   && e.s1_start_time >= -0.25 && e.s1_start_time <= -0.15) ||
       (e.run_id >= 7344 && e.run_id < 7641   && e.s1_start_time >= -4.10 && e.s1_start_time <= -4.00) ||
       (e.run_id >= 7641 && e.run_id < 999999 && e.s1_start_time >= -6.10 && e.s1_start_time <= -6.00)); }
  bool Cut_Gamma(double,double);
  bool Cut_Neutron(double,double);
  
  //---------------------------------------------------------------------------------------------------------
 protected:
  double tpc_s1_late_min, tpc_s1_late_max;
  double tpc_s1_min, tpc_s1_max;
  double tpc_total_f90Nmin, tpc_total_f90Nmax;
  double tpc_total_f90Gmin, tpc_total_f90Gmax;
  double od_coinc_wind;

 private:
  TString tpc_path;
  TString od_path;
  TString out_path;
  TString out_file; 

  TTree  *DSTtree;
  TFile  *f;
  TChain *tpc_chain, *s2_fraction, *masaxy, *jasonxy;
  TChain *od_chain;

  ClassDef(SladDSTClass,0);
};

#endif
