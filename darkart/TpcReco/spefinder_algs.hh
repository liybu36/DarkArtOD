#ifndef darkart_TpcReco_spefinder_algs_hh
#define darkart_TpcReco_spefinder_algs_hh


#include "fhiclcpp/ParameterSet.h"

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/Pulse.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/Spe.hh"

#include <vector>

namespace darkart
{
  struct SpeFinderParams {
    SpeFinderParams() {}
    SpeFinderParams(fhicl::ParameterSet const& p):
      max_npe(p.get<double>("max_npe")),
      energy_dep_start_time(p.get<bool>("energy_dep_start_time")),
      search_start_time(p.get<double>("search_start_time")),
      search_end_time(p.get<double>("search_end_time")),
      rough_threshold(p.get<double>("rough_threshold")),
      rough_threshold_ch_overrides(p.get<std::vector<std::vector<double> > >("rough_threshold_ch_overrides",
                                                                             std::vector<std::vector<double> >())),
      threshold_fraction(p.get<double>("threshold_fraction")),
      return_fraction(p.get<double>("return_fraction")),
      fine_threshold(p.get<double>("fine_threshold")),
      fine_threshold_ch_overrides(p.get<std::vector<std::vector<double> > >("fine_threshold_ch_overrides",
                                                                            std::vector<std::vector<double> >())),
      baseline_window(p.get<double>("baseline_window")),
      pre_threshold(p.get<double>("pre_threshold")),
      post_threshold(p.get<double>("post_threshold")),
      max_photons(p.get<int>("max_photons"))
    { }

    //parameters
    double max_npe; ///<Max number of total photoelectrons in pulses to consider.
    bool energy_dep_start_time; ///< Use energy dependent search start time instead of fixed value given by 'search_start_time'
    double search_start_time; ///< Time from start of pulse to begin search [us]
    double search_end_time;   ///< Time from start of pulse to end search [us]
    double rough_threshold; ///< ADC threshold relative to global baseline
    std::vector<std::vector<double> > rough_threshold_ch_overrides; ///< overrides specific channels
    double threshold_fraction; ///< fraction of threshold for nearby samples
    double return_fraction; ///< fraction of threshold for return to baseline
    double fine_threshold; ///< ADC threshold relative to local baseline
    std::vector<std::vector<double> > fine_threshold_ch_overrides; ///< overrides specific channels
    double baseline_window; ///< Time in us to evaluate local baseline before/after pulse
    double pre_threshold; ///< Time in us before pulse to add to spe integration window
    double post_threshold; ///< Time in us after pulse to add to spe integration window
    int max_photons; ///< maximum number of photons to find before exiting

  };

  darkart::SpeVec findSPEs(darkart::Channel const& ch,
			   darkart::Waveform const& bswfm,
			   darkart::Waveform const& intWfm,
                           darkart::PulseVec const& pulseVec,
			   //vector< Ptr<darkart::Pulse>> pulseVec,
			   double npeOfFirstPls,
			   darkart::SpeFinderParams const& params);
  
  bool IsPulse(int sample, std::vector <double> const& wave, double rough_threshold );
  bool IsSecondPulse(int sample, std::vector <double> const& wave, double fine_threshold);
}


#endif
