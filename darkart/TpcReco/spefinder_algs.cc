
#include "darkart/TpcReco/spefinder_algs.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "darkart/TpcReco/common_algs.hh"

#include <cmath>
//#include <numeric>
#include <algorithm>
#include <TMath.h>
#include <iostream>

//-----------------------------------------------------------------------------
darkart::SpeVec darkart::findSPEs(darkart::Channel const& ch,
				  darkart::Waveform const& bswfm,
				  darkart::Waveform const& intWfm,
				  darkart::PulseVec const& pulseVec,
				  //vector< Ptr<darkart::Pulse>> pulseVec,
				  double_t nepOfFirstPls,
				  darkart::SpeFinderParams const& params)
{
  darkart::SpeVec spe_vec;


  double debug_start = -0.02;
  double debug_end = 0.06;
  int n_peaks_found=0;

  double rough_threshold = params.rough_threshold;
  double fine_threshold = params.fine_threshold;
  // See if rough_threshold should have a different value for this channel
  for (auto const& ch_rough_thresh : params.rough_threshold_ch_overrides) {
    if (ch_rough_thresh.size() == 0) continue;
    if (ch_rough_thresh[0] == ch.channel_id())
      rough_threshold = ch_rough_thresh[1];
  }
  // See if fine_threshold should have a different value for this channel
  for (auto const& ch_fine_thresh : params.fine_threshold_ch_overrides) {
    if (ch_fine_thresh.size() == 0) continue;
    if (ch_fine_thresh[0] == ch.channel_id())
      fine_threshold = ch_fine_thresh[1];
  }
  
  //convert the window lengths in time to sample numbers
  int winscan;
  if (params.energy_dep_start_time)
    winscan = bswfm.TimeToSample(pulseVec[0].start_time + 1.6*TMath::Log(nepOfFirstPls/10 + 1));
  else
    winscan = bswfm.TimeToSample(pulseVec[0].start_time + params.search_start_time);

  if (winscan < 0)
    winscan = bswfm.TimeToSample(pulseVec[0].start_time);

  LOG_DEBUG("Info") << "Channel " <<ch.channel_id()
		    << " Start search@ " <<bswfm.SampleToTime(winscan)
		    << ", which is sample " <<winscan;

  int baseline_window_samps = (int)(params.baseline_window * bswfm.sample_rate);
  int pre_threshold_samps = (int)(params.pre_threshold * bswfm.sample_rate);
  int post_threshold_samps = (int)(params.post_threshold * bswfm.sample_rate);

  //determine the end of the search
  int nsamps;
  if (params.energy_dep_start_time)
    {
      if (pulseVec.size() > 1) //End search before start of next pulse
	nsamps = std::min(pulseVec[1].start_index,
			  bswfm.TimeToSample(pulseVec[0].start_time +
					     1.6*TMath::Log(nepOfFirstPls + 1)));
      else
	nsamps = bswfm.TimeToSample(pulseVec[0].start_time +
				    1.6*TMath::Log(nepOfFirstPls + 1));
    }
  else
    {
      if (pulseVec.size() > 1) //End search before start of next pulse
	nsamps = std::min(pulseVec[1].start_index, bswfm.TimeToSample(pulseVec[0].start_time + params.search_end_time));
      else
	nsamps = bswfm.TimeToSample(pulseVec[0].start_time + params.search_end_time);
    }

  //Check to make sure search ends before end of window (leave room for the post_window ranges)
  if (nsamps > ch.nsamps - baseline_window_samps)
    nsamps = ch.nsamps - baseline_window_samps;

  LOG_DEBUG("Info") << "Channel " <<ch.channel_id() << " End search@ " <<bswfm.SampleToTime(nsamps) << ", which is sample " <<nsamps;




  //bswfm.TimeToSample(curr_ev_data->s1_end_time); to end of s1
  std::vector <double> const& wave = bswfm.wave;

  //previous is the sample index of the last located pulse
  //int previous_start = 0; //this variable doesn't appear to be used for anything...
  int previous_end = 0;

  //search starting at winscan and go until nsamps 
  bool secondary_pulse = false; //Is there a pulse immediately after this one?
  bool prev_sec_pulse = false;  //Was there a pulse immediately before this one?
  double prev_loc_bl = 0;
  for(int test_sample = winscan; test_sample < nsamps; test_sample++)
    {
      prev_sec_pulse = secondary_pulse;
      secondary_pulse = false;
      double current_time=bswfm.SampleToTime(test_sample);
      if(current_time>=debug_start && current_time<=debug_end)
        LOG_DEBUG("Info") << "At " << current_time << "us the amplitude is " << wave[test_sample];

      if(!(IsPulse(test_sample, wave, rough_threshold) || prev_sec_pulse)) continue;

      //      {

      //Start of spe
      int samp;

      if(!prev_sec_pulse)
	samp = test_sample - pre_threshold_samps;
      else
	samp = test_sample;


      //case 1: this is the first peak, so we need to search the area before
      if(previous_end == 0 && (samp-winscan < baseline_window_samps) )
	{
	  LOG_DEBUG("Info") << "Channel " << ch.channel_id()
			    << " " << current_time << ": this is the first peak";

	  //this is the first peak, and we haven't searched the area behind
	  //look back and make sure there are no peaks in the pre area
	  bool pre_peak_found = false;
	  for(int presample = samp - baseline_window_samps; presample < samp; presample++)
	    {
	      if( (wave[presample] - wave[presample+2] >= fine_threshold) &&
		  (wave[presample+1] <= wave[presample] ) &&
		  (wave[presample+2] <= wave[presample+1] ) )
		{
		  pre_peak_found = true;
		  LOG_DEBUG("Info") << "Channel " << ch.channel_id() <<"\n"
				    << "samp = " <<bswfm.SampleToTime(samp)<<"\n"
				    <<"presample = "<<bswfm.SampleToTime(presample)<<"\n"
				    <<"wave[ps] = "<<wave[presample]<<"\n"
				    <<"wave[ps+1] = "<<wave[presample+1]<<"\n"
				    <<"wave[ps+2] = "<<wave[presample+2]<<"\n"
				    <<".\n";
		  break;
		}
	    }
	  if(pre_peak_found)
	    {
	      LOG_DEBUG("Info") << "Channel " << ch.channel_id()
				<< " " << current_time <<": pre_peak_found";
	      // there was a peak before this one that will mess stuff up, so skip
	      continue;
	    }
	}//end if for the first peak

      //case 2: we are too close to the previous peak (don't care anymore)

      // evaluate the local baseline within the baseline_window_samps window
      // shift index by 1 to match SpeFinder in genroot, which uses accumulate
      double local_baseline = darkart::integral(intWfm, samp-baseline_window_samps-1, samp-1)/ baseline_window_samps;
      
      //Check for the end of the pulse. Find the point at which the wave goes
      //back to close to the original value.
      
      if(current_time>debug_start && current_time<debug_end)
        {
          LOG_DEBUG("Info") << bswfm.SampleToTime(samp)   << "\t"
                            << wave[samp]                 << "\n"
                            << bswfm.SampleToTime(samp+1) << "\t"
                            << wave[samp+1]               << "\n"
                            << bswfm.SampleToTime(samp+2) << "\t"
                            << wave[samp+2]               << "\n";
        }

      int exit_sample = samp+3;
      for(; exit_sample <= nsamps - post_threshold_samps; exit_sample++)
        {
          if(current_time>debug_start && current_time<debug_end)
            LOG_DEBUG("Info") << "\tThen " << wave[exit_sample] << " ("
                              << wave[exit_sample-1] << ") at "
                              << bswfm.SampleToTime(exit_sample) << "\n";

          if (IsSecondPulse(exit_sample, wave, fine_threshold))
            {
              LOG_DEBUG("Info") << "\nEnded because another pulse was found\n";
              secondary_pulse = true;
              break;
            }
          if(-wave[exit_sample]<=params.return_fraction*rough_threshold)
            {
              secondary_pulse = false;
              LOG_DEBUG("Info") << "\nEnded because of return fraction\n";
              break;
            }

        } //End check for pulse end

      int end_sample;
      if (!secondary_pulse)
        {//Make sure there is no secondary pulse in the post threshold samples
          for(end_sample = exit_sample; end_sample - exit_sample <= post_threshold_samps; end_sample++)
            {
              if (IsSecondPulse(end_sample, wave, fine_threshold))
                {
                  LOG_DEBUG("Info") << "\nEnded because another pulse was found in post samples\n";
                  secondary_pulse = true;
                  break;
                }
            }
          if (!secondary_pulse)
            end_sample = exit_sample + post_threshold_samps; // Do we need this??? end_sample is exit_sample + post_threshold_samps anyway if secondary_pulse is false.

        } else
        end_sample = exit_sample;

      LOG_DEBUG("Info") << "end_sample: " << bswfm.SampleToTime(end_sample) << "\n";

      if(end_sample >= nsamps)
        { //We went to the end of the window
          LOG_DEBUG("Info") << current_time << ": We went to the end of the window\n";
          secondary_pulse = false;
          continue;
        }

      //if we get here, this peak is good
      LOG_DEBUG("Info") << bswfm.SampleToTime(samp) << "," << bswfm.SampleToTime(end_sample) << "\n";

      //Evaluate integral
      // shift index by 1 to match SpeFinder in genroot, which uses accumulate.
      double integral = darkart::integral(intWfm, samp-1, end_sample-1); // integral() is defined in common_algs.hh

      //invert integral to make positive
      integral =- integral;
      LOG_DEBUG("Info") << "integral: "<<integral << "\n";
      int max_sample = std::min_element(wave.begin()+samp,wave.begin()+end_sample)-wave.begin();

      darkart::Spe found_spe;

      //store everything in the tree
      found_spe.integral = integral;
      found_spe.start_time = bswfm.SampleToTime(samp);
      found_spe.amplitude = - wave[max_sample];
      found_spe.peak_time = bswfm.SampleToTime(max_sample);
      if(prev_sec_pulse)
        found_spe.local_baseline =
          (secondary_pulse ? prev_loc_bl :
           // shift index by 1 to match SpeFinder in genroot, which uses accumulate
           darkart::integral(intWfm, end_sample-1, end_sample+baseline_window_samps-1)/baseline_window_samps);
      else
	found_spe.local_baseline = local_baseline;
      found_spe.length = (end_sample-samp)*1.0/bswfm.sample_rate;
      found_spe.start_clean = !prev_sec_pulse;
      found_spe.end_clean = !secondary_pulse;
      found_spe.dt = (samp - previous_end)*1.0/bswfm.sample_rate;

      //Save spe
      spe_vec.push_back(found_spe);

      ++n_peaks_found;
      LOG_DEBUG("Info") << "Found good one: "<<current_time<<", end point: "
			<<bswfm.SampleToTime(end_sample)
			<<"\nThere are "<<spe_vec.size()<<" spes\n";

      //we've found a peak, so update the previous designation
      //previous_start = samp;
      previous_end = end_sample;

      //have we found the max number yet?
      if(spe_vec.size() >= (size_t)params.max_photons)
	{
	  LOG_DEBUG("Info") << current_time
			    <<"\nWe have found the max number of photons already\n";
	  return spe_vec;
	}
      if(!prev_sec_pulse)
	prev_loc_bl = local_baseline;

      test_sample=end_sample-1; //Start search for next pulse at the end of this pulse
      //      }//end if statement looking for start of pulse
    } //end for loop over samples
  LOG_DEBUG("Info") << "SpeFinder ending\n"
		    << n_peaks_found << " peaks found\n";
  return spe_vec;
}

