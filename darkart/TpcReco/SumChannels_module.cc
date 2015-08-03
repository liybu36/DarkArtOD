////////////////////////////////////////////////////////////////////////
// Class:       SumChannels
// Module Type: producer
// File:        SumChannels_module.cc
//
// Generated at Fri Sep  6 11:48:56 2013 by Masayuki Wada using artmod
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

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/Products/Baseline.hh"
#include "darkart/Products/Pmt.hh"
#include "darkart/Products/ProductAssns.hh"
#include "darkart/Products/Channel.hh"

#include "darkart/TpcReco/ByChannel.hh"
#include "darkart/TpcReco/PmtHelper.hh"
#include "darkart/TpcReco/utilities.hh"
#include "darkart/TpcReco/common_algs.hh"

#include <memory>
#include <functional>   // std::binary_function
#include <algorithm>    // std::transform

namespace darkart {
  struct add_with_scaling;
  class SumChannels;
}

struct darkart::add_with_scaling : public std::binary_function<double, double, double>
{
  double const& _scale_factor;
  add_with_scaling(double const& scale_factor) : _scale_factor(scale_factor){}
  double operator()(double x, double y) { return x + y*_scale_factor; }
};

    
class darkart::SumChannels : public art::EDProducer {
public:
  explicit SumChannels(fhicl::ParameterSet const & p);
  virtual ~SumChannels();

  void produce(art::Event & e) override;
  void beginRun(art::Run & r) override;

private:

  // Declare member data here.
  std::vector<int> skip_channels_;
  art::InputTag ch_tag_;
  art::InputTag raw_wfm_tag_;
  art::InputTag pmt_tag_;
  art::InputTag baseline_tag_;
  art::InputTag bs_wfm_tag_;
  bool zero_suppressed_flag_;
  double zero_suppression_threshold; //Need to implement a check for value if zero_suppressed_flag_==true --by Masa

};


darkart::SumChannels::SumChannels(fhicl::ParameterSet const & p) :
// Initialize member data here.
  skip_channels_(p.get<std::vector<int> >("skip_channels", std::vector<int>())),
  ch_tag_(p.get<std::string>("ch_tag")),
  raw_wfm_tag_(p.get<std::string>("raw_wfm_tag")),
  pmt_tag_(p.get<std::string>("pmt_tag")),
  baseline_tag_(p.get<std::string>("baseline_tag")),
  bs_wfm_tag_(p.get<std::string>("bs_wfm_tag")),
  zero_suppressed_flag_(p.get<bool>("zero_suppressed_flag")),
  zero_suppression_threshold(p.get<double>("zero_suppression_threshold"))	
{
  // Call appropriate Produces<>() functions here.
  produces<darkart::ChannelVec>();
  produces<darkart::WaveformVec>();
  produces<darkart::WaveformAssns>();
  produces<darkart::WaveformInfoVec>();
  produces<darkart::WaveformInfoAssns>();
  produces<darkart::PmtVec, art::InRun>();
}

darkart::SumChannels::~SumChannels()
{
  // Clean up dynamic memory and other resources here.
}

void darkart::SumChannels::beginRun(art::Run & r)
{
  // Insert a Pmt object for the sum channel with spe mean of 1.
  
  std::unique_ptr<darkart::PmtVec> pmtVec(new darkart::PmtVec);
  darkart::Pmt pmt;
  pmt.channel_id = darkart::Channel::SUMCH_ID;
  pmt.spe_mean = 1.;
  pmtVec->push_back(pmt);
  r.put(std::move(pmtVec));
  
}

