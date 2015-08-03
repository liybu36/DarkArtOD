#include "darkart/TpcReco/laserfit_algs.hh"
#include "bprinter/include/bprinter/table_printer.h"

#include <iostream>

#include "TMath.h"
#include "TROOT.h"
#include "TF1.h"
#include "TMatrixDSym.h"
#include "TFitResult.h"
#include "TBox.h"

#define NPEAKS 7

#define mCON params[CONSTANT]
#define mLAM params[LAMBDA]
#define mGCENTER params[GAUSSCENTER]
#define mGSIG params[GAUSSSIGMA]
#define mSLOPE params[SLOPE_E]
#define mPE params[P_E]
#define mPSG params[PEDSIGMA]


void darkart::draw_laser_spectra(LaserSpectrumMap & spectra, TCanvas & can, darkart::RootGraphics &gr, std::map<int,int> &ch_pad_map)
{
  LockGuard lock(gr.mutex);
  can.Clear();
  divide_canvas(can, spectra.size());
  int padn=1;
  for( auto spect_pair : spectra)
  {
       LaserResults res = spect_pair.second->results;
       can.cd(padn);
       ch_pad_map.insert( std::pair<int,int>(spect_pair.first.asInt(), padn) );
       padn++;
       gPad->SetLogy(1);
       spect_pair.second->Draw(true);
  }
  can.cd(0);    
  can.Draw();
  can.Update();
}

void darkart::draw_integral_spectra(LaserSpectrumMap & spectra, TCanvas & can, darkart::RootGraphics &gr)
{
  LockGuard lock(gr.mutex);
  can.Clear();
  divide_canvas(can, spectra.size());
  int padn=1;
  for( auto spect_pair : spectra)
  {
    can.cd(padn);
    padn++;
    gPad->SetLogy(1);
    spect_pair.second->DrawIntegrals();
  }
  can.cd(0);    
  can.Draw();
  can.Update();
}

void darkart::draw_amplitude_spectra(LaserSpectrumMap & spectra, TCanvas & can, darkart::RootGraphics &gr)
{
  LockGuard lock(gr.mutex);
  can.Clear();
  divide_canvas(can, spectra.size());
  int padn=1;
  for( auto spect_pair : spectra)
  {
    can.cd(padn);
    padn++;
    gPad->SetLogy(1);
    spect_pair.second->DrawAmplitudes();
  }
  can.cd(0);    
  can.Draw();
  can.Update();
}

void darkart::draw_subtracted_integral_spectra(LaserSpectrumMap & spectra, TCanvas & can, darkart::RootGraphics &gr)
{
  LockGuard lock(gr.mutex);
  can.Clear();
  divide_canvas(can, spectra.size());
  int padn=1;
  for( auto spect_pair : spectra)
  {
    can.cd(padn);
    padn++;
    spect_pair.second->results.DrawSubtractedIntegral();
  }
  can.cd(0);    
  can.Draw();
  can.Update();
}

void darkart::draw_subtracted_amplitude_spectra(LaserSpectrumMap & spectra, TCanvas & can, darkart::RootGraphics &gr)
{
  LockGuard lock(gr.mutex);
  can.Clear();
  divide_canvas(can, spectra.size());
  int padn=1;
  for( auto spect_pair : spectra)
  {
    can.cd(padn);
    padn++;
    spect_pair.second->results.DrawSubtractedAmplitude();
  }
  can.cd(0);    
  can.Draw();
  can.Update();
}

