////////////////////////////////////////////////////////////////////////
// Class:       ClusterShapeEvaluator
// Module Type: Submodule of ClusterEvaluator
// File:        ClusterShapeEvaluator.cc
//
// Generated at Tue Feb  5 12:58:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

/*
This is a submodule of ClusterEvaluator
Parameters are set by parameter list: shapeparams
Required Parameters:
  - (none)
Optional Parameters:
  - verbosity
*/

#include "darkart/ODReco/Submodules/ClusterShapeEvaluator.hh"
#include <iostream> 
#include <cmath>

using namespace std;
// Constructors
ClusterShapeEvaluator::ClusterShapeEvaluator():
  _verbosity(0)
{;}
ClusterShapeEvaluator::ClusterShapeEvaluator(int nsamps):
  _verbosity(0)
{
  nsamps += 0;
}

ClusterShapeEvaluator::ClusterShapeEvaluator(fhicl::ParameterSet const &p):
  _verbosity(0)
  ,_mode          (p.get<std::string>( "mode"     ))
  ,_threshold     (p.get<float>      ( "threshold",300e6))
  ,_channel_type  (p.get<int>        ( "channel_type",1 ))
{}

void ClusterShapeEvaluator::eval(const darkart::od::SumWF& wf,
       darkart::od::ClusterVec& cluster_vec, size_t cl_idx){

  //Get some constants [1 for LSV; 2 for WT]
  const double sample_rate_Hz = _utils->getSampleRateHz(_channel_type);
  
  //Create Shape struct inside the cluster
  darkart::od::Cluster::Shape* shape = new darkart::od::Cluster::Shape();
  
  //read parameters from the existing cluster_vec 
  const float ene              = cluster_vec.at(cl_idx).charge;
  // TODO: if ene <= 0 -> give default unphysical value to shape variables and print a warning
  const float height           = cluster_vec.at(cl_idx).height;
 // const float start_ns         = cluster_vec.at(cl_idx).start_ns;
  const float start_sample     = cluster_vec.at(cl_idx).start_sample;
  const float end_sample       = cluster_vec.at(cl_idx).end_sample;
  const float peak_sample      = cluster_vec.at(cl_idx).peak_sample;
 
  //create new parameters for Cluster::Shape struct 
  double  mean_time_sample  = 0.; 
  double  sigma_mean        = 0.;
  double  sigma_peak        = 0.;
  double  skewness          = 0.;
  double  skewness_max      = 0.;
  double  skewness_10to90   = 0.;
  double  kurtosis          = 0.;
  int     ampl10_sample     = 0., ampl90_sample   = 0.;
  int     charge10_sample   = 0., charge90_sample = 0.;
  int     samples_above_thr = 0.; 
  float   f30               = 0.;
  float   f50               = 0.;
  float   f60               = 0.;
  float   tail50_to_total   = 0.;
  float   tail60_to_total   = 0.;

  //Useful variables  
  float        charge_fraction = 0.;
  double       sum_amplitude  = 0.;
  float const  sample_30ns    = std::floor( 30.*sample_rate_Hz*1.e-9);//_utils -> nsToSample(50., _channel_type);
  float const  sample_50ns    = std::floor( 50.*sample_rate_Hz*1.e-9);//_utils -> nsToSample(100., _channel_type);
  float const  sample_60ns    = std::floor( 60.*sample_rate_Hz*1.e-9);//_utils -> nsToSample(200., _channel_type);
  int          nsamples       = 0;

  const float ampl10 = 0.1*height;
  const float ampl90 = 0.9*height;
  const float charge10 = 0.1*ene;
  const float charge90 = 0.9*ene;

  for(int current_sample = start_sample; current_sample <= end_sample ; current_sample++){

    nsamples++;   
    const double wf_ampl  = wf.at(current_sample).amplitude;
    
    // variable: time above threshold
    if (wf_ampl >= _threshold) samples_above_thr++;

    if (wf_ampl < 0.) continue; //wf_ampl = 0.;

    // variable: mean time -> wighted mean on the amplitude of the cluster
    mean_time_sample  += current_sample * wf_ampl;
    sum_amplitude += wf_ampl;

    // variable: rise time and time corresponding to 10% and 90% of the maximum amplitude
    if((wf_ampl < ampl10) && (current_sample<peak_sample)){  
      ampl10_sample = current_sample;
    }
    if((wf_ampl < ampl90) && (current_sample<peak_sample)){
      ampl90_sample = current_sample;
    }

    // variable: relevant time -> deta t 90% - 10% for charge
    if(charge_fraction/sample_rate_Hz < charge10){
      charge_fraction += wf_ampl;
      charge10_sample = current_sample;
    }  
    if(charge_fraction/sample_rate_Hz < charge90){
      charge_fraction += wf_ampl;
      charge90_sample = current_sample;
    }


    // variable: f30, f50 and f60
    if(nsamples <= sample_30ns)  f30  += wf_ampl;
    if(nsamples <= sample_50ns)  f50  += wf_ampl;
    if(nsamples <= sample_60ns)  f60  += wf_ampl;

    // variable: tail to total (at 50ns and 60ns)
    if(nsamples >= sample_50ns) tail50_to_total += wf_ampl;
    if(nsamples >= sample_60ns) tail60_to_total += wf_ampl;
  }

  //Calculate parameters, convert from sample to ns and normalize
  mean_time_sample = (sum_amplitude > 0) ? (mean_time_sample/sum_amplitude) : -1;
  // TODO: print warning if sum_amplitude <= 0
  const float rise_10ampl_ns    = (ampl10_sample - start_sample)*1.e9/sample_rate_Hz;
  const float rise_90ampl_ns    = (ampl90_sample - start_sample)*1.e9/sample_rate_Hz;
  const float time_10ampl_ns    = _utils->sampleToNs(ampl10_sample, _channel_type);
  const float time_90ampl_ns    = _utils->sampleToNs(ampl90_sample, _channel_type);
  const float time_above_thr_ns = samples_above_thr*1.e9/sample_rate_Hz;
  f30              /= (sample_rate_Hz*ene);
  f50              /= (sample_rate_Hz*ene);
  f60              /= (sample_rate_Hz*ene);
  tail50_to_total  /= (sample_rate_Hz*ene);
  tail60_to_total  /= (sample_rate_Hz*ene);

  // complete the moments of the cluster pdf, weighted on amplitude
  for(int current_sample = start_sample; current_sample <= end_sample ; current_sample++){
    const double wf_ampl  = wf.at(current_sample).amplitude;
    if (wf_ampl < 0)  continue;
    const double bpeak = current_sample - peak_sample;
    sigma_mean += wf_ampl * (current_sample - mean_time_sample) * (current_sample - mean_time_sample);
    sigma_peak += wf_ampl * bpeak * bpeak ;
    skewness   += wf_ampl * bpeak * bpeak * bpeak;
    kurtosis   += wf_ampl * bpeak * bpeak * bpeak * bpeak;  
    if((current_sample > charge10_sample) && (current_sample < charge90_sample))  
      skewness_10to90 += wf_ampl * bpeak * bpeak * bpeak;
  }

  int  current_sample = peak_sample -1;
  double wf_ampl = 0.;
  do{
    wf_ampl  = wf.at(current_sample).amplitude;
    skewness_max   += wf_ampl * std::pow(current_sample - peak_sample,3);
    if((wf_ampl>0) && (current_sample<peak_sample)){
      current_sample--;
    }else if((wf_ampl<=0) && (current_sample<peak_sample)){
      current_sample = peak_sample+1;
      wf_ampl  = wf.at(current_sample).amplitude;
    }else if((wf_ampl>0) && (current_sample>peak_sample)){
      current_sample++;
    }
  }while(wf_ampl>0);

  const float charge10_time_ns  = (charge10_sample - start_sample)*1.e9/sample_rate_Hz; 
  const float charge90_time_ns  = (charge90_sample - start_sample)*1.e9/sample_rate_Hz; 
  const float mean_time_ns      = (mean_time_sample - start_sample)*1.e9/sample_rate_Hz;
  sigma_mean       /= sum_amplitude;
  sigma_mean        = (sigma_mean > 0) ? (std::sqrt(sigma_mean)*1.e9/sample_rate_Hz) : -1.;
  sigma_peak       /= (end_sample - start_sample -1);
  sigma_peak        = (sigma_peak > 0) ? (std::sqrt(sigma_peak)*1.e9/sample_rate_Hz) : -1.;
  skewness         /= (std::pow(sigma_peak, 3./2.)*1.e6); // now sigma is in ns; is *1e6 still necessary?
  skewness_max     /= (std::pow(sigma_peak, 3./2.)*1.e6);
  skewness_10to90  /= (std::pow(sigma_peak, 3./2.)*1.e6);
  kurtosis         /= (std::pow(sigma_peak, 4./2)) - 3.;

  //Assign values to shape parameters
  shape->mean_time_ns      = mean_time_ns;        // first moment of the cluster pdf, weighted on amplitude
  shape->sigma_mean_ns     = sigma_mean;          // second moment 
  shape->sigma_peak_ns     = sigma_peak;          // second moment 
  shape->skewness          = skewness;            // third moment
  shape->skewness_max      = skewness_max;        // third moment
  shape->skewness_10to90   = skewness_10to90;     // third moment
  shape->kurtosis          = kurtosis;            // fourth moment
  shape->a10_time_ns       = time_10ampl_ns;      // time when the amplitude is 10% of max_amplitude
  shape->a90_time_ns       = time_90ampl_ns;      // time when the amplitude is 90% of max_amplitude
  shape->rise10_time_ns    = rise_10ampl_ns;      // a10_time - start_time
  shape->rise90_time_ns    = rise_90ampl_ns;      // a90_time - start_time
  shape->above_thr_time_ns = time_above_thr_ns;   // time above threshold
  shape->c10_time_ns       = charge10_time_ns;    // time when the charge is 10% of the total charge
  shape->c90_time_ns       = charge90_time_ns;    // time when the charge is 90% of the total charge
  shape->f30               = f30;                 // fractional charge in first  50 ns
  shape->f50               = f50;                 // fractional charge in first  100 ns
  shape->f60               = f60;                 // fractional charge in first  200 ns
  shape->tail50_to_total   = tail50_to_total;     // fractional charge after 20 ns (for alpha beta discrimination is the tail that change!)
  shape->tail60_to_total   = tail60_to_total;     // fractional charge after 20 ns (for alpha beta discrimination is the tail that change!)

  //Write the shape inside the new evaluated_cluster_vec 
  cluster_vec.at(cl_idx).shape = std::move(shape);

}
