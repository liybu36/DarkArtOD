////////////////////////////////////////////////////////////////////////
// Class:       PulseParameters
// Module Type: producer
// File:        PulseParameters_module.cc
//
// Generated at Tue Sep 17 19:12:25 2013 by Guangyong Koh using artmod
// from cetpkgsupport v1_02_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include "darkart/Products/Pulse.hh"
#include "darkart/Products/Waveform.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/Baseline.hh"
#include "darkart/Products/PulseParam.hh"
#include "darkart/Products/ProductAssns.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/ManyByChannel.hh"
#include "darkart/TpcReco/PmtHelper.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/common_algs.hh"

#include <vector>
#include <map>
#include <memory>
#include <algorithm> //min

namespace darkart {
  class PulseParameters;
}

class darkart::PulseParameters : public art::EDProducer {
public:
  explicit PulseParameters(fhicl::ParameterSet const & p);
  virtual ~PulseParameters();

  void produce(art::Event & e) override;

private:

  // Declare member data here.
  std::vector<int> skip_channels_;

  // Input tags for all modules whose output we need.
  art::InputTag ch_tag_;
  art::InputTag raw_wfm_tag_;
  art::InputTag bs_wfm_tag_;
  art::InputTag baseline_tag_;
  art::InputTag int_wfm_tag_;
  art::InputTag pulse_tag_;
  art::InputTag pmt_tag_;

  double fixed_time1_;
  double fixed_time2_;

  std::vector< std::vector<double> > pulse_roi_times_;
};


darkart::PulseParameters::PulseParameters(fhicl::ParameterSet const & p) :
// Initialize member data here.
  skip_channels_(p.get<std::vector<int> >("skip_channels", std::vector<int>())),
  ch_tag_(p.get<std::string>("ch_tag")),
  raw_wfm_tag_(p.get<std::string>("raw_wfm_tag")),
  bs_wfm_tag_(p.get<std::string>("bs_wfm_tag")),
  baseline_tag_(p.get<std::string>("baseline_tag")),
  int_wfm_tag_(p.get<std::string>("int_wfm_tag")),
  pulse_tag_(p.get<std::string>("pulse_tag")),
  pmt_tag_(p.get<std::string>("pmt_tag")),
  fixed_time1_(p.get<double>("fixed_time1")),
  fixed_time2_(p.get<double>("fixed_time2")),
  pulse_roi_times_(p.get<std::vector< std::vector<double> > >("pulse_roi_times"))
{
  // Call appropriate Produces<>() functions here.
  produces<darkart::PulseParamVec>();
  produces<darkart::PulseParamAssns>();
}

