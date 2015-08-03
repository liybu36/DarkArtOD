/*
  How to run it:
  set the right path for darkartod 
  source darkroot.sh
  make 
  ./SLADOD run_number (run_number)
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <map>
#include <ctime>
#include <algorithm>

#include "TFile.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TMath.h"
#include "TNtuple.h"
#include "TAxis.h"
#include "TArray.h"
#include "TRint.h"
#include "TROOT.h"

#include "../darkart/ODProducts/ODEventData.hh"
#include "../darkart/ODProducts/RunInfo.hh"

TRint *theApp;

using namespace std;
using namespace darkart;

bool VerifyDataFile(TString mainfile)
{
 ifstream NameCheck;
 NameCheck.open(mainfile.Data());
 if(!NameCheck.good())
   { NameCheck.close();
     return false;
   }
 else{
   TFile *f = new TFile(mainfile);
   if(!f->IsOpen() || f->IsZombie())
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

bool LoopOverChain(TChain* od_chain, TString outFileName){

  darkart::od::ODEventData* odeventdata = NULL;
  od_chain->SetBranchAddress("ODEventData", &odeventdata);
  int nentries = od_chain->GetEntries();
  std::cout<<"Number of Events: "<<nentries<<std::endl;
  if(nentries<1) return false;

  std::cout << "Saving output to "<<outFileName<<std::endl;
  TFile* f = new TFile(outFileName, "RECREATE");

  //Create DSTtree
  TTree* odtree = new TTree ("odtree", "tree of darkartod events");
  string darkart_version;

  //EventInfo
  int run_id;
  int subrun_id;
  int event_id;
  uint32_t gps_coarse;      // GPS Coarse Time counter
  uint32_t gps_fine;        // GPS Fine Time counter. It counts the number of clock cycles (50 MHz) in the second
  uint16_t gps_pps;         // 1PPS counter. It counts the number of seconds since the start of the run
  double gps_timestamp_sec; // GPS timestamp in seconds of the event constructected using the 3 gps counters
  double dt_usec; //time since the last event in microsecond
  uint16_t trigger_type; //trigger types. Now it is empty
  int bad_time_alignment; // 0=good alignment, 1=time missmatch between digitizers       
  
  //ChannelData
  std::vector<int> channel_data_chassis;
  std::vector<int> channel_data_board;
  std::vector<int> channel_data_channel; //specific to the board
  std::vector<int> channel_data_channel_id; //global ID
  std::vector<int> channel_data_channel_type; //0: default 1: LSV, 2: WT, -1: disabled LSV, -2: disable WT
  std::vector<bool> channel_data_is_enabled;
  std::vector<int> channel_data_npulses;
  
  std::vector< std::vector<int> > channel_data_pulses_pulse_id;
  std::vector< std::vector<int> > channel_data_pulses_start_sample;   // sample in the fADC
  std::vector< std::vector<double> > channel_data_pulses_start_ns;    // basically start_sample/sample_rate_GHz-trigger_time_ns
  std::vector< std::vector<int> > channel_data_pulses_peak_sample;    // sample corresponding to the mimimum volt waveform (maximum spe waveform)
  std::vector< std::vector<double> > channel_data_pulses_peak_ns;     // time corresponding to peak_sample; mainly used for laser runs
  std::vector< std::vector<float> > channel_data_pulses_peak_amplitude; // minimum amplitude in V (maximum in spe) of the waveform
  std::vector< std::vector<int> > channel_data_pulses_size_sample;    // size of the waveform in sample
  std::vector< std::vector<float> > channel_data_pulses_pedestal_mean; // in V
  std::vector< std::vector<float> > channel_data_pulses_pedestal_rms;  // in V
  std::vector< std::vector<float> > channel_data_pulses_integral;     // in V*s or spe
  std::vector< std::vector<float> > channel_data_pulses_offset; // average amplitude of the waveform (integral/length); used in pedestal calibration runs;Will not be changed in Saturation.
  std::vector< std::vector<float> > channel_data_pulses_saturation_correction; // same units of integral variable
  std::vector< std::vector<float> > channel_data_pulses_saturation_width;
  std::vector< std::vector<bool> > channel_data_pulses_is_saturated;

  int nclusters_lsv;
  int nclusters_wt;

  //Cluster LSV
  std::vector<int> clusters_lsv_cluster_id;
  std::vector<int> clusters_lsv_start_sample;
  std::vector<int> clusters_lsv_end_sample;
  std::vector<int> clusters_lsv_peak_sample;
  std::vector<int> clusters_lsv_max_multiplicity;
  std::vector<float> clusters_lsv_charge;
  std::vector<float> clusters_lsv_height;
  std::vector<float> clusters_lsv_start_ns;
  std::vector<float> clusters_lsv_end_ns;
  std::vector<float> clusters_lsv_peak_ns;

  std::vector<float> clusters_lsv_shape_mean_time_ns;   // first moment of the cluster pdf, weighted on amplitude
  std::vector<float> clusters_lsv_shape_sigma_mean_ns;  // second moment with respect to the mean time
  std::vector<float> clusters_lsv_shape_sigma_peak_ns;  // second moment with respect to the peak time
  std::vector<float> clusters_lsv_shape_skewness;       // second moment
  std::vector<float> clusters_lsv_shape_skewness_max;   // second moment calculated only on the main pulse without considering small pulses close to it
  std::vector<float> clusters_lsv_shape_skewness_10to90;   // second moment calculated only between the time in witch the charge reaches 10% and the time in witch the charge reaches 90%
  std::vector<float> clusters_lsv_shape_kurtosis;       // second moment
  //std::vector<float> clusters_lsv_shape_max_time_ns;
  // std::vector<float> clusters_lsv_shape_max_amplitude;
  std::vector<float> clusters_lsv_shape_a10_time_ns;        // time when the amplitude is 10% of max_amplitude
  std::vector<float> clusters_lsv_shape_a90_time_ns;        // time when the amplitude is 90% of max_amplitude
  std::vector<float> clusters_lsv_shape_rise10_time_ns;       // a90_time - a10_time
  std::vector<float> clusters_lsv_shape_rise90_time_ns;       // a90_time - a10_time
  std::vector<float> clusters_lsv_shape_above_thr_time_ns;  // time above threshold
  std::vector<float> clusters_lsv_shape_c10_time_ns;        // time when the charge is 10% of the total charge
  std::vector<float> clusters_lsv_shape_c90_time_ns;        // time when the charge is 90% of the total charge
  std::vector<float> clusters_lsv_shape_f30;         // fractional charge in first  50 ns
  std::vector<float> clusters_lsv_shape_f50;        // fractional charge in first 100 ns
  std::vector<float> clusters_lsv_shape_f60;        // fractional charge in first 200 ns
  std::vector<float> clusters_lsv_shape_tail50_to_total;   //fractional charge after the first 20 ns
  std::vector<float> clusters_lsv_shape_tail60_to_total;   //fractional charge after the first 20 ns   

  std::vector<int> clusters_lsv_pmt_max_ch;               //channel with maximum charge
  std::vector<float> clusters_lsv_pmt_max_ch_fraction;      //fraction of charge holded by the max_ch
  std::vector<float> clusters_lsv_pmt_charge_mean;      //mean charge for each pmt within the cluster
  std::vector<float> clusters_lsv_pmt_charge_variance;  //variance relative to the pmt_charge_mean       

  //Cluster WT
  std::vector<int> clusters_wt_cluster_id;
  std::vector<int> clusters_wt_start_sample;
  std::vector<int> clusters_wt_end_sample;
  std::vector<int> clusters_wt_peak_sample;
  std::vector<int> clusters_wt_max_multiplicity;
  std::vector<float> clusters_wt_charge;
  std::vector<float> clusters_wt_height;
  std::vector<float> clusters_wt_start_ns;
  std::vector<float> clusters_wt_end_ns;
  std::vector<float> clusters_wt_peak_ns;

  std::vector<float> clusters_wt_shape_mean_time_ns;   // first moment of the cluster pdf, weighted on amplitude
  std::vector<float> clusters_wt_shape_sigma_mean_ns;  // second moment with respect to the mean time
  std::vector<float> clusters_wt_shape_sigma_peak_ns;  // second moment with respect to the peak time
  std::vector<float> clusters_wt_shape_skewness;       // second moment
  std::vector<float> clusters_wt_shape_skewness_max;   // second moment calculated only on the main pulse without considering small pulses close to it
  std::vector<float> clusters_wt_shape_skewness_10to90;   // second moment calculated only between the time in witch the charge reaches 10% and the time in witch the charge reaches 90%
  std::vector<float> clusters_wt_shape_kurtosis;       // second moment
  //std::vector<float> clusters_wt_shape_max_time_ns;
  // std::vector<float> clusters_wt_shape_max_amplitude;
  std::vector<float> clusters_wt_shape_a10_time_ns;        // time when the amplitude is 10% of max_amplitude
  std::vector<float> clusters_wt_shape_a90_time_ns;        // time when the amplitude is 90% of max_amplitude
  std::vector<float> clusters_wt_shape_rise10_time_ns;       // a90_time - a10_time
  std::vector<float> clusters_wt_shape_rise90_time_ns;       // a90_time - a10_time
  std::vector<float> clusters_wt_shape_above_thr_time_ns;  // time above threshold
  std::vector<float> clusters_wt_shape_c10_time_ns;        // time when the charge is 10% of the total charge
  std::vector<float> clusters_wt_shape_c90_time_ns;        // time when the charge is 90% of the total charge
  std::vector<float> clusters_wt_shape_f30;         // fractional charge in first  50 ns
  std::vector<float> clusters_wt_shape_f50;        // fractional charge in first 100 ns
  std::vector<float> clusters_wt_shape_f60;        // fractional charge in first 200 ns
  std::vector<float> clusters_wt_shape_tail50_to_total;   //fractional charge after the first 20 ns
  std::vector<float> clusters_wt_shape_tail60_to_total;   //fractional charge after the first 20 ns   

  std::vector<int> clusters_wt_pmt_max_ch;               //channel with maximum charge
  std::vector<float> clusters_wt_pmt_max_ch_fraction;      //fraction of charge holded by the max_ch
  std::vector<float> clusters_wt_pmt_charge_mean;      //mean charge for each pmt within the cluster
  std::vector<float> clusters_wt_pmt_charge_variance;  //variance relative to the pmt_charge_mean       

  //ROI LSV
  std::vector<int> roi_lsv_id_vec;
  std::vector<float> roi_lsv_charge_vec;
  std::vector<int> roi_lsv_max_multiplicity_vec;

  //Slider LSV
  std::vector<int> slider_lsv_id_vec;
  std::vector<float> slider_lsv_charge_vec;
  std::vector<float> slider_lsv_time_ns_vec;
  std::vector<int> slider_lsv_max_multiplicity_vec;

  //ROI WT
  std::vector<int> roi_wt_id_vec;
  std::vector<float> roi_wt_charge_vec;
  std::vector<int> roi_wt_max_multiplicity_vec;

  //Slider WT
  std::vector<int> slider_wt_id_vec;
  std::vector<float> slider_wt_charge_vec;
  std::vector<float> slider_wt_time_ns_vec;
  std::vector<int> slider_wt_max_multiplicity_vec;
  
  //TotalCharge
  float lsv_total_charge;
  float wt_total_charge;
  std::vector<float> chan_charge_vec;

  odtree->Branch("darkart_version",&darkart_version);    
  odtree->Branch("run_id",&run_id,"run_id/I");    
  odtree->Branch("subrun_id",&subrun_id,"subrun_id/I");    
  odtree->Branch("event_id",&event_id,"event_id/I");                                                                    
  odtree->Branch("gps_fine",&gps_fine,"gps_fine/i");                                                                 
  odtree->Branch("gps_coarse",&gps_coarse,"gps_coarse/i");                                                           
  odtree->Branch("gps_pps",&gps_pps,"gps_pps/s");                                                                 
  odtree->Branch("gps_timestamp_sec", &gps_timestamp_sec,"gps_timestamp_sec/D"); //microsecond   
  odtree->Branch("dt_usec",&dt_usec,"dt_usec/D");      
  odtree->Branch("trigger_type",&trigger_type,"trigger_type/s");      
  odtree->Branch("bad_time_alignment",&bad_time_alignment,"bad_time_alignment/I");      

  odtree->Branch("channel_data_chassis",&channel_data_chassis,32000,1);    
  odtree->Branch("channel_data_board",&channel_data_board,32000,1);    
  odtree->Branch("channel_data_channel",&channel_data_channel,32000,1);    
  odtree->Branch("channel_data_channel_id",&channel_data_channel_id,32000,1);    
  odtree->Branch("channel_data_channel_type",&channel_data_channel_type,32000,1);    
  odtree->Branch("channel_data_is_enabled",&channel_data_is_enabled,32000,1);    
  odtree->Branch("channel_data_npulses",&channel_data_npulses,32000,1);    

  odtree->Branch("channel_data_pulses_pulse_id",&channel_data_pulses_pulse_id);
  odtree->Branch("channel_data_pulses_start_sample",&channel_data_pulses_start_sample);
  odtree->Branch("channel_data_pulses_start_ns",&channel_data_pulses_start_ns);
  odtree->Branch("channel_data_pulses_peak_sample",&channel_data_pulses_peak_sample);
  odtree->Branch("channel_data_pulses_peak_ns",&channel_data_pulses_peak_ns);
  odtree->Branch("channel_data_pulses_peak_amplitude",&channel_data_pulses_peak_amplitude);
  odtree->Branch("channel_data_pulses_size_sample",&channel_data_pulses_size_sample);
  odtree->Branch("channel_data_pulses_pedestal_mean",&channel_data_pulses_pedestal_mean);
  odtree->Branch("channel_data_pulses_pedestal_rms",&channel_data_pulses_pedestal_rms);
  odtree->Branch("channel_data_pulses_integral",&channel_data_pulses_integral);
  odtree->Branch("channel_data_pulses_saturation_correction",&channel_data_pulses_saturation_correction);
  odtree->Branch("channel_data_pulses_saturation_width",&channel_data_pulses_saturation_width);
  odtree->Branch("channel_data_pulses_is_saturated",&channel_data_pulses_is_saturated);
 
  odtree->Branch("nclusters_lsv",&nclusters_lsv,"nclusters_lsv/I");    
  odtree->Branch("nclusters_wt",&nclusters_wt,"nclusters_wt/I");    

  odtree->Branch("clusters_lsv_cluster_id",&clusters_lsv_cluster_id,32000,1);
  odtree->Branch("clusters_lsv_start_sample",&clusters_lsv_start_sample,32000,1);
  odtree->Branch("clusters_lsv_end_sample",&clusters_lsv_end_sample,32000,1);
  odtree->Branch("clusters_lsv_peak_sample",&clusters_lsv_peak_sample,32000,1);
  odtree->Branch("clusters_lsv_max_multiplicity",&clusters_lsv_max_multiplicity,32000,1);
  odtree->Branch("clusters_lsv_charge",&clusters_lsv_charge,32000,1);
  odtree->Branch("clusters_lsv_height",&clusters_lsv_height,32000,1);
  odtree->Branch("clusters_lsv_start_ns",&clusters_lsv_start_ns,32000,1);
  odtree->Branch("clusters_lsv_end_ns",&clusters_lsv_end_ns,32000,1);
  odtree->Branch("clusters_lsv_peak_ns",&clusters_lsv_peak_ns,32000,1);

  odtree->Branch("clusters_lsv_shape_mean_time_ns",&clusters_lsv_shape_mean_time_ns,32000,1);
  odtree->Branch("clusters_lsv_shape_sigma_mean_ns",&clusters_lsv_shape_sigma_mean_ns,32000,1);
  odtree->Branch("clusters_lsv_shape_sigma_peak_ns",&clusters_lsv_shape_sigma_peak_ns,32000,1);
  odtree->Branch("clusters_lsv_shape_skewness",&clusters_lsv_shape_skewness,32000,1);
  odtree->Branch("clusters_lsv_shape_skewness_max",&clusters_lsv_shape_skewness_max,32000,1);
  odtree->Branch("clusters_lsv_shape_skewness_10to90s",&clusters_lsv_shape_skewness_10to90,32000,1);
  odtree->Branch("clusters_lsv_shape_kurtosis",&clusters_lsv_shape_kurtosis,32000,1);
  odtree->Branch("clusters_lsv_shape_a10_time_ns",&clusters_lsv_shape_a10_time_ns,32000,1);
  odtree->Branch("clusters_lsv_shape_a90_time_ns",&clusters_lsv_shape_a90_time_ns,32000,1);
  odtree->Branch("clusters_lsv_shape_rise10_time_ns",&clusters_lsv_shape_rise10_time_ns,32000,1);
  odtree->Branch("clusters_lsv_shape_rise90_time_ns",&clusters_lsv_shape_rise90_time_ns,32000,1);
  odtree->Branch("clusters_lsv_shape_above_thr_time_ns",&clusters_lsv_shape_above_thr_time_ns,32000,1);
  odtree->Branch("clusters_lsv_shape_c10_time_ns",&clusters_lsv_shape_c10_time_ns,32000,1);
  odtree->Branch("clusters_lsv_shape_c90_time_ns",&clusters_lsv_shape_c90_time_ns,32000,1);
  odtree->Branch("clusters_lsv_shape_f30",&clusters_lsv_shape_f30,32000,1);
  odtree->Branch("clusters_lsv_shape_f50",&clusters_lsv_shape_f50,32000,1);
  odtree->Branch("clusters_lsv_shape_f60",&clusters_lsv_shape_f60,32000,1);
  odtree->Branch("clusters_lsv_shape_tail50_to_total",&clusters_lsv_shape_tail50_to_total,32000,1);
  odtree->Branch("clusters_lsv_shape_tail60_to_total",&clusters_lsv_shape_tail60_to_total,32000,1);

  odtree->Branch("clusters_lsv_pmt_max_ch",&clusters_lsv_pmt_max_ch,32000,1);
  odtree->Branch("clusters_lsv_pmt_max_ch_fraction",&clusters_lsv_pmt_max_ch_fraction,32000,1);
  odtree->Branch("clusters_lsv_pmt_charge_mean",&clusters_lsv_pmt_charge_mean,32000,1);
  odtree->Branch("clusters_lsv_pmt_charge_variance",&clusters_lsv_pmt_charge_variance,32000,1);

  odtree->Branch("clusters_wt_cluster_id",&clusters_wt_cluster_id,32000,1);
  odtree->Branch("clusters_wt_start_sample",&clusters_wt_start_sample,32000,1);
  odtree->Branch("clusters_wt_end_sample",&clusters_wt_end_sample,32000,1);
  odtree->Branch("clusters_wt_peak_sample",&clusters_wt_peak_sample,32000,1);
  odtree->Branch("clusters_wt_max_multiplicity",&clusters_wt_max_multiplicity,32000,1);
  odtree->Branch("clusters_wt_charge",&clusters_wt_charge,32000,1);
  odtree->Branch("clusters_wt_height",&clusters_wt_height,32000,1);
  odtree->Branch("clusters_wt_start_ns",&clusters_wt_start_ns,32000,1);
  odtree->Branch("clusters_wt_end_ns",&clusters_wt_end_ns,32000,1);
  odtree->Branch("clusters_wt_peak_ns",&clusters_wt_peak_ns,32000,1);

  odtree->Branch("clusters_wt_shape_mean_time_ns",&clusters_wt_shape_mean_time_ns,32000,1);
  odtree->Branch("clusters_wt_shape_sigma_mean_ns",&clusters_wt_shape_sigma_mean_ns,32000,1);
  odtree->Branch("clusters_wt_shape_sigma_peak_ns",&clusters_wt_shape_sigma_peak_ns,32000,1);
  odtree->Branch("clusters_wt_shape_skewness",&clusters_wt_shape_skewness,32000,1);
  odtree->Branch("clusters_wt_shape_skewness_max",&clusters_wt_shape_skewness_max,32000,1);
  odtree->Branch("clusters_wt_shape_skewness_10to90s",&clusters_wt_shape_skewness_10to90,32000,1);
  odtree->Branch("clusters_wt_shape_kurtosis",&clusters_wt_shape_kurtosis,32000,1);
  odtree->Branch("clusters_wt_shape_a10_time_ns",&clusters_wt_shape_a10_time_ns,32000,1);
  odtree->Branch("clusters_wt_shape_a90_time_ns",&clusters_wt_shape_a90_time_ns,32000,1);
  odtree->Branch("clusters_wt_shape_rise10_time_ns",&clusters_wt_shape_rise10_time_ns,32000,1);
  odtree->Branch("clusters_wt_shape_rise90_time_ns",&clusters_wt_shape_rise90_time_ns,32000,1);
  odtree->Branch("clusters_wt_shape_above_thr_time_ns",&clusters_wt_shape_above_thr_time_ns,32000,1);
  odtree->Branch("clusters_wt_shape_c10_time_ns",&clusters_wt_shape_c10_time_ns,32000,1);
  odtree->Branch("clusters_wt_shape_c90_time_ns",&clusters_wt_shape_c90_time_ns,32000,1);
  odtree->Branch("clusters_wt_shape_f30",&clusters_wt_shape_f30,32000,1);
  odtree->Branch("clusters_wt_shape_f50",&clusters_wt_shape_f50,32000,1);
  odtree->Branch("clusters_wt_shape_f60",&clusters_wt_shape_f60,32000,1);
  odtree->Branch("clusters_wt_shape_tail50_to_total",&clusters_wt_shape_tail50_to_total,32000,1);
  odtree->Branch("clusters_wt_shape_tail60_to_total",&clusters_wt_shape_tail60_to_total,32000,1);

  odtree->Branch("clusters_wt_pmt_max_ch",&clusters_wt_pmt_max_ch,32000,1);
  odtree->Branch("clusters_wt_pmt_max_ch_fraction",&clusters_wt_pmt_max_ch_fraction,32000,1);
  odtree->Branch("clusters_wt_pmt_charge_mean",&clusters_wt_pmt_charge_mean,32000,1);
  odtree->Branch("clusters_wt_pmt_charge_variance",&clusters_wt_pmt_charge_variance,32000,1);

  odtree->Branch("roi_lsv_id_vec",&roi_lsv_id_vec,32000,1);
  odtree->Branch("roi_lsv_charge_vec",&roi_lsv_charge_vec,32000,1);
  odtree->Branch("roi_lsv_max_multiplicity_vec",&roi_lsv_max_multiplicity_vec,32000,1);

  odtree->Branch("slider_lsv_id_vec",&slider_lsv_id_vec,32000,1);
  odtree->Branch("slider_lsv_charge_vec",&slider_lsv_charge_vec,32000,1);
  odtree->Branch("slider_lsv_time_ns_vec",&slider_lsv_time_ns_vec,32000,1);
  odtree->Branch("slider_lsv_max_multiplicity_vec",&slider_lsv_max_multiplicity_vec,32000,1);
   
  odtree->Branch("roi_wt_id_vec",&roi_wt_id_vec,32000,1);
  odtree->Branch("roi_wt_charge_vec",&roi_wt_charge_vec,32000,1);
  odtree->Branch("roi_wt_max_multiplicity_vec",&roi_wt_max_multiplicity_vec,32000,1);

  odtree->Branch("slider_wt_id_vec",&slider_wt_id_vec,32000,1);
  odtree->Branch("slider_wt_charge_vec",&slider_wt_charge_vec,32000,1);
  odtree->Branch("slider_wt_time_ns_vec",&slider_wt_time_ns_vec,32000,1);
  odtree->Branch("slider_wt_max_multiplicity_vec",&slider_wt_max_multiplicity_vec,32000,1);
  
  odtree->Branch("lsv_total_charge",&lsv_total_charge,"lsv_total_charge/F");    
  odtree->Branch("wt_total_charge",&wt_total_charge,"wt_total_charge/F");    
  odtree->Branch("chan_charge_vec",&chan_charge_vec,32000,1);    
  
  //////////////////////////////////////////////////////////////////////////
  /////////////////     BEGIN LOOP OVER EVENTS       ///////////////////////
  //////////////////////////////////////////////////////////////////////////

  for (Int_t i = 0; i < nentries; i++){
    od_chain->GetEntry(i);

    if(!(i%1000))
      std::cout<<"Processing Event "<<i<<", "
	       <<Int_t(100*i/nentries)<<"% Completed"<<std::endl;

    darkart_version = odeventdata->darkart_version;
    run_id = odeventdata->event_info.run_id;
    subrun_id = odeventdata->event_info.subrun_id;
    event_id = odeventdata->event_info.event_id;
    gps_coarse = odeventdata->event_info.gps_coarse;
    gps_fine = odeventdata->event_info.gps_fine;
    gps_pps = odeventdata->event_info.gps_pps;
    gps_timestamp_sec = odeventdata->event_info.gps_timestamp_sec;
    dt_usec = odeventdata->event_info.dt_usec;
    trigger_type = odeventdata->event_info.trigger_type;
    bad_time_alignment = odeventdata->event_info.bad_time_alignment;

    if(channel_data_chassis.size())
      {
	channel_data_chassis.clear();
	channel_data_board.clear();
	channel_data_channel.clear();
	channel_data_channel_id.clear();	
	channel_data_is_enabled.clear();
	channel_data_npulses.clear();
      }

    if(channel_data_pulses_pulse_id.size())
      {	
	channel_data_pulses_pulse_id.clear();
	channel_data_pulses_start_sample.clear();
	channel_data_pulses_start_ns.clear();
	channel_data_pulses_peak_sample.clear();
	channel_data_pulses_peak_ns.clear();
	channel_data_pulses_peak_amplitude.clear();
	channel_data_pulses_size_sample.clear();
	channel_data_pulses_pedestal_mean.clear();
	channel_data_pulses_pedestal_rms.clear();
	channel_data_pulses_integral.clear();
	channel_data_pulses_offset.clear();
	channel_data_pulses_saturation_correction.clear();
	channel_data_pulses_saturation_width.clear();
	channel_data_pulses_is_saturated.clear();	
      }

    for(size_t j=0; j<odeventdata->channel_data.size(); j++)
      {
	channel_data_chassis.push_back(odeventdata->channel_data.at(j).chassis);
	channel_data_board.push_back(odeventdata->channel_data.at(j).board);
	channel_data_channel.push_back(odeventdata->channel_data.at(j).channel);
	channel_data_channel_id.push_back(odeventdata->channel_data.at(j).channel_id);
	channel_data_channel_type.push_back(odeventdata->channel_data.at(j).channel_type);
	channel_data_is_enabled.push_back(odeventdata->channel_data.at(j).is_enabled);
	channel_data_npulses.push_back(odeventdata->channel_data.at(j).npulses);

	std::vector<int> _pulse_id;
	std::vector<int> _start_sample;  
	std::vector<double> _start_ns;   
	std::vector<int> _peak_sample;   
	std::vector<double> _peak_ns;    
	std::vector<float> _peak_amplitude;
	std::vector<int> _size_sample;   
	std::vector<float> _pedestal_mean;
	std::vector<float> _pedestal_rms; 
	std::vector<float> _integral;     
	std::vector<float> _offset;      
	std::vector<float> _saturation_correction;
	std::vector<float> _saturation_width;
	std::vector<bool> _is_saturated;

	for(size_t k=0; k<odeventdata->channel_data.at(j).pulses.size(); k++)
	  {
	    _pulse_id.push_back(odeventdata->channel_data.at(j).pulses.at(k).pulse_id);
	    _start_sample.push_back(odeventdata->channel_data.at(j).pulses.at(k).start_sample);
	    _start_ns.push_back(odeventdata->channel_data.at(j).pulses.at(k).start_ns);
	    _peak_sample.push_back(odeventdata->channel_data.at(j).pulses.at(k).peak_sample);
	    _peak_ns.push_back(odeventdata->channel_data.at(j).pulses.at(k).peak_ns);
	    _peak_amplitude.push_back(odeventdata->channel_data.at(j).pulses.at(k).peak_amplitude);
	    _size_sample.push_back(odeventdata->channel_data.at(j).pulses.at(k).size_sample);
	    _pedestal_mean.push_back(odeventdata->channel_data.at(j).pulses.at(k).pedestal_mean);
	    _pedestal_rms.push_back(odeventdata->channel_data.at(j).pulses.at(k).pedestal_rms);
	    _integral.push_back(odeventdata->channel_data.at(j).pulses.at(k).integral);
	    _offset.push_back(odeventdata->channel_data.at(j).pulses.at(k).offset);
	    _saturation_correction.push_back(odeventdata->channel_data.at(j).pulses.at(k).saturation_correction);
	    _saturation_width.push_back(odeventdata->channel_data.at(j).pulses.at(k).saturation_width);
	    _is_saturated.push_back(odeventdata->channel_data.at(j).pulses.at(k).is_saturated);	    
	  }

	channel_data_pulses_pulse_id.push_back(_pulse_id);
	channel_data_pulses_start_sample.push_back(_start_sample);
	channel_data_pulses_start_ns.push_back(_start_ns);
	channel_data_pulses_peak_sample.push_back(_peak_sample);
	channel_data_pulses_peak_ns.push_back(_peak_ns);
	channel_data_pulses_peak_amplitude.push_back(_peak_amplitude);
	channel_data_pulses_size_sample.push_back(_size_sample);
	channel_data_pulses_pedestal_mean.push_back(_pedestal_mean);
	channel_data_pulses_pedestal_rms.push_back(_pedestal_rms);
	channel_data_pulses_integral.push_back(_integral);
	channel_data_pulses_offset.push_back(_offset);
	channel_data_pulses_saturation_correction.push_back(_saturation_correction);
	channel_data_pulses_saturation_width.push_back(_saturation_width);
	channel_data_pulses_is_saturated.push_back(_is_saturated);	
	
      }

    nclusters_lsv = odeventdata->nclusters_lsv;
    nclusters_wt = odeventdata->nclusters_wt;

    if(clusters_lsv_cluster_id.size())
      {
	clusters_lsv_cluster_id.clear();
	clusters_lsv_start_sample.clear();
	clusters_lsv_end_sample.clear();
	clusters_lsv_peak_sample.clear();
	clusters_lsv_max_multiplicity.clear();
	clusters_lsv_charge.clear();
	clusters_lsv_height.clear();
	clusters_lsv_start_ns.clear();
	clusters_lsv_end_ns.clear();
	clusters_lsv_peak_ns.clear();

	clusters_lsv_shape_mean_time_ns.clear();
	clusters_lsv_shape_sigma_mean_ns.clear();
	clusters_lsv_shape_sigma_peak_ns.clear();
	clusters_lsv_shape_skewness.clear();
	clusters_lsv_shape_skewness_max.clear();
	clusters_lsv_shape_skewness_10to90.clear();
	clusters_lsv_shape_kurtosis.clear();
	clusters_lsv_shape_a10_time_ns.clear();
	clusters_lsv_shape_a90_time_ns.clear();
	clusters_lsv_shape_rise10_time_ns.clear();  
	clusters_lsv_shape_rise90_time_ns.clear();
	clusters_lsv_shape_above_thr_time_ns.clear();
	clusters_lsv_shape_c10_time_ns.clear();
	clusters_lsv_shape_c90_time_ns.clear();
	clusters_lsv_shape_f30.clear();
	clusters_lsv_shape_f50.clear();
	clusters_lsv_shape_f60.clear();  
	clusters_lsv_shape_tail50_to_total.clear();
	clusters_lsv_shape_tail60_to_total.clear();

	clusters_lsv_pmt_max_ch.clear();
	clusters_lsv_pmt_max_ch_fraction.clear();
	clusters_lsv_pmt_charge_mean.clear();
	clusters_lsv_pmt_charge_variance.clear();	
      }
    
    for(size_t j=0; j<odeventdata->clusters_lsv.size(); j++)
      {
	clusters_lsv_cluster_id.push_back(odeventdata->clusters_lsv.at(j).cluster_id);
	clusters_lsv_start_sample.push_back(odeventdata->clusters_lsv.at(j).start_sample);
	clusters_lsv_end_sample.push_back(odeventdata->clusters_lsv.at(j).end_sample);
	clusters_lsv_peak_sample.push_back(odeventdata->clusters_lsv.at(j).peak_sample);
	clusters_lsv_max_multiplicity.push_back(odeventdata->clusters_lsv.at(j).max_multiplicity);
	clusters_lsv_charge.push_back(odeventdata->clusters_lsv.at(j).charge);
	clusters_lsv_height.push_back(odeventdata->clusters_lsv.at(j).height);
	clusters_lsv_start_ns.push_back(odeventdata->clusters_lsv.at(j).start_ns);
	clusters_lsv_end_ns.push_back(odeventdata->clusters_lsv.at(j).end_ns);
	clusters_lsv_peak_ns.push_back(odeventdata->clusters_lsv.at(j).peak_ns);

	if(odeventdata->clusters_lsv.at(j).shape != NULL)
	  {
	    clusters_lsv_shape_mean_time_ns.push_back(odeventdata->clusters_lsv.at(j).shape->mean_time_ns);
	    clusters_lsv_shape_sigma_mean_ns.push_back(odeventdata->clusters_lsv.at(j).shape->sigma_mean_ns);
	    clusters_lsv_shape_sigma_peak_ns.push_back(odeventdata->clusters_lsv.at(j).shape->sigma_peak_ns);
	    clusters_lsv_shape_skewness.push_back(odeventdata->clusters_lsv.at(j).shape->skewness);
	    clusters_lsv_shape_skewness_max.push_back(odeventdata->clusters_lsv.at(j).shape->skewness_max);
	    clusters_lsv_shape_skewness_10to90.push_back(odeventdata->clusters_lsv.at(j).shape->skewness_10to90);
	    clusters_lsv_shape_kurtosis.push_back(odeventdata->clusters_lsv.at(j).shape->kurtosis);
	    clusters_lsv_shape_a10_time_ns.push_back(odeventdata->clusters_lsv.at(j).shape->a10_time_ns);
	    clusters_lsv_shape_a90_time_ns.push_back(odeventdata->clusters_lsv.at(j).shape->a90_time_ns);
	    clusters_lsv_shape_rise10_time_ns.push_back(odeventdata->clusters_lsv.at(j).shape->rise10_time_ns);  
	    clusters_lsv_shape_rise90_time_ns.push_back(odeventdata->clusters_lsv.at(j).shape->rise90_time_ns);
	    clusters_lsv_shape_above_thr_time_ns.push_back(odeventdata->clusters_lsv.at(j).shape->above_thr_time_ns);
	    clusters_lsv_shape_c10_time_ns.push_back(odeventdata->clusters_lsv.at(j).shape->c10_time_ns);
	    clusters_lsv_shape_c90_time_ns.push_back(odeventdata->clusters_lsv.at(j).shape->c90_time_ns);
	    clusters_lsv_shape_f30.push_back(odeventdata->clusters_lsv.at(j).shape->f30);
	    clusters_lsv_shape_f50.push_back(odeventdata->clusters_lsv.at(j).shape->f50);
	    clusters_lsv_shape_f60.push_back(odeventdata->clusters_lsv.at(j).shape->f60);  
	    clusters_lsv_shape_tail50_to_total.push_back(odeventdata->clusters_lsv.at(j).shape->tail50_to_total);
	    clusters_lsv_shape_tail60_to_total.push_back(odeventdata->clusters_lsv.at(j).shape->tail60_to_total);
	  }
	if(odeventdata->clusters_lsv.at(j).pmt_distr != NULL)
	  {
	    clusters_lsv_pmt_max_ch.push_back(odeventdata->clusters_lsv.at(j).pmt_distr->max_ch);
	    clusters_lsv_pmt_max_ch_fraction.push_back(odeventdata->clusters_lsv.at(j).pmt_distr->max_ch_fraction);
	    clusters_lsv_pmt_charge_mean.push_back(odeventdata->clusters_lsv.at(j).pmt_distr->pmt_charge_mean);
	    clusters_lsv_pmt_charge_variance.push_back(odeventdata->clusters_lsv.at(j).pmt_distr->pmt_charge_variance);
	  }
      }

    if(clusters_wt_cluster_id.size())
      {
	clusters_wt_cluster_id.clear();
	clusters_wt_start_sample.clear();
	clusters_wt_end_sample.clear();
	clusters_wt_peak_sample.clear();
	clusters_wt_max_multiplicity.clear();
	clusters_wt_charge.clear();
	clusters_wt_height.clear();
	clusters_wt_start_ns.clear();
	clusters_wt_end_ns.clear();
	clusters_wt_peak_ns.clear();

	clusters_wt_shape_mean_time_ns.clear();
	clusters_wt_shape_sigma_mean_ns.clear();
	clusters_wt_shape_sigma_peak_ns.clear();
	clusters_wt_shape_skewness.clear();
	clusters_wt_shape_skewness_max.clear();
	clusters_wt_shape_skewness_10to90.clear();
	clusters_wt_shape_kurtosis.clear();
	clusters_wt_shape_a10_time_ns.clear();
	clusters_wt_shape_a90_time_ns.clear();
	clusters_wt_shape_rise10_time_ns.clear();  
	clusters_wt_shape_rise90_time_ns.clear();
	clusters_wt_shape_above_thr_time_ns.clear();
	clusters_wt_shape_c10_time_ns.clear();
	clusters_wt_shape_c90_time_ns.clear();
	clusters_wt_shape_f30.clear();
	clusters_wt_shape_f50.clear();
	clusters_wt_shape_f60.clear();  
	clusters_wt_shape_tail50_to_total.clear();
	clusters_wt_shape_tail60_to_total.clear();

	clusters_wt_pmt_max_ch.clear();
	clusters_wt_pmt_max_ch_fraction.clear();
	clusters_wt_pmt_charge_mean.clear();
	clusters_wt_pmt_charge_variance.clear();	
      }

    for(size_t j=0; j<odeventdata->clusters_wt.size(); j++)
      {
	clusters_wt_cluster_id.push_back(odeventdata->clusters_wt.at(j).cluster_id);
	clusters_wt_start_sample.push_back(odeventdata->clusters_wt.at(j).start_sample);
	clusters_wt_end_sample.push_back(odeventdata->clusters_wt.at(j).end_sample);
	clusters_wt_peak_sample.push_back(odeventdata->clusters_wt.at(j).peak_sample);
	clusters_wt_max_multiplicity.push_back(odeventdata->clusters_wt.at(j).max_multiplicity);
	clusters_wt_charge.push_back(odeventdata->clusters_wt.at(j).charge);
	clusters_wt_height.push_back(odeventdata->clusters_wt.at(j).height);
	clusters_wt_start_ns.push_back(odeventdata->clusters_wt.at(j).start_ns);
	clusters_wt_end_ns.push_back(odeventdata->clusters_wt.at(j).end_ns);
	clusters_wt_peak_ns.push_back(odeventdata->clusters_wt.at(j).peak_ns);

	if(odeventdata->clusters_wt.at(j).shape != NULL)
	  {
	    clusters_wt_shape_mean_time_ns.push_back(odeventdata->clusters_wt.at(j).shape->mean_time_ns);
	    clusters_wt_shape_sigma_mean_ns.push_back(odeventdata->clusters_wt.at(j).shape->sigma_mean_ns);
	    clusters_wt_shape_sigma_peak_ns.push_back(odeventdata->clusters_wt.at(j).shape->sigma_peak_ns);
	    clusters_wt_shape_skewness.push_back(odeventdata->clusters_wt.at(j).shape->skewness);
	    clusters_wt_shape_skewness_max.push_back(odeventdata->clusters_wt.at(j).shape->skewness_max);
	    clusters_wt_shape_skewness_10to90.push_back(odeventdata->clusters_wt.at(j).shape->skewness_10to90);
	    clusters_wt_shape_kurtosis.push_back(odeventdata->clusters_wt.at(j).shape->kurtosis);
	    clusters_wt_shape_a10_time_ns.push_back(odeventdata->clusters_wt.at(j).shape->a10_time_ns);
	    clusters_wt_shape_a90_time_ns.push_back(odeventdata->clusters_wt.at(j).shape->a90_time_ns);
	    clusters_wt_shape_rise10_time_ns.push_back(odeventdata->clusters_wt.at(j).shape->rise10_time_ns);  
	    clusters_wt_shape_rise90_time_ns.push_back(odeventdata->clusters_wt.at(j).shape->rise90_time_ns);
	    clusters_wt_shape_above_thr_time_ns.push_back(odeventdata->clusters_wt.at(j).shape->above_thr_time_ns);
	    clusters_wt_shape_c10_time_ns.push_back(odeventdata->clusters_wt.at(j).shape->c10_time_ns);
	    clusters_wt_shape_c90_time_ns.push_back(odeventdata->clusters_wt.at(j).shape->c90_time_ns);
	    clusters_wt_shape_f30.push_back(odeventdata->clusters_wt.at(j).shape->f30);
	    clusters_wt_shape_f50.push_back(odeventdata->clusters_wt.at(j).shape->f50);
	    clusters_wt_shape_f60.push_back(odeventdata->clusters_wt.at(j).shape->f60);  
	    clusters_wt_shape_tail50_to_total.push_back(odeventdata->clusters_wt.at(j).shape->tail50_to_total);
	    clusters_wt_shape_tail60_to_total.push_back(odeventdata->clusters_wt.at(j).shape->tail60_to_total);
	  }
	if(odeventdata->clusters_wt.at(j).pmt_distr != NULL)
	  {
	    clusters_wt_pmt_max_ch.push_back(odeventdata->clusters_wt.at(j).pmt_distr->max_ch);
	    clusters_wt_pmt_max_ch_fraction.push_back(odeventdata->clusters_wt.at(j).pmt_distr->max_ch_fraction);
	    clusters_wt_pmt_charge_mean.push_back(odeventdata->clusters_wt.at(j).pmt_distr->pmt_charge_mean);
	    clusters_wt_pmt_charge_variance.push_back(odeventdata->clusters_wt.at(j).pmt_distr->pmt_charge_variance);
	  }
      }

    roi_lsv_id_vec = odeventdata->roi_lsv.id_vec;
    roi_lsv_charge_vec = odeventdata->roi_lsv.charge_vec;
    roi_lsv_max_multiplicity_vec = odeventdata->roi_lsv.max_multiplicity_vec;
    
    slider_lsv_id_vec = odeventdata->slider_lsv.id_vec;
    slider_lsv_charge_vec = odeventdata->slider_lsv.charge_vec;
    slider_lsv_time_ns_vec = odeventdata->slider_lsv.time_ns_vec;
    slider_lsv_max_multiplicity_vec = odeventdata->slider_lsv.max_multiplicity_vec;

    roi_wt_id_vec = odeventdata->roi_wt.id_vec;
    roi_wt_charge_vec = odeventdata->roi_wt.charge_vec;
    roi_wt_max_multiplicity_vec = odeventdata->roi_wt.max_multiplicity_vec;
    
    slider_wt_id_vec = odeventdata->slider_wt.id_vec;
    slider_wt_charge_vec = odeventdata->slider_wt.charge_vec;
    slider_wt_time_ns_vec = odeventdata->slider_wt.time_ns_vec;
    slider_wt_max_multiplicity_vec = odeventdata->slider_wt.max_multiplicity_vec;

    lsv_total_charge = odeventdata->total_charge.lsv_total_charge;
    wt_total_charge = odeventdata->total_charge.wt_total_charge;
    chan_charge_vec = odeventdata->total_charge.chan_charge_vec;    
     
    odtree->Fill();
  }    
  f->cd();
  f->Write();
  f->Close();

  return 1;
}

bool LoopOverRunChain(TChain* run_chain, TString outFileName){

  darkart::od::RunInfo* runinfo = NULL;
  run_chain->SetBranchAddress("RunInfo", &runinfo);
  int nentries = run_chain->GetEntries();
  std::cout<<"Number of Events: "<<nentries<<std::endl;
  if(nentries<1) return false;

  std::cout << "Saving output to "<<outFileName<<std::endl;
  TFile *f = NULL;
  //  TFile* f = new TFile(outFileName, "RECREATE");
  if(!VerifyDataFile(outFileName))
    return 0;
  else  f = new TFile(outFileName, "UPDATE");
  
  TTree* odtree_RunInfo = new TTree ("odtree_RunInfo", "tree of darkartod runinfo");
 
  int runinfo_run_id;
  int n_chassis;
  std::vector<int> n_scopes;

  std::vector<int> chHCD_channel_type; //0: default, 1: LSV, 2: WT, -1: disabled LSV, -2: disabled WT
  std::vector<double> chHCD_sample_rate;
  std::vector<int> chHCD_record_length;
  std::vector<double> chHCD_reference_pos;
  std::vector<bool> chHCD_is_enabled;
  std::vector<double> chHCD_vertical_range;
  std::vector<double> chHCD_vertical_offset;
  std::vector<double> chHCD_zs_enabled;
  std::vector<double> chHCD_zs_threshold;
  std::vector<double> chHCD_zs_minwidth;
  std::vector<double> chHCD_zs_pre;
  std::vector<double> chHCD_zs_post;
  
  bool LSVCfg_same_configuration;
  double LSVCfg_sample_rate_Hz;
  int LSVCfg_record_length;
  double LSVCfg_reference_pos;

  bool WTCfg_same_configuration;
  double WTCfg_sample_rate_Hz;
  int WTCfg_record_length;
  double WTCfg_reference_pos;
  
  //ROICfg LSV
  std::vector<int> roicfg_lsv_id_vec;
  std::vector<float> roicfg_lsv_start_ns_vec;
  std::vector<float> roicfg_lsv_end_ns_vec;
  std::vector<int> roicfg_lsv_label_vec;

  //SliderCfg LSV
  std::vector<int> slidercfg_lsv_id_vec;
  std::vector<float> slidercfg_lsv_start_ns_vec;
  std::vector<float> slidercfg_lsv_end_ns_vec;
  std::vector<float> slidercfg_lsv_width_ns_vec;
  std::vector<int> slidercfg_lsv_label_vec;

  //ROICfg WT
  std::vector<int> roicfg_wt_id_vec;
  std::vector<float> roicfg_wt_start_ns_vec;
  std::vector<float> roicfg_wt_end_ns_vec;
  std::vector<int> roicfg_wt_label_vec;

  //SliderCfg WT
  std::vector<int> slidercfg_wt_id_vec;
  std::vector<float> slidercfg_wt_start_ns_vec;
  std::vector<float> slidercfg_wt_end_ns_vec;
  std::vector<float> slidercfg_wt_width_ns_vec;
  std::vector<int> slidercfg_wt_label_vec;

  odtree_RunInfo->Branch("runinfo_run_id",&runinfo_run_id,"runinfo_run_id/I");
  odtree_RunInfo->Branch("n_chassis",&n_chassis,"n_chassis/I");
  odtree_RunInfo->Branch("n_scopes",&n_scopes,32000,1);

  odtree_RunInfo->Branch("chHCD_channel_type",&chHCD_channel_type,32000,1);
  odtree_RunInfo->Branch("chHCD_sample_rate",&chHCD_sample_rate,32000,1);
  odtree_RunInfo->Branch("chHCD_record_length",&chHCD_record_length,32000,1);
  odtree_RunInfo->Branch("chHCD_reference_pos",&chHCD_reference_pos,32000,1);
  odtree_RunInfo->Branch("chHCD_is_enabled",&chHCD_is_enabled,32000,1);
  odtree_RunInfo->Branch("chHCD_vertical_range",&chHCD_vertical_range,32000,1);
  odtree_RunInfo->Branch("chHCD_vertical_offset",&chHCD_vertical_offset,32000,1);
  odtree_RunInfo->Branch("chHCD_zs_enabled",&chHCD_zs_enabled,32000,1);
  odtree_RunInfo->Branch("chHCD_zs_threshold",&chHCD_zs_threshold,32000,1);
  odtree_RunInfo->Branch("chHCD_zs_minwidth",&chHCD_zs_minwidth,32000,1);
  odtree_RunInfo->Branch("chHCD_zs_pre",&chHCD_zs_pre,32000,1);
  odtree_RunInfo->Branch("chHCD_zs_post",&chHCD_zs_post,32000,1);

  odtree_RunInfo->Branch("LSVCfg_same_configuraton",&LSVCfg_same_configuration,"LSVCfg_same_configuration/O");
  odtree_RunInfo->Branch("LSVCfg_sample_rate_Hz",&LSVCfg_sample_rate_Hz,"LSVCfg_sample_rate_Hz/D");
  odtree_RunInfo->Branch("LSVCfg_record_length",&LSVCfg_record_length,"LSVCfg_record_length/I");
  odtree_RunInfo->Branch("LSVCfg_reference_pos",&LSVCfg_reference_pos,"LSVCfg_reference_pos/O");

  odtree_RunInfo->Branch("WTCfg_same_configuraton",&WTCfg_same_configuration,"WTCfg_same_configuration/O");
  odtree_RunInfo->Branch("WTCfg_sample_rate_Hz",&WTCfg_sample_rate_Hz,"WTCfg_sample_rate_Hz/D");
  odtree_RunInfo->Branch("WTCfg_record_length",&WTCfg_record_length,"WTCfg_record_length/I");
  odtree_RunInfo->Branch("WTCfg_reference_pos",&WTCfg_reference_pos,"WTCfg_reference_pos/O");

  odtree_RunInfo->Branch("roicfg_lsv_id_vec",&roicfg_lsv_id_vec,32000,1);
  odtree_RunInfo->Branch("roicfg_lsv_start_ns_vec",&roicfg_lsv_start_ns_vec,32000,1);
  odtree_RunInfo->Branch("roicfg_lsv_end_ns_vec",&roicfg_lsv_end_ns_vec,32000,1);
  odtree_RunInfo->Branch("roicfg_lsv_label_vec",&roicfg_lsv_label_vec,32000,1);
 
  odtree_RunInfo->Branch("slidercfg_lsv_id_vec",&slidercfg_lsv_id_vec,32000,1);
  odtree_RunInfo->Branch("slidercfg_lsv_start_ns_vec",&slidercfg_lsv_start_ns_vec,32000,1);
  odtree_RunInfo->Branch("slidercfg_lsv_end_ns_vec",&slidercfg_lsv_end_ns_vec,32000,1);
  odtree_RunInfo->Branch("slidercfg_lsv_width_ns_vec",&slidercfg_lsv_width_ns_vec,32000,1);
  odtree_RunInfo->Branch("slidercfg_lsv_label_vec",&slidercfg_lsv_label_vec,32000,1);
 
  odtree_RunInfo->Branch("roicfg_wt_id_vec",&roicfg_wt_id_vec,32000,1);
  odtree_RunInfo->Branch("roicfg_wt_start_ns_vec",&roicfg_wt_start_ns_vec,32000,1);
  odtree_RunInfo->Branch("roicfg_wt_end_ns_vec",&roicfg_wt_end_ns_vec,32000,1);
  odtree_RunInfo->Branch("roicfg_wt_label_vec",&roicfg_wt_label_vec,32000,1);
 
  odtree_RunInfo->Branch("slidercfg_wt_id_vec",&slidercfg_wt_id_vec,32000,1);
  odtree_RunInfo->Branch("slidercfg_wt_start_ns_vec",&slidercfg_wt_start_ns_vec,32000,1);
  odtree_RunInfo->Branch("slidercfg_wt_end_ns_vec",&slidercfg_wt_end_ns_vec,32000,1);
  odtree_RunInfo->Branch("slidercfg_wt_width_ns_vec",&slidercfg_wt_width_ns_vec,32000,1);
  odtree_RunInfo->Branch("slidercfg_wt_label_vec",&slidercfg_wt_label_vec,32000,1);
 
  nentries = 1;
  for (Int_t i = 0; i < nentries; i++){
    run_chain->GetEntry(i);

    if(!(i%1000))
      std::cout<<"Processing Event "<<i<<", "
	       <<Int_t(100*i/nentries)<<"% Completed"<<std::endl;

    runinfo_run_id = runinfo->run_id;
    n_chassis = runinfo->n_chassis;
    n_scopes = runinfo->n_scopes;

    if(chHCD_channel_type.size())
      {
	chHCD_sample_rate.clear();
	chHCD_record_length.clear();
	chHCD_reference_pos.clear();
	chHCD_is_enabled.clear();
        chHCD_vertical_range.clear();
	chHCD_vertical_offset.clear();
	chHCD_zs_enabled.clear();
	chHCD_zs_threshold.clear();
	chHCD_zs_minwidth.clear();
	chHCD_zs_pre.clear();
	chHCD_zs_post.clear();
      }
    
    for(size_t j=0; j<runinfo->chHCD.size(); j++)
      {	
	chHCD_channel_type.push_back(runinfo->chHCD.at(j).channel_type);
	chHCD_sample_rate.push_back(runinfo->chHCD.at(j).sample_rate);
	chHCD_record_length.push_back(runinfo->chHCD.at(j).record_length);
	chHCD_reference_pos.push_back(runinfo->chHCD.at(j).reference_pos);
	chHCD_is_enabled.push_back(runinfo->chHCD.at(j).is_enabled);
        chHCD_vertical_range.push_back(runinfo->chHCD.at(j).vertical_range);
	chHCD_vertical_offset.push_back(runinfo->chHCD.at(j).vertical_offset);
	chHCD_zs_enabled.push_back(runinfo->chHCD.at(j).zs_enabled);
	chHCD_zs_threshold.push_back(runinfo->chHCD.at(j).zs_threshold);
	chHCD_zs_minwidth.push_back(runinfo->chHCD.at(j).zs_minwidth);
	chHCD_zs_pre.push_back(runinfo->chHCD.at(j).zs_pre);
	chHCD_zs_post.push_back(runinfo->chHCD.at(j).zs_post);
      }

    LSVCfg_same_configuration = runinfo->LSVCfg.same_configuration;
    LSVCfg_sample_rate_Hz = runinfo->LSVCfg.sample_rate_Hz;
    LSVCfg_record_length = runinfo->LSVCfg.record_length;
    LSVCfg_reference_pos = runinfo->LSVCfg.reference_pos;
    
    WTCfg_same_configuration = runinfo->WTCfg.same_configuration;
    WTCfg_sample_rate_Hz = runinfo->WTCfg.sample_rate_Hz;
    WTCfg_record_length = runinfo->WTCfg.record_length;
    WTCfg_reference_pos = runinfo->WTCfg.reference_pos;
    
    roicfg_lsv_id_vec = runinfo->roicfg_lsv.id_vec;
    roicfg_lsv_start_ns_vec = runinfo->roicfg_lsv.start_ns_vec;
    roicfg_lsv_end_ns_vec = runinfo->roicfg_lsv.end_ns_vec;
    roicfg_lsv_label_vec = runinfo->roicfg_lsv.label_vec;
   
    slidercfg_lsv_id_vec = runinfo->slidercfg_lsv.id_vec;
    slidercfg_lsv_start_ns_vec = runinfo->slidercfg_lsv.start_ns_vec;
    slidercfg_lsv_end_ns_vec = runinfo->slidercfg_lsv.end_ns_vec;
    slidercfg_lsv_width_ns_vec = runinfo->slidercfg_lsv.width_ns_vec;
    slidercfg_lsv_label_vec = runinfo->slidercfg_lsv.label_vec;
   
    roicfg_wt_id_vec = runinfo->roicfg_wt.id_vec;
    roicfg_wt_start_ns_vec = runinfo->roicfg_wt.start_ns_vec;
    roicfg_wt_end_ns_vec = runinfo->roicfg_wt.end_ns_vec;
    roicfg_wt_label_vec = runinfo->roicfg_wt.label_vec;
   
    slidercfg_wt_id_vec = runinfo->slidercfg_wt.id_vec;
    slidercfg_wt_start_ns_vec = runinfo->slidercfg_wt.start_ns_vec;
    slidercfg_wt_end_ns_vec = runinfo->slidercfg_wt.end_ns_vec;
    slidercfg_wt_width_ns_vec = runinfo->slidercfg_wt.width_ns_vec;
    slidercfg_wt_label_vec = runinfo->slidercfg_wt.label_vec;

    odtree_RunInfo->Fill();
  }
  f->cd();
  f->Write();
  f->Close();

  return 1;
}

void ReadDataFile(TString mainfile,TChain* od_chain, TChain* run_chain)
{
  if(VerifyDataFile(mainfile))
    od_chain->Add(mainfile);      
  else 
    {
      TString tempfile = mainfile;
      tempfile.Remove(tempfile.Length()-5);      
      
      od_chain->Add(Form("%s_*.root",tempfile.Data()));
      run_chain->Add(Form("%s_*.root",tempfile.Data()));	            
      /*
      for(int i=0; i<30; i++)
	{
	  TString brfile;      
	  brfile.Form("%s_%03d.root",tempfile.Data(),i);
	  if(VerifyDataFile(brfile))	
	    {    
	      od_chain->Add(brfile);
	      run_chain->Add(brfile);	      
	    }
	}
      */
    }
}