void darkart::SumChannels::produce(art::Event & e)
{
  // Retrieve the ChannelVec data product from the event.
  // The e.getValidHandle<>() method does not play well with making art::Assns,
  // so use explicit art::Handles.
  art::Handle <darkart::ChannelVec> chVecHandle;
  e.getByLabel(ch_tag_, chVecHandle);
  darkart::ChannelVec const& chVec(*chVecHandle);

  if (chVec.size() < 2) //No point in summing channels
    return;

  // Make our products, which begin empty
  std::unique_ptr <darkart::ChannelVec> sumChVec(new darkart::ChannelVec);
  std::unique_ptr <darkart::WaveformVec> sumWfmVec(new darkart::WaveformVec);
  std::unique_ptr <darkart::WaveformAssns> sumWfmAssns(new darkart::WaveformAssns);
  std::unique_ptr <darkart::WaveformInfoVec> sumWfmInfoVec(new darkart::WaveformInfoVec);
  std::unique_ptr <darkart::WaveformInfoAssns> sumWfmInfoAssns(new darkart::WaveformInfoAssns);

  darkart::Channel sumChInfo;
  darkart::Waveform sumWfm;

  int n_channels_summed = 0;

  // Fill the products. Call the algorithm to find and subtract the baseline.
  // Loop through the channels here, rather than in a separate algorithms file
  // to better ensure that products get mapped to the correct channel. For each
  // new object, add an entry to the corresponding Assns object.
  darkart::ByChannel<darkart::Waveform> get_wfm(chVecHandle, e, raw_wfm_tag_);
  darkart::ByChannel<darkart::Baseline> get_baseline(chVecHandle, e, baseline_tag_);
  darkart::ByChannel<darkart::Waveform> get_bs_wfm(chVecHandle, e, bs_wfm_tag_);
  darkart::PmtHelper get_pmt(e, pmt_tag_);
  for (auto const& chInfo : chVec)
    {
      // If told to skip this channel, then don't do anything
      if (util::skipChannel(chInfo, skip_channels_))
        continue;

      //Fill Channel Info for the sum channel
      if (n_channels_summed == 0)
        {
          sumChInfo.channelID.channel_id = darkart::Channel::SUMCH_ID;
          sumChInfo.label = "sum";
          sumChInfo.sample_bits = 32;

          sumChInfo.sample_rate = chInfo.sample_rate;
          sumChInfo.trigger_index = chInfo.trigger_index;
          sumChInfo.nsamps = chInfo.nsamps;
          sumWfm.sample_rate = chInfo.sample_rate;
          sumWfm.trigger_index = chInfo.trigger_index;

          sumWfm.wave.resize(chInfo.nsamps);
        }

      // Retrieve the input raw waveform from the event
      darkart::Channel::ChannelID const chID = chInfo.channelID;
      auto const& wfm = get_wfm(chID); //Ptr.
      auto const& bsData = get_baseline(chID); //Ptr.
      auto const& bsWfm = get_bs_wfm(chID); //Ptr.
      //line up the waveforms of the channels
      const int presamps = std::min(sumChInfo.trigger_index, chInfo.trigger_index);
      const int postsamps = std::min(sumChInfo.nsamps - sumChInfo.trigger_index, chInfo.nsamps - chInfo.trigger_index);
      auto const& pmt = get_pmt(chID);
//      //Should calibration constants like PmtImfo be a service? See http://mu2e.fnal.gov/public/hep/computing/Services.shtml --by Masa
      double scale_factor = 1. / pmt.spe_mean;

      //sum the two channels into a temporary vector
      std::vector<double> tempsum(presamps + postsamps);
      std::vector<double>::const_iterator sumit = sumWfm.wave.begin();
      std::vector<double>::const_iterator chit;
      if (wfm->isValid() || zero_suppressed_flag_)
        {
          if (!(bsData->found_baseline) || bsData->saturated)
            {
              //Baseline not found for one of the channels - do not save any sum
              n_channels_summed = 0;
              break;
            }

          if (!bsWfm->isValid())
            {
              n_channels_summed = 0;
              break;
            }
          chit = bsWfm->wave.begin();
        }
      else
        {
          if (!wfm->isValid())
            {
              n_channels_summed = 0;
              break;
            }
          chit = wfm->wave.begin();
        }

      if (zero_suppressed_flag_)
        {
          for (int i = 0; i < presamps + postsamps; i++)
            {
              if (-(*(chit + chInfo.trigger_index - presamps + i)*scale_factor) > zero_suppression_threshold)
                tempsum[i] = (*(sumit + sumChInfo.trigger_index - presamps + i)
                            + *(chit + chInfo.trigger_index - presamps + i) * scale_factor);
              else
                tempsum[i] = *(sumit + sumChInfo.trigger_index - presamps + i);
            }
        }
      else
        {
          //sum from trigger_index - resamps to trigger_index+postsamps
          std::transform(sumit + sumChInfo.trigger_index - presamps,
                         sumit + sumChInfo.trigger_index + postsamps,
                         chit + chInfo.trigger_index - presamps, tempsum.begin(),
                         add_with_scaling(scale_factor)); //defined above
        }
      //copy the temporary into the sumdata waveform vector
      sumWfm.wave.assign(tempsum.begin(), tempsum.end());

      //reset the historical channel_start and end pointers
      sumChInfo.trigger_index = presamps;
      sumChInfo.nsamps = presamps + postsamps;

      // the sum is "saturated" if any single channel is
      if (chInfo.saturated)
        sumChInfo.saturated = true;

      n_channels_summed++;
    }

  // Fill the WaveformInfo object
  darkart::WaveformInfo sumWfmInfo = darkart::fillWaveformInfo(sumWfm); //fillWaveformInfo() is defined in the TpcReco/commou_algs.hh

  // if could not create a sum channel, save default versions of products
  // but still save the filled darkart::Channel object for the sum channel
  if (n_channels_summed == 0)
    {
      sumWfm = darkart::Waveform();
      sumWfmInfo = darkart::WaveformInfo();

    }

  
  // Add the objects to the corresponding vectors
  sumChVec->push_back(sumChInfo);
  sumWfmVec->push_back(sumWfm);
  sumWfmInfoVec->push_back(sumWfmInfo);

  // Fill the Assns
  util::createAssn(*this, e, *sumChVec, sumChInfo, *sumWfmVec, sumWfm,
      *sumWfmAssns);

  util::createAssn(*this, e, *sumChVec, sumChInfo, *sumWfmInfoVec, sumWfmInfo,
      *sumWfmInfoAssns);

  // Put our complete products into the Event
  e.put(std::move(sumChVec));
  e.put(std::move(sumWfmVec));
  e.put(std::move(sumWfmAssns));
  e.put(std::move(sumWfmInfoVec));
  e.put(std::move(sumWfmInfoAssns));

}


DEFINE_ART_MODULE(darkart::SumChannels)
