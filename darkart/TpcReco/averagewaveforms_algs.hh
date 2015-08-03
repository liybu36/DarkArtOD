#ifndef darkart_TpcReco_averagewaveforms_algs_hh
#define darkart_TpcReco_averagewaveforms_algs_hh

#include "darkart/Products/Waveform.hh"


#include "TGraphErrors.h"

namespace darkart
{
  void add_waveform(darkart::Waveform const& wfm,
                    TGraphErrors* gr,
                    double const start_time,
                    double const end_time,
                    double const bin_size,
                    double const spe_mean);




}







#endif
