#include "darkart/TpcReco/baselinefinder_algs.hh"
#include "darkart/TpcReco/common_algs.hh"


#include <iostream>
#include <cmath>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////
//////////////////      DRIFTING BASELINE ALGORITHM       /////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
bool darkart::DriftingBaselineParams::isValidRegion(size_t region) const
{
    bool valid = true;
    if (region >= region_times.size())
	valid = false;
    else if (region_times.at(region).size() != 2)
	valid = false;
    else if (region_times.at(region).at(0) > region_times.at(region).at(1))
	valid = false;
    
    return valid;
}

//-----------------------------------------------------------------------------
void darkart::convertRegionTimesToSamples(DriftingBaselineParams const& p,
					  darkart::Waveform const& wfm, 
					  std::vector<std::vector<int> >& region_samples)
{
    for (size_t i = 0; i < p.numRegions(); i++)
    {
	if (p.isValidRegion(i))
	{
	    // Convert region times to samples
	    int start = wfm.TimeToSample(p.region_times.at(i).at(0), true);
	    int end = wfm.TimeToSample(p.region_times.at(i).at(1), true);
	    std::vector<int> samps;
	    samps.push_back(start);
	    samps.push_back(end);
	    region_samples.push_back(samps);
	}
    }
}

//-----------------------------------------------------------------------------
bool darkart::isWithinRegion(int const samp, std::vector<std::vector<int> > const& region_samples)
{
    for (size_t i = 0; i < region_samples.size(); i++)
    {
	if (samp >= region_samples.at(i).at(0) && samp <= region_samples.at(i).at(1))
	    return true;
    }
    return false;
}


//-----------------------------------------------------------------------------
void darkart::interpolateBaseline(std::vector<double> & baseform,
                                  double const mean,
                                  int const samp,
                                  int const last_good_samp,
                                  DriftingBaselineParams const& p)
{
  double preval = mean;
  if (last_good_samp >= 0)
    preval = baseform[last_good_samp];
  double slope = (mean-preval)/((samp-p.post_samps)-last_good_samp);
  for (int backsamp = last_good_samp+1; backsamp < samp; backsamp++)
    baseform[backsamp] = preval + slope*(backsamp-last_good_samp);
}

