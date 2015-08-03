#include "darkart/TpcReco/pulsefinder_algs.hh"
#include "darkart/TpcReco/common_algs.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <vector>
#include <cmath>
#include <iostream>

darkart::PulseVec darkart::curvatureSearch(darkart::Waveform const& bs_wfm,
					   darkart::Waveform const& int_wfm,
					   double spe_mean,
					   double sample_rate,
					   darkart::CurvatureSearchParams const& params)
{
    std::vector<int> start_index;
    std::vector<int> end_index;

    int dsf = params.down_sample_factor;
    int nsamps = bs_wfm.wave.size()/dsf;
    
    std::vector<double> ds_int_wfm(nsamps); //Down-sampled integrated waveform
    std::vector<double> ds_diff_wfm(nsamps); //Down-sampled difference waveform
    std::vector<double> ds_curve_wfm(nsamps); //Down-sampled curvature waveform
    
    for (int i = 0; i < nsamps; i++)
	ds_int_wfm[i] = int_wfm.wave[i*dsf];

    ds_diff_wfm[0] = ds_int_wfm[1];
    for (int i = 1; i < nsamps-1; i++)
	ds_diff_wfm[i] = ds_int_wfm[i+1] - ds_int_wfm[i-1];

    ds_curve_wfm[0] = ds_diff_wfm[1];
    for (int i = 1; i < nsamps-1; i++)
	ds_curve_wfm[i] = ds_diff_wfm[i+1] - ds_diff_wfm[i-1];

    int s1_pulse_end_samps = (int)(params.s1_pulse_end_window * sample_rate);
    int s2_pulse_end_samps = (int)(params.s2_pulse_end_window * sample_rate);
    std::vector<bool> is_s1_like_vec;
    std::vector<double> ratio1_vec;
    int look_ahead_end_samps;
    double pulse_end_threshold;
  
    //int pulse_end_samps = (int) (params.pulse_end_window * sample_rate);
    
    bool in_pulse = false;
    bool before_peak = true; //before peak of diff
    int last_max = -1; //index of last local maximum on diff
    
    for (int i = 0; i < nsamps-1; i++)
    {
	if (!in_pulse)
	{
	    if (ds_curve_wfm[i] < params.pulse_start_curvature)
	    {
		in_pulse = true;
		int start = i*dsf;
       
		int loop_count = 0;
		int max_loop = dsf;
		if (i < nsamps-2)
		    max_loop += dsf;
		while (++loop_count < max_loop && -bs_wfm.wave[start] < params.amplitude_start_threshold)
		    start++;
		
		start_index.push_back((start-2 > 0) ? start - 2 : 0);
        
		//Determine pulse shape for search of pulse end
		if(start_index.back() + (int)(3.5*sample_rate) < (int)int_wfm.wave.size()){
		  ratio1_vec.push_back((int_wfm.wave[start_index.back() + (int)(0.088*sample_rate)] - int_wfm.wave[start_index.back()])/
				       (int_wfm.wave[start_index.back() + (int)(3.5*sample_rate)] - int_wfm.wave[start_index.back()]));
		}
        
		else
		  ratio1_vec.push_back(1E6); //Default to S1-like pulse
        
		if (ratio1_vec.back() > params.s1_ratio1_threshold){
		  look_ahead_end_samps = s1_pulse_end_samps;
		  pulse_end_threshold = params.s1_pulse_end_threshold;
		  is_s1_like_vec.push_back(true);
		}
        
		else{
		  look_ahead_end_samps = s2_pulse_end_samps;
		  pulse_end_threshold = params.s2_pulse_end_threshold;
		  is_s1_like_vec.push_back(false);
		}
        
	    }	    
	}
	else 
	{    //in pulse
	    if (before_peak)
	    {   //look for peak of diff
		if (ds_curve_wfm[i] > 0)
		{
		    before_peak = false;
		}
	    }
	    else
	    {   //after peak of diff
		if (ds_curve_wfm[i] < 0 && last_max < 0)
		{    //keep track of last diff maximum
		    last_max = i;
		}
		if (ds_curve_wfm[i] > 0 && last_max > 0)
		{    //last diff maximum not start of pileup
		    last_max = -1;
		}
		
		if (ds_curve_wfm[i] < params.pile_up_curvature)
		{    //found pile up
		    int pileup = i*dsf;
		    //start fine-grained search for start of pile up
		    int loopcount = 0;
		    int maxloop = dsf;
		    if (i < nsamps - 2) maxloop += dsf;
		  
		    while ( ++loopcount < maxloop && 
			    -(int_wfm.wave[pileup + 1] - int_wfm.wave[pileup]) < params.pile_up_start_threshold 
			    &&  -(int_wfm.wave[pileup + 2] - int_wfm.wave[pileup + 1]) < params.pile_up_start_threshold 
			)
		      pileup++;
		  
		    end_index.push_back(pileup);
		    start_index.push_back(pileup);
		    before_peak = true;
		    last_max = -1;
      
		    //Determine pulse shape for search of pulse end
		    if(start_index.back() + (int)(3.5*sample_rate) < (int)int_wfm.wave.size()){
		      ratio1_vec.push_back((int_wfm.wave[start_index.back() + (int)(0.088*sample_rate)] - int_wfm.wave[start_index.back()])/
					   (int_wfm.wave[start_index.back() + (int)(3.5*sample_rate)] - int_wfm.wave[start_index.back()]));
		    }
      
		    else
		      ratio1_vec.push_back(1E6); //Default to S1-like pulse
      
		    if (ratio1_vec.back() > params.s1_ratio1_threshold){
		      look_ahead_end_samps = s1_pulse_end_samps;
		      pulse_end_threshold = params.s1_pulse_end_threshold;
		      is_s1_like_vec.push_back(true);
		    }
      
		    else{
		      look_ahead_end_samps = s2_pulse_end_samps;
		      pulse_end_threshold = params.s2_pulse_end_threshold;
		      is_s1_like_vec.push_back(false);
		    }
                    
		}
        
    else if ( (i*dsf + look_ahead_end_samps) < (int)bs_wfm.wave.size() &&
	      ( (int_wfm.wave[i*dsf] - int_wfm.wave[i*dsf + look_ahead_end_samps]) / spe_mean < pulse_end_threshold*look_ahead_end_samps/sample_rate)

	   /* (i*dsf + pulse_end_samps) < (int)bs_wfm.wave.size() &&
	      (int_wfm.wave[i*dsf] - int_wfm.wave[i*dsf + pulse_end_samps])/spe_mean < params.pulse_end_threshold
	   */
	      )
      { //pulse gradually ends
	end_index.push_back(i*dsf);
	in_pulse = false;
	before_peak = true;
	last_max = -1;
      }
        
	    }
	}
    }
    
    if (in_pulse)
    {
	end_index.push_back(bs_wfm.wave.size() - 1);
    }

    
    darkart::PulseVec pulse_vec; 

    if (start_index.size() != end_index.size())
    {
	//mf::LogWarning("DataState")<<"Pulse finding algorithm failure";
	return pulse_vec;
    }

/*  //We repurpose a lot of these variables for the tdriftbump correction.
    //Port old code checking if start is too far from peak ? It's messy !
    std::vector<bool> is_s1_like_vec;
    std::vector<double> ratio1_vec;
    std::vector<double> ratio2_vec;
    std::vector<bool> is_start_shifted_vec;
    std::vector<double> start_shift_vec;
    s1StartAdjustment(bs_wfm, int_wfm, start_index, end_index,
                      is_s1_like_vec, ratio1_vec, ratio2_vec,
                      is_start_shifted_vec, start_shift_vec);
*/
    //Fill pulse info
    int n_pulses = start_index.size();
    for (int i = 0; i < n_pulses; i++)
    {
	    darkart::Pulse temp_pulse;
	    temp_pulse.start_index = start_index[i];
	    temp_pulse.end_index = end_index[i];
	    temp_pulse.start_time = bs_wfm.SampleToTime(start_index[i]);
	    temp_pulse.end_time = bs_wfm.SampleToTime(end_index[i]);
            temp_pulse.is_s1_like = is_s1_like_vec[i];
            temp_pulse.ratio1 = ratio1_vec[i];
	    //temp_pulse.ratio2 = ratio2_vec[i];
            //temp_pulse.start_shifted = is_start_shifted_vec[i];
            //temp_pulse.start_shift = start_shift_vec[i];
            
	    if (i == 0)
	    {
		temp_pulse.start_clean = (start_index[i] > 0);
		temp_pulse.dt = temp_pulse.start_time;
	    }
	    else
	    {
		temp_pulse.start_clean = (start_index[i] > end_index[i-1]);
		temp_pulse.dt = bs_wfm.SampleToTime(start_index[i]) - bs_wfm.SampleToTime(start_index[i-1]);
	    }

	    if ( i == n_pulses - 1)
		temp_pulse.end_clean = (end_index[i] < (int)bs_wfm.wave.size() - 1);
	    else
		temp_pulse.end_clean = (end_index[i] < start_index[i+1]);

            //Save pulse
	    pulse_vec.push_back(temp_pulse);

    }

    return pulse_vec;
}