bool darkart::IsPulse(int sample, std::vector <double> const& wave, double rough_threshold ) {
  return ( wave[sample] - wave[sample+2] >= rough_threshold &&
           ( ( (wave[sample+1]-wave[sample])/
               (wave[sample+2]-wave[sample]) >= 0) || // Should it be 1 instead of 0 ?
             ( (wave[sample+3]-wave[sample])/
               (wave[sample+2]-wave[sample]) >= 0) ) );  // Should it be 1 instead of 0 ?
         
}

bool darkart::IsSecondPulse(int sample, std::vector <double> const& wave, double fine_threshold) {
  return (
          wave[sample]>=wave[sample-1]-0.5 //make sure pulse is going up
	  && (wave[sample-1]>=wave[sample-2] || fabs(wave[sample-1]-wave[sample-2]) <= 0.01*std::min(fabs(wave[sample-1]), fabs(wave[sample-2])))// step one before has the same trend(going up). Or the difference between sample-1 and sample-2 is smaller than 1% of amplitude
	  && wave[sample]>=wave[sample+1]+fine_threshold //looking for trend change in the next step
	  )
    ||
    (
     wave[sample]>=wave[sample-1]+fine_threshold //make sure pulse is going up
     && wave[sample]>=wave[sample+1]+fine_threshold //looking for trend change in the next step
     )
    ||
    (
     wave[sample]>=wave[sample-1]-0.5 //make sure pulse is going up
     && wave[sample]>=wave[sample+2]+fine_threshold //looking for trend change in the next next step
     && (wave[sample]>=wave[sample+1] || fabs(wave[sample]-wave[sample+1]) <= 0.01*std::min(fabs(wave[sample]), fabs(wave[sample+1])))// the next step has the same trend(going down). Or the difference between sample and sample+1 is smaller than 1% of amplitude
     );
}


