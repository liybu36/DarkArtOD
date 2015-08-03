/*
  Definition of the MCTruth class
  2014-11-01 AFan

  Based on R. Saldanha's set of MC truth variables implemented in initial version of electronics MC
  
 */


#ifndef darkart_Products_MCTruth_hh
#define darkart_Products_MCTruth_hh

#include <vector>

namespace darkart
{
  namespace MC
  {
    struct MCTruth;
  }
}

struct darkart::MC::MCTruth
{
  //MCTruth() { ; }
  
  int event_id;
  double npe;
  double npe_fixed;
  double npe_fixed2;
  double npe_prompt;
  double npe_late;  
  double npe_late_fixed;  
  double charge;
  double charge_fixed;
  double charge_fixed2;
  double charge_prompt;
  double charge_late;  
  double charge_late_fixed;  
  double signal;
  double signal_fixed;
  double signal_fixed2;
  double signal_prompt;
  double signal_late;
  double signal_late_fixed;
  std::vector<int>   ch_id;
  std::vector< std::vector<double> > ch_pe_times;
  std::vector<float> ch_npe;
  std::vector<float> ch_npe_fixed;
  std::vector<float> ch_npe_fixed2;
  std::vector<float> ch_npe_prompt;
  std::vector<float> ch_npe_late;
  std::vector<float> ch_npe_late_fixed;

  std::vector<double> ch_signal;
  std::vector<double> ch_signal_fixed;
  std::vector<double> ch_signal_fixed2;
  std::vector<double> ch_signal_prompt;
  std::vector<double> ch_signal_late;
  std::vector<double> ch_signal_late_fixed;

  void clear() {
    event_id = -1; 
    npe=0;    npe_fixed=0;    npe_fixed2=0;    npe_prompt=0;    npe_late=0;    npe_late_fixed=0;
    charge=0; charge_fixed=0; charge_fixed2=0; charge_prompt=0; charge_late=0; charge_late_fixed=0;
    signal=0; signal_fixed=0; signal_fixed2=0; signal_prompt=0; signal_late=0; signal_late_fixed=0;
    ch_id.clear();
    ch_pe_times.clear();
    ch_npe.clear();  ch_npe_fixed.clear();  ch_npe_fixed2.clear();
    ch_npe_prompt.clear();  ch_npe_late.clear();  ch_npe_late_fixed.clear();
    ch_signal.clear();  ch_signal_fixed.clear();  ch_signal_fixed2.clear();
    ch_signal_prompt.clear();  ch_signal_late.clear();  ch_signal_late_fixed.clear();
  }

  void resize(int n);
};


inline void darkart::MC::MCTruth::resize(int n)
{
  ch_id.resize(n);
  ch_pe_times.resize(n);
  ch_npe.resize(n);  ch_npe_fixed.resize(n);  ch_npe_fixed2.resize(n);
  ch_npe_prompt.resize(n);  ch_npe_late.resize(n);  ch_npe_late_fixed.resize(n);
  ch_signal.resize(n);  ch_signal_fixed.resize(n);  ch_signal_fixed2.resize(n);
  ch_signal_prompt.resize(n);  ch_signal_late.resize(n);  ch_signal_late_fixed.resize(n);
}

#endif