void darkart::s1StartAdjustment(darkart::Waveform const& bs_wfm,
				darkart::Waveform const& int_wfm,
				std::vector<int>& start_index, std::vector<int>& end_index,
                                std::vector<bool>& is_s1_like_vec,
				std::vector<double>& ratio1_vec, std::vector<double>& ratio2_vec,
				std::vector<bool>& is_start_shifted_vec, std::vector<double>& start_shift_vec)
{


  //Check for whether the start found is too far from the peak, when it is an S1 pulse
    
    int ratio_samps = (int)(0.02*bs_wfm.sample_rate);
    double max_s1_peak_sep_time = 0.04;  // 40 ns
    double default_peak_sep_time = 0.02; // 20 ns

    int n_samps = bs_wfm.wave.size();
    for (size_t i = 0; i < start_index.size(); i++)
    {
	double pulse_integral = darkart::integrate(bs_wfm, start_index[i], end_index[i]);
	int peak_index = darkart::findMinSamp(bs_wfm, start_index[i], end_index[i]);

	double ratio1 = 0, ratio2 = 0;
	if (peak_index >= ratio_samps && 
	    peak_index < n_samps - ratio_samps)
	{
	    ratio1 = (int_wfm.wave[peak_index + ratio_samps] - int_wfm.wave[peak_index - ratio_samps])/pulse_integral;
	    ratio2 = (int_wfm.wave[peak_index - ratio_samps] - int_wfm.wave[start_index[i]])/pulse_integral;
	}

        is_s1_like_vec.push_back(false); //initialize
	is_start_shifted_vec.push_back(false); //initialize
	start_shift_vec.push_back(0.); //initialize
	if (ratio1 > 0.05 && ratio2 < 0.02)
	{   //Pulse looks like S1
            is_s1_like_vec[i] = true;
	    int max_sep_samps = (int)(max_s1_peak_sep_time*bs_wfm.sample_rate);
	    int default_sep_samps = (int)(default_peak_sep_time*bs_wfm.sample_rate);
	    if (std::abs(start_index[i] - peak_index) > max_sep_samps)
	    {    //Move start index closer to peak
                is_start_shifted_vec[i] = true;
                int new_start_index = peak_index - default_sep_samps;
                if(bs_wfm.sample_rate) start_shift_vec[i] = (new_start_index - start_index[i])/bs_wfm.sample_rate;
                start_index[i] = new_start_index;
	    }
	}

        ratio1_vec.push_back(ratio1);
        ratio2_vec.push_back(ratio2);
    }
}
