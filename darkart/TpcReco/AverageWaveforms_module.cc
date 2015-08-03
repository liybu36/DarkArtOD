////////////////////////////////////////////////////////////////////////
// Class:       AverageWaveforms
// Module Type: producer
// File:        AverageWaveforms_module.cc
//
// Generated at Thu Oct 10 17:53:56 2013 by Kirsten Randle using artmod
// from cetpkgsupport v1_02_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/Products/ProductAssns.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/EventInfo.hh"

#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/PmtHelper.hh"
#include "darkart/TpcReco/averagewaveforms_algs.hh"

#include "TFile.h"
#include "TGraphErrors.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TList.h"
#include "TH1F.h"

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm> //std::min


namespace darkart {
  class AverageWaveforms;
}

class darkart::AverageWaveforms : public art::EDProducer {
public:
  explicit AverageWaveforms(fhicl::ParameterSet const & p);
  virtual ~AverageWaveforms();

  void produce(art::Event & e) override;

  void beginRun(art::Run & r) override;
  void beginJob() override;
  void endJob() override;

private:
  art::InputTag event_info_producer_tag_;
  art::InputTag wfm_producer_tag_;
  art::InputTag ch_producer_tag_;

  
  art::ServiceHandle<art::TFileService> tfs_;
  std::map<int,TGraphErrors*> plots_;
  std::map<int,int> nevents;

  int n_channels;
  
  int bin_size;
  double avgwfm_start_time;
  double avgwfm_end_time;
  bool normalize;
  bool use_input_file;
  std::string input_file;
  std::ifstream infile;
  std::string line;
  
  // variables filled by input file
  int event_id;
  int run_id;
  double zero_point;
  
};


darkart::AverageWaveforms::AverageWaveforms(fhicl::ParameterSet const & p) :
  // Initialize member data here.
  event_info_producer_tag_(p.get<std::string>("event_info_producer_tag")),
  wfm_producer_tag_(p.get<std::string>("wfm_producer_tag")),
  ch_producer_tag_(p.get<std::string>("ch_producer_tag")),
  tfs_(),
  plots_(),
  nevents(),
  n_channels(p.get<int>("n_channels",38)),
  bin_size(p.get<int>("bin_size")),
  avgwfm_start_time(p.get<int>("avgwfm_start_time")),
  avgwfm_end_time(p.get<int>("avgwfm_end_time")),
  normalize(p.get<bool>("normalize",true)),
  use_input_file(p.get<bool>("use_input_file", false)),
  input_file(p.get<std::string>("input_file","")),
  event_id(-1),
  run_id(-1),
  zero_point(0.)
{
}

darkart::AverageWaveforms::~AverageWaveforms()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::AverageWaveforms::beginRun(art::Run & r)
{
  //Number of Pmt objects dictates number of avg wfms                                                                                                                                                       
  art::Handle<PmtVec> pmtVecHandle;
  r.getByLabel(ch_producer_tag_, pmtVecHandle);

}

void darkart::AverageWaveforms::beginJob()
{
  if (use_input_file) {
    infile.open(input_file);
    if (!infile.good())
      LOG_ERROR("FILE") << "Problem opening file "<<input_file<<std::endl;
    std::getline(infile, line);
    std::istringstream iss(line);
    iss >> run_id >> event_id >> zero_point;
  }

  for (int i=0; i<n_channels; i++) {
    TGraphErrors* gr = tfs_->make<TGraphErrors>();
    char name[25];
    sprintf(name,"average_waveform%d",i);
    gr->SetName(name);
    gr->SetTitle(name);
    plots_.insert( std::pair<int, TGraphErrors*>(i, gr));
    nevents.insert( std::pair<int, int>(i, 0) );
  }
}

void darkart::AverageWaveforms::produce(art::Event & e)
{
  art::Handle<darkart::EventInfo> evtInfo;
  e.getByLabel(event_info_producer_tag_, evtInfo);
  if (use_input_file && (evtInfo->event_id != event_id || evtInfo->run_id != run_id)) {
    LOG_ERROR("EVENTID") << "unexpected event!"<<std::endl;
    return;
  }
  if(evtInfo->event_id%100==0) std::cout << "Processing event "<<evtInfo->event_id<<std::endl;
  //Get the input channels
  art::Handle<darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_producer_tag_, chVecHandle);

  darkart::ByChannel<darkart::Waveform> get_wfm(chVecHandle, e, wfm_producer_tag_);
  darkart::PmtHelper get_pmt(e, ch_producer_tag_); //whoever makes the Channel object also makes the pmt object.

  
  for(auto const& ch : *chVecHandle){
    darkart::Channel::ChannelID const& chID = ch.channelID;

    auto const& wfm = *get_wfm(chID);
    auto const& pmt = get_pmt(chID);
    
    if (!wfm.isValid())
      continue;

    double spe_mean = pmt.spe_mean; //1;

    int gr_index = -1;
    if (n_channels == 1)
      gr_index = 0;
    else
      gr_index = ch.channel_id();
    
    TGraphErrors* gr = plots_[gr_index];

    // make sure start and end of desired window to add to average is inside wfm window
    double start_time = std::max(avgwfm_start_time+zero_point, wfm.SampleToTime(0));
    double end_time = std::min(avgwfm_end_time+zero_point, wfm.SampleToTime(wfm.wave.size()-1));
    
    if (nevents[gr_index] == 0) {
      int start_samp = wfm.TimeToSample(start_time, true);
      int end_samp = wfm.TimeToSample(end_time, true);
      int nsamps = (end_samp - start_samp)/bin_size;
      gr->Set(nsamps);

      double* x = gr->GetX();
      for (int i=0; i<nsamps; i++)
        x[i] = avgwfm_start_time + i*bin_size/ch.sample_rate;
    }

    darkart::add_waveform(wfm, gr, start_time, end_time, bin_size, spe_mean);

    nevents[gr_index]++;
  }//end loop over channels

  if (use_input_file) {
    if (!infile.is_open())
      LOG_ERROR("FILE") << "Input file "<<input_file<<" is not open."<<std::endl;
    std::getline(infile, line);
    std::istringstream iss(line);
    iss >> run_id >> event_id >> zero_point;
  }

}  
 
void darkart::AverageWaveforms::endJob()
{
  if (use_input_file)
    infile.close();

  tfs_->file().ReOpen("UPDATE");
  for (std::map<int, TGraphErrors*>::iterator it = plots_.begin(); it != plots_.end(); it++) {
    if (nevents[it->first] > 0) {
      TGraphErrors* gr = it->second;
      const int nsamps = gr->GetN();
      double* y = gr->GetY();
      double* ey = gr->GetEY();
      for (int i=0; i<nsamps; i++) {
        if (normalize) {
          y[i] /= nevents[it->first];
          ey[i] = sqrt(ey[i]) / nevents[it->first];
        }
        else
          ey[i] = sqrt(ey[i]);
      }
      gr->Write();
      std::cout << "average_waveform"<<it->first<<" has "<<nevents[it->first]<<" events."<<std::endl;
    }
  }
  
}

DEFINE_ART_MODULE(darkart::AverageWaveforms)