//-----------------------------------------------------------------------------
void darkart::driftingBaseline(darkart::Channel const& ch,
                               darkart::Waveform const& wfm,
                               darkart::Waveform & bsWfm,
                               darkart::Baseline & baseline,
                               DriftingBaselineParams const& p)
{
  const int nsamps = wfm.wave.size();

  // bsWfm starts off the same as wfm
  bsWfm = wfm;
  
  // Convert region times to samples
  std::vector<std::vector<int> > region_samples; //vector containing range of region samples
  convertRegionTimesToSamples(p, wfm, region_samples);
  
  // find the maximum sample value within the pre-trigger area
  int pre_trig_samp = wfm.TimeToSample(p.signal_begin_time);
  if (pre_trig_samp < 0) pre_trig_samp = p.pre_samps+p.post_samps;
  if (pre_trig_samp >= nsamps) pre_trig_samp = nsamps-1;
  double max_pre_trig = wfm.wave[darkart::findMaxSamp(wfm, 0, pre_trig_samp)];
  if (std::abs(ch.GetVerticalRange() - max_pre_trig) < 0.01)
    baseline.saturated = true;

  double sum = 0;
  int sum_samps = 0;
  int samp = -1;
  int window_samps = p.pre_samps + p.post_samps + 1;
  int last_good_samp = -1;
  double moving_base = -1;
  baseline.found_baseline = false;
  bool was_in_baseline = false; //State of previous sample
  
  double start_max_amplitude = p.start_max_amplitude;
  double end_max_amplitude = p.end_max_amplitude;
  // See if max_amplitude should have a different value for this channel
  for (auto const& ch_max_amp : p.start_max_amplitude_ch_overrides) 
  {
      if (ch_max_amp.size() == 0) 
	  continue;
      if (ch_max_amp[0] == ch.channel_id())
	      start_max_amplitude = ch_max_amp[1];
  }
  for (auto const& ch_max_amp : p.end_max_amplitude_ch_overrides) 
  {
      if (ch_max_amp.size() == 0) 
	  continue;
      if (ch_max_amp[0] == ch.channel_id())
	  end_max_amplitude = ch_max_amp[1];
  }
       
  while (++samp < nsamps-1) 
  {
     
    // Determine if we're in baseline
    bool in_baseline = false;
    if (!baseline.found_baseline && max_pre_trig-wfm.wave[samp] < 2.*start_max_amplitude)
      in_baseline = true;
    else if (baseline.found_baseline && was_in_baseline 
	     && std::abs(wfm.wave[samp+1]-moving_base) < start_max_amplitude)
      in_baseline = true;
    else if (baseline.found_baseline && !was_in_baseline 
	     && std::abs(wfm.wave[samp-2]-moving_base) < end_max_amplitude)
	in_baseline = true;
    
    if (p.interpolate_regions && (isWithinRegion(samp, region_samples)))
    {
	in_baseline = false;
	baseline.laserskip = true;
    }
    
    if (in_baseline)
    {
	//this is part of the baseline
	sum += wfm.wave[samp];
	sum_samps++;
	
	if (sum_samps > window_samps) 
	{
	    //we have collected too many samples, so remove the earliest one
	    sum -= wfm.wave[samp-window_samps];
	    sum_samps--;
	}
	if (sum_samps == window_samps)
	{
	    //this is a validly averaged sample segment'
	    double mean = sum/sum_samps;
	    moving_base = mean;
	    bsWfm.wave[samp-p.post_samps] = mean;
	    if (last_good_samp < samp - p.post_samps-1)
	    {
		//linearily interpolate the baseline to fill this region
		interpolateBaseline(bsWfm.wave, mean, samp, last_good_samp, p);
		baseline.interpolations.emplace_back(std::make_pair(wfm.SampleToTime(last_good_samp),wfm.SampleToTime(samp-p.post_samps)));
	    }
	    last_good_samp = samp-p.post_samps;
	    if (!baseline.found_baseline)
	    {
		baseline.found_baseline = true;
		baseline.mean = mean;
		baseline.search_start_index = samp - window_samps + 1;
		baseline.length = window_samps;
		//calculate the variance
		double sum2 = 0;
		for (int backsamp = samp - window_samps+1; backsamp<=samp; backsamp++)
		    sum2 += wfm.wave[backsamp]*wfm.wave[backsamp];
		baseline.variance = sum2/window_samps - mean*mean;
	    }
	}// end if valid sample segment
    }// end if in_baseline
    else
    {
	// this is not part of the baseline; we are in real signal
	sum = 0;
	sum_samps = 0;
	if (!baseline.found_baseline && samp>pre_trig_samp)
	{
	    //can't find baseline in pre-trigger region! abort!
	    bsWfm.wave.clear();
	    return;
	}
    }
    was_in_baseline = in_baseline;
  }// end loop over samples

  // if the baseline is no good, save an empty waveform
  if (baseline.saturated || !baseline.found_baseline)
  {
      bsWfm.wave.clear();
      return;
  }

  // 2015-04-14 AFan -- Change baseline.mean and baseline.variance to be from fixed window
  baseline.mean = 0;
  baseline.variance = 0;
  double baseline_mean_nsamps = wfm.sample_rate*p.baseline_mean_window;
  for (samp=0; samp<baseline_mean_nsamps; samp++) {
    baseline.mean += wfm.wave[samp];
    baseline.variance += wfm.wave[samp]*wfm.wave[samp];
  }
  baseline.mean /= baseline_mean_nsamps;
  baseline.variance /= baseline_mean_nsamps;
  baseline.variance -= baseline.mean*baseline.mean;
  
  
  //fill in the missing part at the end
  for (samp = last_good_samp+1; samp<nsamps; samp++)
  {
      bsWfm.wave[samp] = bsWfm.wave[last_good_samp];
  }
  if (last_good_samp < nsamps - 1)
      baseline.interpolations.emplace_back(std::make_pair(wfm.SampleToTime(last_good_samp),wfm.SampleToTime(nsamps - 1)));  
  
  //subtract off the baseline
  for (samp=0; samp<nsamps; samp++)
  {
      bsWfm.wave[samp] = wfm.wave[samp]-bsWfm.wave[samp];
  }
  
}


