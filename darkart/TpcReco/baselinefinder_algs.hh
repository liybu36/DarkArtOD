#ifndef darkart_TpcReco_baselinefinder_algs_hh
#define darkart_TpcReco_baselinefinder_algs_hh

#include "fhiclcpp/ParameterSet.h"

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/Baseline.hh"
#include "darkart/Products/Channel.hh"

#include <vector>
#include <map>


namespace darkart
{

  ///////////////////////////////////////////////////////////////////////////////////////
  //////////////////      DRIFTING BASELINE ALGORITHM       /////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////

  struct DriftingBaselineParams 
  {
      DriftingBaselineParams() {}
      DriftingBaselineParams(fhicl::ParameterSet const& p):
          baseline_mean_window(p.get<double>("baseline_mean_window", 5)),
	  signal_begin_time(p.get<double>("signal_begin_time")),
	  start_max_amplitude(p.get<double>("start_max_amplitude")),
	  start_max_amplitude_ch_overrides(p.get<std::vector<std::vector<double> > >
					   ("start_max_amplitude_ch_overrides",
					    std::vector<std::vector<double> >())),
	  end_max_amplitude(p.get<double>("end_max_amplitude")),
	  end_max_amplitude_ch_overrides(p.get<std::vector<std::vector<double> > >
					   ("end_max_amplitude_ch_overrides",
					    std::vector<std::vector<double> >())),
	  pre_samps(p.get<int>("pre_samps")),
	  post_samps(p.get<int>("post_samps")),
	  interpolate_regions(p.get<bool>("interpolate_regions")),
	  region_times(p.get<std::vector<std::vector<double> > >("region_times", std::vector<std::vector<double> >()))
	  {}
      size_t numRegions() const { return region_times.size();}
      bool isValidRegion(size_t region) const;
      double   baseline_mean_window; //window from beginning of waveform to use for calculation baseline mean and variance
      double      signal_begin_time; //Search for baseline before this time [us]
      double          start_max_amplitude; //Max amplitude for sample to be considered in baseline when prev in baseline
      std::vector<std::vector<double> > start_max_amplitude_ch_overrides;
      double          end_max_amplitude; //Max amplitude for sample to be considered in baseline when prev in signal region
      std::vector<std::vector<double> > end_max_amplitude_ch_overrides;      
      int                 pre_samps; //Samples before to include in moving average
      int                post_samps; //Samples after to include in moving average
      bool interpolate_regions;            //Always interpolate the baseline in the defined windows?
      std::vector<std::vector<double> > region_times; //vector containing range of region times [us]
  };
    
  // Check if the given sample is within any of the defined regions
  bool isWithinRegion (int const samp, std::vector<std::vector<int> >const& region_samples); 

  //Convert region times to samples
  void convertRegionTimesToSamples(DriftingBaselineParams const& p, 
				   darkart::Waveform const & wfm,
				   std::vector<std::vector<int> >& region_samples);

  // The waveform has just returned to baseline. Interpolate the baseline in the
  // region where the waveform deviated from baseline
  void interpolateBaseline(std::vector<double> & baseform,
                           double const mean,
                           int const samp,
                           int const last_good_samp,
                           DriftingBaselineParams const& p);
  
  // Drifting baseline algorithm which calculates a moving average and interpolates
  // regions where the waveform goes beyond threshold
  void driftingBaseline(darkart::Channel const& ch,
                        darkart::Waveform const& wfm,
                        darkart::Waveform & bsWfm,
                        darkart::Baseline & baseline,
                        DriftingBaselineParams const& p);



  ///////////////////////////////////////////////////////////////////////////////////////
  /////////////////////      FIXED BASELINE ALGORITHM       /////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////

  struct FixedBaselineParams {
    FixedBaselineParams() {}
    FixedBaselineParams(fhicl::ParameterSet const& p):
      signal_begin_time(p.get<double>("signal_begin_time")),
      segment_samps(p.get<int>("segment_samps")),
      min_valid_samps(p.get<int>("min_valid_samps")),
      max_sigma(p.get<double>("max_sigma")),
      max_sigma_diff(p.get<double>("max_sigma_diff")),
      max_mean_diff(p.get<double>("max_mean_diff"))
    { }

    double  signal_begin_time; //Search for baseline before this time [us]
    int         segment_samps; //Samples in each baseline segment
    int       min_valid_samps; //Min samples for a baseline to be valid
    double          max_sigma; //Max standard deviation for a baseline segment to be accepted
    double     max_sigma_diff; //Max diff. between baseline sigma and the sigma of a new valid segment
    double      max_mean_diff; //Max diff. between baseline mean and the mean of a new valid segment
  };

  
  // Search for a flat baseline in the pre-trigger window
  void fixedBaseline(darkart::Channel const& ch,
                     darkart::Waveform const& wfm,
                     darkart::Waveform & bsWfm,
                     darkart::Baseline & baseline,
                     FixedBaselineParams const& p);
  
  
  ///////////////////////////////////////////////////////////////////////////////////////
  /////////////////////      ZERO BASELINE ALGORITHM       //////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////

  void zeroBaseline(darkart::Waveform const& wfm,
                    darkart::Waveform & bsWfm,
                    darkart::Baseline & baseline);  
}


#endif
