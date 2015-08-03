#include "darkart/TpcReco/common_algs.hh"

#include <limits>
#include <algorithm>
#include <numeric>



//----------------------------------------------------------------------------
darkart::WaveformInfo darkart::fillWaveformInfo(darkart::Waveform const& wfm)
{
  darkart::WaveformInfo wfmInfo;
  if (!wfm.isValid())
    return wfmInfo;

  
  wfmInfo.min_index = wfmInfo.max_index = 0;
  wfmInfo.maximum = wfmInfo.minimum = wfm.wave[0];
  for (size_t samp = 0, sz = wfm.wave.size(); samp < sz; ++samp) {
    if (wfm.wave[samp] < wfmInfo.minimum) {
      wfmInfo.min_index = samp;
      wfmInfo.minimum = wfm.wave[samp];
    }
    if (wfm.wave[samp] > wfmInfo.maximum) {
      wfmInfo.max_index = samp;
      wfmInfo.maximum = wfm.wave[samp];
    }
  }
   
  wfmInfo.min_time = wfm.SampleToTime(wfmInfo.min_index);
  wfmInfo.max_time = wfm.SampleToTime(wfmInfo.max_index);
  wfmInfo.nsamps = wfm.wave.size();
 

  return std::move(wfmInfo);

  
}


//----------------------------------------------------------------------------
int
darkart::findMinSamp(darkart::Waveform const& wfm, std::size_t start_idx, std::size_t end_idx)
{
  if (!wfm.isValid())
    return -1;

  if (end_idx > wfm.wave.size())
    end_idx = wfm.wave.size();
  
  return std::min_element(wfm.wave.begin()+start_idx, wfm.wave.begin()+end_idx) - wfm.wave.begin();
}

//----------------------------------------------------------------------------
int
darkart::findMaxSamp(darkart::Waveform const& wfm, std::size_t start_idx, std::size_t end_idx)
{
  if (!wfm.isValid())
    return -1;

  if (end_idx > wfm.wave.size())
    end_idx = wfm.wave.size();
  
  return std::max_element(wfm.wave.begin()+start_idx, wfm.wave.begin()+end_idx) - wfm.wave.begin();
}

double
darkart::findMin(darkart::Waveform const& wfm, std::size_t start_idx, std::size_t end_idx)
{
  if (!wfm.isValid())
    return -1;

  if (end_idx > wfm.wave.size())
    end_idx = wfm.wave.size();
  
  return *std::min_element(wfm.wave.begin()+start_idx, wfm.wave.begin()+end_idx);
}

//----------------------------------------------------------------------------
double
darkart::findMax(darkart::Waveform const& wfm, std::size_t start_idx, std::size_t end_idx)
{
  if (!wfm.isValid())
    return -1;

  if (end_idx > wfm.wave.size())
    end_idx = wfm.wave.size();
  
  return *std::max_element(wfm.wave.begin()+start_idx, wfm.wave.begin()+end_idx);
}


//----------------------------------------------------------------------------
double
darkart::integrate(darkart::Waveform const& wfm, std::size_t start_idx, std::size_t end_idx)
{
  if (!wfm.isValid())
    return 0;


  // 2013-10-02 -- Integration from index1 to index2 is really index1+1 to index2, inclusive.
  // This is to match genroot. This may cause some of the tests to fail. That's ok for now.
  // TODO: change this to the proper once we're sure genroot is ported to darkart.
  
  if (end_idx > wfm.wave.size())
    //end_idx = wfm.wave.size();
    end_idx = wfm.wave.size()-1;

  //return std::accumulate(wfm.wave.begin()+start_idx, wfm.wave.begin()+end_idx, 0.0);
  return std::accumulate(wfm.wave.begin()+start_idx+1, wfm.wave.begin()+end_idx+1, 0.0);
}

//----------------------------------------------------------------------------
double
darkart::integral(darkart::Waveform const& wfm, std::size_t start_idx, std::size_t end_idx)
{
  if (!wfm.isValid())
    return 0;

  // 2013-10-02 -- Integration from index1 to index2 is really index1+1 to index2, inclusive.
  // This is to match genroot. This may cause some of the tests to fail. That's ok for now.
  // TODO: change this to the proper indices once we're sure genroot is ported to darkart.

  if (end_idx >= wfm.wave.size())
    end_idx = wfm.wave.size()-1;

  return wfm.wave[end_idx]-wfm.wave[start_idx];
  
}




