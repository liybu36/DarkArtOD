/*

  Some common functions usable by multiple modules in DarkArt

  AFan 2013-07-29

 */


#ifndef darkart_TpcReco_common_hh
#define darkart_TpcReco_common_hh

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"

#include <vector>
#include <climits>

namespace darkart
{ 
  darkart::WaveformInfo fillWaveformInfo(darkart::Waveform const& wfm);


  // Find the sample in the wfm at which the MINIMUM occurs
  // Look only between start_idx and end_idx
  int findMinSamp(darkart::Waveform const& wfm,
                  std::size_t start_idx=0,
                  std::size_t end_idx=UINT_MAX);

  // Find the sample in the wfm at which the MAXIMUM occurs
  // Look only between start_idx and end_idx
  int findMaxSamp(darkart::Waveform const& wfm,
                  std::size_t start_idx=0,
                  std::size_t end_idx=UINT_MAX);
  // Find the MINIMUM of wfm
  // Look only between start_idx and end_idx
  double findMin(darkart::Waveform const& wfm,
		 std::size_t start_idx=0,
		 std::size_t end_idx=UINT_MAX);

  // Find the MAXIMUM of wfm
  // Look only between start_idx and end_idx
  double findMax(darkart::Waveform const& wfm,
		 std::size_t start_idx=0,
		 std::size_t end_idx=UINT_MAX);

  // Integrate a wfm between start_idx and end_idx
  double integrate(darkart::Waveform const& wfm,
                   std::size_t start_idx=0,
                   std::size_t end_idx=UINT_MAX);

  // Calculate an integral using the cumulative waveform
  double integral(darkart::Waveform const& integral,
                  std::size_t start_idx=0,
                  std::size_t end_idx=UINT_MAX);


}




#endif
