#include "darkart/TpcReco/averagewaveforms_algs.hh"


#include <algorithm>


void darkart::add_waveform(darkart::Waveform const& wfm,
                           TGraphErrors* gr,
                           double const start_time,
                           double const end_time,
                           double const bin_size,
                           double const spe_mean)
{
  int start_samp = wfm.TimeToSample(start_time, true);
  int end_samp = wfm.TimeToSample(end_time, true);
  int nsamps = (end_samp - start_samp)/bin_size;
  double* y = gr->GetY();
  double* ey = gr->GetEY();
  int index;
  double yj;
  int j;
  // loop over the samples to construct the average waveform
  for(int i=0; i < nsamps; i++) {
    index = start_samp+i*bin_size;

    yj=0;
    //Loop over corresponding samples in channel waveform (possibly finer binning than average waveform)
    for (j=0; j<bin_size; j++) {
      yj = yj - wfm.wave[index+j]/spe_mean;
    }
    y[i] += yj;
    
    // Add variances, taking overall sqrt when finalize
    ey[i] += fabs(yj);
  }//end loop over samples
}