void ReadRunList(vector<int>& run)
{
  //  string runlist = "50days_runlist.txt";
  string runlist = "uar_od_runlist.txt";
  ifstream indata (runlist.c_str());
  if(indata.is_open())
    {
      while(!indata.eof())
	{
	  string s1,s2;
	  // indata>>s1>>s2;
	  indata>>s1;
	  run.push_back(atoi(s1.c_str()));
	}
    }
  indata.close();

}

void SLADOD(int start, int end) {      
  //od_path and out_path may need changes 
  TString od_path  = "/ds50/data/test_processing/lsv/darkartod/v1/";
  TString out_path = "/ds50/data/test_processing/lsv/darkartod/v1/sladod/";
  // TString out_path = "/ds50/data/user/hqian36/DarkODDSTData/"; 
 
  for(int i=start; i<=end; i++)
    {
      TString mainfile;
      mainfile.Form("VetoOutput_Run%06d/ODRun%06d.root",i,i);
      mainfile.Prepend(od_path);
      TChain* od_chain = new TChain("odtree/Events");
      TChain* run_chain = new TChain("odtree/Run");
    
      //      ReadDataFile(mainfile,od_chain);
      ReadDataFile(mainfile,od_chain,run_chain);
      
      TString outFileName;
      outFileName.Form("SLADOD_Run%06d.root",i);
      outFileName.Prepend(out_path);

      /*      TString outRunFileName;
      outRunFileName.Form("DarkArtODDST_Run%06d_RunInfo.root",i);
      outRunFileName.Prepend(out_path);
      */
      if(!LoopOverChain(od_chain,outFileName))	
	continue;
      else if(!LoopOverRunChain(run_chain,outFileName))
	continue;
    }
}

