#include "darkart/TpcReco/smoother_algs.hh"
#include "darkart/TpcReco/common_algs.hh"

#include <algorithm>

void darkart::smoother(darkart::Waveform const& wfm, 
                       darkart::Waveform & smoothedWfm, 
                       darkart::WaveformInfo & smoothedData, 
                       int pre_samp, int post_samp)
{
  //get/initialize data
  const int nsamps = wfm.wave.size();
  double running_sum = 0;
  double samps_in_sum = post_samp;
  smoothedWfm = wfm;

  for(int j=0 ; j<post_samp && j<nsamps; j++) running_sum += wfm.wave[j];

  for(int samp = 0; samp<nsamps ; samp++){
    if(samp < (nsamps - post_samp)){
	running_sum += wfm.wave[samp+post_samp];
	samps_in_sum++;
      }
      if(samp > pre_samp){
	running_sum -= wfm.wave[samp-pre_samp-1];
	samps_in_sum--;      
      }
      //create smoothed wfm
      smoothedWfm.wave[samp] = running_sum / samps_in_sum;
  }
    //at this point smoothedWfm != wfm (since it has been smoothed)
    smoothedData = fillWaveformInfo(smoothedWfm); //get smoothed_min and smoothed_max through this method, amongst others
}