///////////////////////////////////////////////////////////////////////////////////////
/////////////////////      FIXED BASELINE ALGORITHM       /////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
void darkart::fixedBaseline(darkart::Channel const& ch,
                            darkart::Waveform const& wfm,
                            darkart::Waveform & bsWfm,
                            darkart::Baseline & baseline,
                            FixedBaselineParams const& p)
{
  const int nsamps = wfm.wave.size();
  bsWfm = wfm;
  
  // find the maximum sample value within the pre-trigger area
  int pre_trig_samp = wfm.TimeToSample(p.signal_begin_time);
  if (pre_trig_samp < 0 || pre_trig_samp >= nsamps) return;
  double max_pre_trig = wfm.wave[darkart::findMaxSamp(wfm, 0, pre_trig_samp)];
  double min_pre_trig = wfm.wave[darkart::findMinSamp(wfm, 0, pre_trig_samp)];
  if (std::abs(ch.GetVerticalRange() - max_pre_trig) < 0.01 || min_pre_trig < 0.01)
    baseline.saturated = true;

  double sum=0, sum2=0, mean=0, sigma=0;
  double seg_sum=0, seg_sum2=0, seg_mean=0, seg_sigma=0;
  int sum_samps = 0;

  baseline.found_baseline = false;
  int samp = -1;
  while (++samp < nsamps) {
    if (samp >= pre_trig_samp) break;

    if (!(samp % p.segment_samps) && samp) {
      // another segment recorded
      seg_mean = seg_sum/p.segment_samps;
      seg_sigma = std::sqrt(seg_sum2/p.segment_samps-seg_mean*seg_mean);
			
      if (seg_sigma>p.max_sigma) {
        //baseline variance is too big, skip this segment
        //do nothing
      }
      else if (sum<1e-6 && sum2<1e-6) {
        //initialize the baseline
        sum  = seg_sum;
        sum2 = seg_sum2;
        mean = seg_mean;
        sigma = seg_sigma;
        sum_samps = p.segment_samps;
        baseline.search_start_index = samp - p.segment_samps;
      }
      else if (std::fabs(sigma-seg_sigma) < p.max_sigma_diff &&
               std::fabs(mean-seg_mean) < p.max_mean_diff) {
        //add new segment to baseline calculateion
        sum += seg_sum;
        sum2 += seg_sum2;
        sum_samps += p.segment_samps;
        mean = sum/sum_samps;
        sigma = sqrt(sum2/sum_samps-mean*mean);
      }
      else if (sum_samps>=p.min_valid_samps) {
        //baseline is valid, ignore the remaining part
        break;
      }
      else if (sigma>seg_sigma) {
        //start new baseline if the new segment has much smaller sigma
        sum = seg_sum;
        sum2 = seg_sum2;
        mean = seg_mean;
        sigma = seg_sigma;
        sum_samps = p.segment_samps;
        baseline.search_start_index = samp - p.segment_samps;
      }
      else{
        //reset baseline and start over if everything is a mess
        sum = 0;
        sum2 = 0;
        mean = 0;
        sigma = 0;
        sum_samps = 0;
      }
      seg_sum = 0;
      seg_sum2 = 0;
    }
    seg_sum  += wfm.wave[samp];
    seg_sum2 += wfm.wave[samp]*wfm.wave[samp];
  }// end loop over samples

 
  if (sum_samps >= p.min_valid_samps) {
    baseline.found_baseline = true;
    baseline.mean = mean;
    baseline.variance = sigma*sigma;
    baseline.length = sum_samps;
    
    //subtract off the baseline
    for(samp=0; samp<nsamps; samp++){
      bsWfm.wave[samp] = wfm.wave[samp]-mean;
    }
  }

  // if the baseline is no good, save an empty waveform
  if (baseline.saturated || !baseline.found_baseline) {
    bsWfm.wave.clear();
    return;
  }
  

}


///////////////////////////////////////////////////////////////////////////////////////
/////////////////////      ZERO BASELINE ALGORITHM       //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
void darkart::zeroBaseline(darkart::Waveform const& wfm,
                           darkart::Waveform & bsWfm,
                           darkart::Baseline & baseline)
{
  // Set the baseline to zero everywhere
  baseline.found_baseline = true;
  baseline.mean = 0;
  baseline.variance = 0;
  baseline.saturated = false;
  baseline.length = wfm.wave.size();
  baseline.search_start_index = 0;
  baseline.laserskip = false;

  bsWfm = wfm;

}