darkart::PulseParameters::~PulseParameters()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::PulseParameters::produce(art::Event & e)
{
  art::Handle <darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_tag_, chVecHandle);
  darkart::ChannelVec const& chVec(*chVecHandle);

  art::Handle<darkart::PulseVec> plsVecHandle;
  e.getByLabel(pulse_tag_, plsVecHandle);

  // Make the products, which begin empty.
  std::unique_ptr<darkart::PulseParamVec> plsParamVec(new darkart::PulseParamVec);
  std::unique_ptr<darkart::PulseParamAssns> plsParamAssns(new darkart::PulseParamAssns);


  darkart::ByChannel<darkart::Waveform> get_wfm(chVecHandle, e, raw_wfm_tag_);
  darkart::ByChannel<darkart::Baseline> get_baseline(chVecHandle, e, baseline_tag_);
  darkart::ByChannel<darkart::Waveform> get_bs_wfm(chVecHandle, e, bs_wfm_tag_);
  darkart::ByChannel<darkart::Waveform> get_int_wfm(chVecHandle, e, int_wfm_tag_);
  darkart::ManyByChannel<darkart::Pulse> get_pulse_vec(chVecHandle, e, pulse_tag_);
  darkart::PmtHelper get_pmt(e, pmt_tag_);
  
  for (auto const& chInfo : chVec)
    {
      // If told to skip this channel, then don't do anything
      if (util::skipChannel(chInfo, skip_channels_))
        continue;

      darkart::Channel::ChannelID const chID = chInfo.channelID;
      // If it is necessary, check baseline info..
//      auto const& bsData = util::getByChannelID <darkart::Baseline> (e, chVecHandle, chID, baselineFinder_tag_);
//      if (!(bsData.found_baseline) || bsData.saturated)
//        //Baseline not found
//        return;
      auto const& wfm = get_wfm(chID); //Ptr.
      auto const& baseline = get_baseline(chID); //Ptr.
      auto const& bsWfm = get_bs_wfm(chID); //Ptr.
      auto const& intWfm = get_int_wfm(chID); //Ptr.
      auto plsVec = get_pulse_vec(chID); //vector< Ptr<Pulse> >
      // sort plsVec because getter doesn't always preserve order
      std::sort(plsVec.begin(), plsVec.end());
      auto const& pmt = get_pmt(chID); 

      // Loop over all pulses in the channel.
      for(auto const& pulse : plsVec)
        {
          // pulse is a Ptr.
          
          int start_index = pulse->start_index;
          int end_index = pulse->end_index;

          // Create the PulseParam product that will be used to fill PulseParamVec.
          darkart::PulseParam plsInfo;

          plsInfo.found_peak = true;
          plsInfo.peak_index = darkart::findMinSamp(*bsWfm, start_index, end_index); // findMinSamp() is defined in common_algs.hh
          plsInfo.peak_time = bsWfm->SampleToTime(plsInfo.peak_index);
          plsInfo.peak_amplitude = -bsWfm->wave[plsInfo.peak_index]; //pulse are neg, amplitude pos
          plsInfo.integral = darkart::integral(*intWfm, start_index, end_index); // integral() is defined in common_algs.hh


          //look for the time it takes to reach X% of total integral
          //remember, integral is negative
          int samp(start_index);
          while( samp<end_index &&
                 std::abs(darkart::integral(*intWfm, start_index, samp))<
                 std::abs(plsInfo.integral)*0.05 ) samp++;
          plsInfo.t05 = bsWfm->SampleToTime(samp)-pulse->start_time;
          while( samp<end_index &&
                 std::abs(darkart::integral(*intWfm, start_index, samp))<
                 std::abs(plsInfo.integral)*0.10 ) samp++;
          plsInfo.t10 = bsWfm->SampleToTime(samp)-pulse->start_time;

          samp = end_index-1;
          while( samp>=start_index &&
                 std::abs(darkart::integral(*intWfm, start_index, samp))>
                 std::abs(plsInfo.integral)*0.95 ) samp--;
          plsInfo.t95 = bsWfm->SampleToTime(++samp)-pulse->start_time;
          while( samp>=start_index &&
                 std::abs(darkart::integral(*intWfm, start_index, samp))>
                 std::abs(plsInfo.integral)*0.90 ) samp--;
          plsInfo.t90 = bsWfm->SampleToTime(++samp)-pulse->start_time;

          //evaluate the fixed integrals
          samp = bsWfm->TimeToSample(pulse->start_time+fixed_time1_,true);
          if (samp>0)
            {
              plsInfo.fixed_int1_valid= true;
              plsInfo.fixed_int1 = darkart::integral(*intWfm, start_index, samp);
            }
          samp = bsWfm->TimeToSample(pulse->start_time+fixed_time2_,true);
          if (samp>0)
            {
              plsInfo.fixed_int2_valid= true;
              plsInfo.fixed_int2 = darkart::integral(*intWfm, start_index, samp);
            }

          //Check to see if peak is saturated
          if (wfm->wave[plsInfo.peak_index] == 0)
            {
              plsInfo.peak_saturated = true;
              int min_end_index = plsInfo.peak_index + 1;
              while (wfm->wave[min_end_index] == 0 && min_end_index < end_index)
                {
                  min_end_index++;
                }
              plsInfo.peak_index = (int) (plsInfo.peak_index + min_end_index) / 2;
            }

          plsInfo.npe = -plsInfo.integral/pmt.spe_mean;
          plsInfo.npe_fixed = -plsInfo.fixed_int1/pmt.spe_mean;

          // calculate f-parameters
          plsInfo.f_param.clear();
          plsInfo.f_param_fixed.clear();
          for (int ft = 10; ft <= 100; ft += 10) // steps in 10 ns
            {
              int fsamp = (int) (start_index + ft*chInfo.sample_rate*1.e-3); // bsWfm.sample_rate*1.e-3 is number of samples in ns
              if (fsamp >= chInfo.nsamps) // Should this be if(fsamp >= end_index) ?
                break;
              double fp = (plsInfo.integral!=0 ? darkart::integral(*intWfm, start_index, fsamp) / plsInfo.integral : 0);
              double fp_fixed = (plsInfo.fixed_int1!=0 ? darkart::integral(*intWfm, start_index, fsamp) / plsInfo.fixed_int1 : 0);
              plsInfo.f_param.push_back(fp);
              plsInfo.f_param_fixed.push_back(fp_fixed);
              if (ft == 90) {
                plsInfo.f90 = fp;
                plsInfo.f90_fixed = fp_fixed;
              }
            }
          
          // plsInfo.max_ch doesn't have much meaning at the channel level, so just store the channel ID for this pulse.
          plsInfo.max_ch = chInfo.channel_id();

          // save the baseline interpolations under this pulse.
          std::vector< std::pair<double,double> >::const_iterator it = baseline->interpolations.begin();
          for (; it!=baseline->interpolations.end(); it++) {
            if (it->first > pulse->start_time && it->first < pulse->end_time)
              plsInfo.baseline_interpolations.emplace_back(it->first, std::min(it->second, pulse->end_time));
          }

          // compute the ROIs for this pulse
          for (size_t i=0; i<pulse_roi_times_.size(); i++) {
	    if (pulse_roi_times_[i].size() != 2) continue;
            uint roi_start_samp = bsWfm->TimeToSample(pulse->start_time+pulse_roi_times_[i][0], true);
            uint roi_end_samp   = bsWfm->TimeToSample(pulse->start_time+pulse_roi_times_[i][1], true);
            double roi_int = 0;
            if (roi_end_samp == bsWfm->wave.size()-1)
              // roi goes outside waveform
              roi_int = 999;
            else
              roi_int = darkart::integral(*intWfm, roi_start_samp, roi_end_samp);
            plsInfo.pulse_rois.push_back(roi_int);
          }
          

          plsParamVec->push_back(plsInfo);
	  
          util::createAssn(*this, e,
			   plsVecHandle, *pulse,
			   *plsParamVec, plsInfo,
			   *plsParamAssns);
        }
    }


  // Put complete products into the event.
  e.put(std::move(plsParamVec));
  e.put(std::move(plsParamAssns));
}

DEFINE_ART_MODULE(darkart::PulseParameters)