#ifndef __CINT__
int main(int argc, char **argv) {
  theApp = new TRint("theApp",&argc,argv,NULL,0);
  int start, end; //the run number range is [start,end]
  if(argc == 1)
    {
      std::vector<int> run;
      ReadRunList(run);
      cout<<"run list size: "<<run.size()<<endl;
      for(size_t j=0; j<run.size(); ++j)
      // for(size_t j=0; j<10; ++j)
	{
	  start = run.at(j);
	  end   = run.at(j);
	  SLADOD(start,end);
	  //  cout<<run.at(j)<<endl;	  
	}
    }
  else if ( argc == 2 ) {
    //specify a single run
    std::cout << "\n==========> analysis <=============" << std::endl;
    start = atoi(theApp->Argv(1));
    end = start;
    SLADOD(start,end);
  } else if ( argc == 3 ) {
    //process a bunch of runs
    std::cout << "\n==========> analysis <=============" << std::endl;
    start = atoi(theApp->Argv(1));
    end = atoi(theApp->Argv(2));
    SLADOD(start,end);
  } else {
    std::cout << "Usage:" <<argc<< std::endl;
    std::cout << "./DarkArtODDST run_number" << std::endl;
  }

  std::cout << "==> Application finished." << std::endl;
  return 1;
}
#endif /* __CINT __ */