void darkart::print_laser_fit_results(LaserSpectrumMap & spectra)
{
  bprinter::TablePrinter tp(&std::cout);
  tp.AddColumn("Chan",4);
  tp.AddColumn("SER mean",8);
  tp.AddColumn("SER mean unc",12);
  tp.AddColumn("SER sigma",9);
  tp.AddColumn("Occup [%]",9);
  tp.AddColumn("Occup Unc", 9);
  tp.AddColumn("Tot mean",8);
  tp.AddColumn("Tot mean unc",12);
  tp.AddColumn("Tot var",7);
  tp.AddColumn("Ped mean",8);
  tp.AddColumn("Ped mean unc",12);
  tp.AddColumn("Ped var",7);
  tp.AddColumn("Gauss cen",9);
  tp.AddColumn("Gauss sig",9);
  tp.AddColumn("L Amp fr cor",12);
  tp.AddColumn("L Amp fr",8);
  tp.AddColumn("P Amp cor",9);
  tp.AddColumn("Amp. thr",8);
  tp.AddColumn("Int Corr.",8);
  tp.AddColumn("Amp Corr",8);
  
  tp.PrintHeader();
  for( auto spect_pair : spectra)
  {
    LaserResultsData resData = spect_pair.second->results.data;
    //mGCENTER etc are macros defined at top

    tp<< spect_pair.first << resData.ser_mean << sqrt(resData.ser_mean_unc2)
      <<resData.ser_sigma << 100*resData.occupancy << 100*sqrt(resData.occupancy_unc2) 
      << resData.total_mean << sqrt(resData.total_mean_unc2) << resData.total_var 
      << resData.pedestal_mean << sqrt(resData.pedestal_mean_unc2) << resData.pedestal_var
      << resData.gauss_center << resData.gauss_sigma
      << resData.laser_amp_frac_corr << resData.laser_amp_frac << resData.pedestal_amp_corr 
      << resData.amplitude_threshold
      << resData.int_shift_correction<< resData.amp_shift_correction;


  }
  tp.PrintFooter();

}

double darkart::hist_shift_chi2 (const TH1F& hist1, const TH1F& hist2, 
				 const int bin_shift, const int min_bin, const int max_bin)
{
    if (min_bin - bin_shift < 1 || max_bin - bin_shift > hist2.GetNbinsX())
    {
	return -1;
    }
    double chi2 = 0; int ndof = 0;
    double hist1_int = hist1.Integral(min_bin, max_bin);
    double hist2_int = hist2.Integral(min_bin - bin_shift, max_bin - bin_shift);
    double scale_factor = hist1_int/hist2_int;
    for (int i = min_bin; i <= max_bin; i++)
    {
	if (hist1.GetBinContent(i) == 0 &&  hist2.GetBinContent(i-bin_shift) == 0)
	    continue;
	chi2 += ((hist1.GetBinContent(i) - hist2.GetBinContent(i-bin_shift)*scale_factor)*
		 (hist1.GetBinContent(i) - hist2.GetBinContent(i-bin_shift)*scale_factor))/
		(hist1.GetBinError(i)*hist1.GetBinError(i) 
		 + hist2.GetBinError(i-bin_shift)*hist2.GetBinError(i-bin_shift)*scale_factor*scale_factor);
	ndof++;
    }
    return chi2/ndof; 
}

void darkart::hist_shift(TH1F& hist1, TH1F& hist2, 
			 const double min, const double max,
			 TGraph& chi2_graph, 
			 double& shift, double& shift_unc2,
			 double chi2_fit_range)
{
    int min_bin = hist1.FindBin(min);
    int max_bin = hist1.FindBin(max);
    int n_points = 2*50;
    int n_graph_points = 0;
    for (int i = -n_points/2; i <= n_points/2; i++, n_graph_points++)
    {
	double chi2 = hist_shift_chi2(hist1, hist2, i, min_bin, max_bin);
	chi2_graph.SetPoint(n_graph_points, i*hist2.GetBinWidth(1), chi2);
    }
    TF1* quad = new TF1("quad", "[0]*(x-[1])^2 + [2]");
    quad->SetLineColor(kRed);
    chi2_graph.Fit(quad, "Q");
    double center = quad->GetParameter(1);
    chi2_graph.Fit(quad, "Q", "", center - chi2_fit_range, center + chi2_fit_range);
    shift = quad->GetParameter(1);
    shift_unc2 = quad->GetParError(1)*quad->GetParError(1);
}

