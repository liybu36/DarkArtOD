#ifndef darkart_TpcReco_roifinder_algs_hh
#define darkart_TpcReco_roifinder_algs_hh

#include "fhiclcpp/ParameterSet.h"

#include "TCanvas.h"
#include "TFitResultPtr.h"

#include "darkart/TpcReco/RootGraphics.hh"
#include "darkart/Products/LaserSpectrum.hh"
#include "TLine.h"
#include "TGraph.h"
#include <map>

namespace darkart
{
    
    struct laserEvalParams 
    {
	laserEvalParams() {}
	
	laserEvalParams(fhicl::ParameterSet const& p):
	    amplitude_frac_threshold(p.get<double>("amplitude_frac_threshold")),
	    int_shift_correction(p.get<bool>("int_shift_correction")),
	    int_shift_fit_min(p.get<double>("int_shift_fit_min")),
	    int_shift_fit_max(p.get<double>("int_shift_fit_max")),
	    int_shift_chi2_fit_range(p.get<double>("int_shift_chi2_fit_range")),
	    amp_shift_correction(p.get<bool>("amp_shift_correction")),
	    amp_shift_fit_min(p.get<double>("amp_shift_fit_min")),
	    amp_shift_fit_max(p.get<double>("amp_shift_fit_max")),
	    amp_shift_chi2_fit_range(p.get<double>("amp_shift_chi2_fit_range"))
	    { }

	double amplitude_frac_threshold;
	bool int_shift_correction;
	double int_shift_fit_min;
	double int_shift_fit_max;
	double int_shift_chi2_fit_range;
	bool amp_shift_correction;
	double amp_shift_fit_min;
	double amp_shift_fit_max;
	double amp_shift_chi2_fit_range;
	
    };
    
    void draw_laser_spectra(LaserSpectrumMap & spectra, TCanvas & can, darkart::RootGraphics &gr, std::map<int,int> &ch_pad_map);
    void draw_integral_spectra(LaserSpectrumMap & spectra, TCanvas & can2, darkart::RootGraphics &gr);
    void draw_amplitude_spectra(LaserSpectrumMap & spectra, TCanvas & can2, darkart::RootGraphics &gr);
    void draw_subtracted_integral_spectra(LaserSpectrumMap & spectra, TCanvas & can2, darkart::RootGraphics &gr);
    void draw_subtracted_amplitude_spectra(LaserSpectrumMap & spectra, TCanvas & can2, darkart::RootGraphics &gr);
    void print_laser_fit_results(LaserSpectrumMap & spectra);
    void eval_laser_spectrum(LaserSpectrum& ls, laserEvalParams config);
    double hist_shift_chi2 (const TH1F& hist1, const TH1F& hist2, 
			    const int bin_shift, const int min_bin, const int max_bin);
    void hist_shift(TH1F& hist1, TH1F& hist2, 
		    const double min, const double max,
		    TGraph& chi2_graph, 
		    double& shift, double& shift_unc2,
		    double chi2_fit_range);
     void hist_shift_gauss_fit(TH1F& hist1, TH1F& hist2, 
			       const double min, const double max,
			       double& shift, double& shift_unc2);
}


#endif
