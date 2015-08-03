/*



 */

#ifndef darkart_ODProducts_SumWF_hh
#define darkart_ODProducts_SumWF_hh

#include <vector>
#include <utility>
#include <functional>
//#include "darkart/ODReco/Tools/Utilities.hh"
//#include "darkart/ODProducts/RunInfo.hh"

// Forward declarations
namespace darkart {
  namespace od {
    struct SumWFSample;
    typedef std::vector<SumWFSample> SumWF;
    typedef std::pair<int,int> ChPulsePair;
    typedef std::vector<ChPulsePair> ChPulsePairVec;

    
    // zero a SumWF
    //    void zeroSumWF(SumWF & wf, int nsamps,double,double,double);
  }
}


struct darkart::od::SumWFSample
{
  
  SumWFSample()
    : sample(-999)
    , sample_ns(-999.)
    , amplitude(-999.)
    , amplitude_discr(-1) // we put unphysical value as default. Before using the SumWF pass through it and change -1 to 0
    , type(0)
    , ch_pulse_start_pairs()
    , ch_pulse_end_pairs()
  {}
  
  int sample;
  float sample_ns;
  float amplitude;
  int amplitude_discr;
  int type; //0: default 1: LSV, 2: WT, -1: disabled LSV, -2: disabled WT
  ChPulsePairVec ch_pulse_start_pairs;  //pair<ch_id, pulse_id>, id's of pulses starting at a given sample
  ChPulsePairVec ch_pulse_end_pairs;  //pair<ch_id, pulse_id>, id's of pulses ending at a given samplex

  //  darkart::od::RunInfo *run_info;
};

/*
inline void darkart::od::zeroSumWF(darkart::od::SumWF & wf, int nsamps, double sample_rate_Hz, double record_length, double reference_pos)
{
  // set all entries in SumWF to zero.
  wf.resize(nsamps);
  for (int samp = 0; samp<nsamps; ++samp) {
    wf[samp].amplitude = 0;
    wf[samp].ch_pulse_start_pairs.clear();
    wf[samp].ch_pulse_end_pairs.clear();
    wf[samp].amplitude_discr = 0;
    wf[samp].sample_ns = (samp-reference_pos*record_length)/(sample_rate_Hz*1e-9);
  }
}
*/
 /*
template<typename Lambda>
//inline void darkart::od::zeroSumWF(darkart::od::SumWF & wf, int nsamps, std::function<float(int)> sampToNsFunc[])
inline void darkart::od::zeroSumWF(darkart::od::SumWF & wf, int nsamps, Lambda&& sampToNsFunc)
{
  // set all entries in SumWF to zero.
  wf.resize(nsamps);
  for (int samp = 0; samp<nsamps; ++samp) {
    wf[samp].amplitude = 0;
    wf[samp].ch_pulse_start_pairs.clear();
    wf[samp].ch_pulse_end_pairs.clear();
    wf[samp].amplitude_discr = 0;
    wf[samp].sample_ns = sampToNsFunc(samp);
  }
}
 */

#endif