void darkart::hist_shift_gauss_fit(TH1F& hist1, TH1F& hist2, 
				   const double min, const double max,
				   double& shift, double& shift_unc2)
{
    TF1* gauss = new TF1("gauss", "gausn(0)");
    hist1.Fit(gauss, "QN", "", min, max);
    double hist1_peak = gauss->GetParameter(1);
    double hist1_peak_unc2 = gauss->GetParError(1)*gauss->GetParError(1);
    hist2.Fit(gauss, "QN", "", min, max);
    double hist2_peak = gauss->GetParameter(1);
    double hist2_peak_unc2 = gauss->GetParError(1)*gauss->GetParError(1);
    
    shift = hist1_peak - hist2_peak;
    shift_unc2 = hist1_peak_unc2 + hist2_peak_unc2;
}

void darkart::eval_laser_spectrum(LaserSpectrum& ls, laserEvalParams config )
{
    //Calculate moments of total charge histogram
    ls.results.data.total_mean = ls.laser_int_hist.GetMean();
    ls.results.data.total_var = ls.laser_int_hist.GetRMS()*ls.laser_int_hist.GetRMS();
    ls.results.data.total_mean_unc2 = ls.results.data.total_var/ls.laser_int_hist.GetEntries();

    //Check for shifts between laser and pedestal integral distributions
    double int_shift_correction_value = 0, int_shift_correction_unc2 = 0;
    int int_shift_correction_bins = 0;
    if (config.int_shift_correction)
    {   //Fit histograms to find shift between laser and pedestal regions
	
        //Need to rebin for greater statistics when fitting
	TH1F laser_int_hist_rb = *((TH1F*)ls.laser_int_hist.Rebin(4, "laser_int_hist_rb"));
	TH1F pedestal_int_hist_rb = *((TH1F*)ls.pedestal_int_hist.Rebin(4, "pedestal_int_hist_rb"));

	hist_shift(laser_int_hist_rb, pedestal_int_hist_rb, 
		   ls.pedestal_int_hist.GetMean() + config.int_shift_fit_min*ls.pedestal_int_hist.GetRMS(), 
		   ls.pedestal_int_hist.GetMean() + config.int_shift_fit_max*ls.pedestal_int_hist.GetRMS(), 
		   ls.results.int_shift_chi2_graph, 
		   int_shift_correction_value, int_shift_correction_unc2,
		   config.int_shift_chi2_fit_range);
	/*hist_shift_gauss_fit(ls.laser_int_hist_rb, ls.pedestal_int_hist_rb, 
			     ls.pedestal_int_hist.GetMean() + config.int_shift_fit_min*ls.pedestal_int_hist.GetRMS(), 
			     ls.pedestal_int_hist.GetMean() + config.int_shift_fit_max*ls.pedestal_int_hist.GetRMS(), 
			     int_shift_correction_value, int_shift_correction_unc2);*/
	int_shift_correction_bins = round(int_shift_correction_value/ls.pedestal_int_hist.GetBinWidth(1));
    }
    ls.results.data.int_shift_correction = int_shift_correction_value;
    ls.results.data.int_shift_correction_unc2 = int_shift_correction_unc2;
    
    //Calculate moments of pedestal
    ls.results.data.pedestal_mean = ls.pedestal_int_hist.GetMean() + int_shift_correction_value;
    ls.results.data.pedestal_var = ls.pedestal_int_hist.GetRMS()*ls.pedestal_int_hist.GetRMS();
    ls.results.data.pedestal_mean_unc2 = (ls.results.data.pedestal_var/ls.pedestal_int_hist.GetEntries() 
				     + ls.results.data.int_shift_correction_unc2);

    //Find bin corresponding to amplitude_frac_threshold
    int pedestal_amp_bin = 0;
    double cuml_int = 0, total_int = ls.pedestal_amp_hist.Integral(0, ls.pedestal_amp_hist.GetNbinsX() + 1);
    for (int bin = 1; bin < ls.pedestal_amp_hist.GetNbinsX(); bin++)
    {
	cuml_int += ls.pedestal_amp_hist.GetBinContent(bin);
	if (cuml_int/total_int > config.amplitude_frac_threshold)
	{
	    pedestal_amp_bin = bin - 1;
	    break;
	}
    }
    
    //Check for shifts between laser and pedestal amplitude distributions
    double amp_shift_correction_value = 0, amp_shift_correction_unc2 = 0;
    int amp_shift_correction_bins = 0;
    if (config.amp_shift_correction)
    {   
	 //Fit histograms to find shift between laser and pedestal regions

        //Need to rebin for greater statistics when fitting
	TH1F laser_amp_hist_rb = *((TH1F*)ls.laser_amp_hist.Rebin(4, "laser_amp_hist_rb"));
	TH1F pedestal_amp_hist_rb = *((TH1F*)ls.pedestal_amp_hist.Rebin(4, "pedestal_amp_hist_rb"));

	hist_shift(laser_amp_hist_rb, pedestal_amp_hist_rb, 
		   ls.pedestal_amp_hist.GetMean() + config.amp_shift_fit_min*ls.pedestal_amp_hist.GetRMS(), 
		   ls.pedestal_amp_hist.GetMean() + config.amp_shift_fit_max*ls.pedestal_amp_hist.GetRMS(), 
		   ls.results.amp_shift_chi2_graph, 
		   amp_shift_correction_value, amp_shift_correction_unc2,
		   config.amp_shift_chi2_fit_range);
	/*hist_shift_gauss_fit(ls.laser_amp_hist, ls.pedestal_amp_hist, 
			     ls.pedestal_amp_hist.GetMean() + config.amp_shift_fit_min*ls.pedestal_amp_hist.GetRMS(), 
			     ls.pedestal_amp_hist.GetMean() + config.amp_shift_fit_max*ls.pedestal_amp_hist.GetRMS(),  
			     amp_shift_correction_value, amp_shift_correction_unc2);*/

	amp_shift_correction_bins = round(amp_shift_correction_value/ls.pedestal_amp_hist.GetBinWidth(1));
    }
    int laser_amp_bin = pedestal_amp_bin + amp_shift_correction_bins; 
    ls.results.data.amp_shift_correction = amp_shift_correction_bins*ls.laser_amp_hist.GetBinWidth(1);

    //Calculate amplitude corresponding to amplitude_frac_threshold
    ls.results.data.amplitude_threshold =  ls.laser_amp_hist.GetBinLowEdge(laser_amp_bin + 1);

    //Calculate fraction of events without signal
    double n_laser_entries = ls.laser_amp_hist.Integral(0, ls.laser_amp_hist.GetNbinsX() + 1);
    double n_ped_entries = ls.pedestal_amp_hist.Integral(0, ls.pedestal_amp_hist.GetNbinsX() + 1);

    ls.results.data.laser_amp_frac = (ls.laser_amp_hist.Integral(0, laser_amp_bin)/n_laser_entries);
    ls.results.data.pedestal_amp_corr = (ls.pedestal_amp_hist.Integral(0, pedestal_amp_bin)/n_ped_entries);
    ls.results.data.laser_amp_frac_corr = ls.results.data.laser_amp_frac/ls.results.data.pedestal_amp_corr;
    
    
    //Calculate occupancy
    ls.results.data.occupancy = -TMath::Log(ls.results.data.laser_amp_frac_corr);
    double n_laser_baseline = ls.results.data.laser_amp_frac_corr*n_laser_entries;
    double n_laser_baseline_unc2 = (n_laser_baseline*(1 - config.amplitude_frac_threshold)*
				    (1 + n_laser_baseline/config.amplitude_frac_threshold/n_ped_entries));
    ls.results.data.occupancy_unc2 = n_laser_baseline_unc2/n_laser_baseline/n_laser_baseline;
    
    //Calculate SER moments
    ls.results.data.ser_mean = (ls.results.data.total_mean - ls.results.data.pedestal_mean)/ls.results.data.occupancy;
    ls.results.data.ser_sigma = sqrt((ls.results.data.total_var - ls.results.data.pedestal_var 
				 - ls.results.data.ser_mean*ls.results.data.ser_mean*ls.results.data.occupancy)
				/ls.results.data.occupancy);
    ls.results.data.ser_mean_unc2 = ((1./ls.results.data.occupancy/ls.results.data.occupancy)*
				(ls.results.data.total_mean_unc2 + ls.results.data.pedestal_mean_unc2 + 
				 ls.results.data.ser_mean*ls.results.data.ser_mean*ls.results.data.occupancy_unc2));

    //Save results graphically
    Double_t int_hist_min = ls.laser_int_hist.GetMinimum();
    Double_t int_hist_max = ls.laser_int_hist.GetMaximum();
    
    ls.results.ser_mean_line.SetX1(ls.results.data.ser_mean);
    ls.results.ser_mean_line.SetX2(ls.results.data.ser_mean);
    ls.results.ser_mean_line.SetY1(int_hist_min);
    ls.results.ser_mean_line.SetY2(int_hist_max);
    ls.results.ser_mean_line.SetLineColor(kRed);

    Double_t amp_hist_min = ls.laser_amp_hist.GetMinimum();
    Double_t amp_hist_max = ls.laser_amp_hist.GetMaximum();
    
    ls.results.amp_thresh_line.SetX1(ls.results.data.amplitude_threshold);
    ls.results.amp_thresh_line.SetX2(ls.results.data.amplitude_threshold);
    ls.results.amp_thresh_line.SetY1(amp_hist_min);
    ls.results.amp_thresh_line.SetY2(amp_hist_max);
    ls.results.amp_thresh_line.SetLineColor(kRed);

    //Calculate subtracted and scaled histograms
    ls.results.laser_int_sub_hist = *(TH1F*)ls.laser_int_hist.Clone("laser_int_sub_hist");
    ls.results.laser_amp_sub_hist = *(TH1F*)ls.laser_amp_hist.Clone("laser_amp_sub_hist");
    ls.results.ped_int_scaled_hist = *(TH1F*)ls.pedestal_int_hist.Clone("ped_int_scaled_hist");
    ls.results.ped_amp_scaled_hist = *(TH1F*)ls.pedestal_amp_hist.Clone("ped_amp_scaled_hist");

    double sub_scale_factor = (ls.laser_amp_hist.Integral(0, laser_amp_bin)
			       /ls.pedestal_amp_hist.Integral(0, pedestal_amp_bin));
    for (int i = 0; i <= ls.results.laser_int_sub_hist.GetNbinsX() + 1; i++)
    {
	ls.results.ped_int_scaled_hist.SetBinContent(i, ls.pedestal_int_hist.GetBinContent(i - int_shift_correction_bins)*sub_scale_factor);

	ls.results.laser_int_sub_hist.SetBinContent(i, (ls.laser_int_hist.GetBinContent(i) - 
							 ls.pedestal_int_hist.GetBinContent(i - int_shift_correction_bins)
							*sub_scale_factor));
    }
    for (int i = 0; i <= ls.results.laser_amp_sub_hist.GetNbinsX() + 1; i++)
    {
	ls.results.ped_amp_scaled_hist.SetBinContent(i, ls.pedestal_amp_hist.GetBinContent(i - amp_shift_correction_bins)*sub_scale_factor);

	ls.results.laser_amp_sub_hist.SetBinContent(i, (ls.laser_amp_hist.GetBinContent(i) - 
							 ls.pedestal_amp_hist.GetBinContent(i - amp_shift_correction_bins)
							*sub_scale_factor));
    }
    
    //Keep track of Gaussian part of SPE response to study stability
    ls.laser_int_hist.Rebin(32);
    TFitResultPtr fitres = ls.laser_int_hist.Fit("gaus","SQ","",100,250);
    ls.results.data.gauss_center = fitres->Parameter(1);
    ls.results.data.gauss_sigma = fitres->Parameter(2);
}
