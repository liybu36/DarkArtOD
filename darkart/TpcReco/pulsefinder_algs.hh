#ifndef darkart_TpcReco_pulsefinder_algs_hh
#define darkart_TpcReco_pulsefinder_algs_hh

#include "fhiclcpp/ParameterSet.h"

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/Baseline.hh"
#include "darkart/Products/Pulse.hh"

#include <vector>

namespace darkart
{
  ///////////////////////////////////////////////////////////////////////////////////////
  ///////////////////      CURVATURE SEARCH ALGORITHM       /////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  
    struct CurvatureSearchParams
    {
	int down_sample_factor;                 ///< reduce the integral vector size by this factor
	double pulse_start_curvature;           ///< curvature threshold to start a new pulse
	double amplitude_start_threshold;       ///< before broad integral search, look for signal above this threshold
	int pile_up_curvature;                  ///< curvature threshold to start a pileup pulse 
        double s1_ratio1_threshold;             ///< threshold on ratio1 to estimate if pulse is S1
        double s1_pulse_end_window;             ///< window length for S1 pulses to evaluate pulse_end_threshold [us]
        double s2_pulse_end_window;             ///< window length for S2 pulses to evaluate pulse_end_threshold [us]
        //int pulse_end_window;                   ///< window length to evaluate pulse_end_threshold
        double pile_up_start_threshold;         ///< raw signal must change by this at actual pileup start
        double s1_pulse_end_threshold;          ///< maximum number of p.e./us in pulse_end_window to end S1 pulse
        double s2_pulse_end_threshold;          ///< maximum number of p.e./us in pulse_end_window to end S2 pulse
        //double pulse_end_threshold;             ///< maximum number of p.e. in pulse_end_window to end pulse
	double pulse_end_slope;                 ///< slope threshold to end a pulse
	
	CurvatureSearchParams(){}
        CurvatureSearchParams(fhicl::ParameterSet const& p):
            down_sample_factor(p.get<int>("down_sample_factor")),
            pulse_start_curvature(p.get<double>("pulse_start_curvature")),
            amplitude_start_threshold(p.get<double>("amplitude_start_threshold")),
            pile_up_curvature(p.get<int>("pileup_start_curvature")),
	    s1_ratio1_threshold(p.get<double>("s1_ratio1_threshold")),
	    s1_pulse_end_window(p.get<double>("s1_pulse_end_window")),
	    s2_pulse_end_window(p.get<double>("s2_pulse_end_window")),
	    //pulse_end_window(p.get<int>("pulse_end_window")),
	    pile_up_start_threshold(p.get<double>("pile_up_start_threshold")),
	    s1_pulse_end_threshold(p.get<double>("s1_pulse_end_threshold")),
	    s2_pulse_end_threshold(p.get<double>("s2_pulse_end_threshold")),
	    //pulse_end_threshold(p.get<double>("pulse_end_threshold")),
	    pulse_end_slope(p.get<double>("pulse_end_slope"))
            { }
    };
    
    darkart::PulseVec curvatureSearch(darkart::Waveform const& bs_wfm,
				      darkart::Waveform const& int_wfm,
				      double spe_mean,
				      double sample_rate,
				      darkart::CurvatureSearchParams const& params);
    
    void s1StartAdjustment(darkart::Waveform const& bs_wfm,
			   darkart::Waveform const& int_wfm,
			   std::vector<int>& start_index, std::vector<int>& end_index,
                           std::vector<bool>& is_s1_like_vec,
			   std::vector<double>& ratio1_vec, std::vector<double>& ratio2_vec,
			   std::vector<bool>& is_start_shifted_vec, std::vector<double>& start_shift_vec);
}
#endif
