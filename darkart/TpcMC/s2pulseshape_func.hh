#ifndef darkart_TpcMC_s2pulseshape_func_hh
#define darkart_TpcMC_s2pulseshape_func_hh

#include <cmath> /* exp, erf, erfc. MUST COMPILE WITH C++11 !!! */

namespace darkart {

  //-----------------------------------------------------------------
  double uniform(double* x, double* params)
  {
    double t = x[0];
    double a = params[0];
    double b = params[1];

    if ( t>a && t<b && a!=b )
      return 1./(b-a);
    else
      return 0;
  }

  //-----------------------------------------------------------------
  double two_comp_expon(double* x, double* params)
  {
    double t = x[0];
    double tau1 = params[0];
    double tau2 = params[1];
    double p = params[2];

    if (t<0)
      return 0;
    else
      return (p/tau1 * std::exp(-t/tau1) + (1.-p)/tau2 * std::exp(-t/tau2));
  }


  //-----------------------------------------------------------------
  double s2pulseshape_basic(double* x, double* params)
  {
    double t = x[0];
    double tau1 = params[0];
    double tau2 = params[1];
    double p = params[2];
    double T = params[3];

    if (t<0)
      return 0;
    else if (t >=0 && t <= T)
      return 1./T * (1. - p*std::exp(-t/tau1) - (1.-p)*std::exp(-t/tau2));
    else
      return 1./T * ( (   p)*std::exp(-t/tau1)*(std::exp(T/tau1)-1) +
                      (1.-p)*std::exp(-t/tau2)*(std::exp(T/tau2)-1) );

  }


  //-----------------------------------------------------------------
  // Function and subfunctions for S2 pulse shape

  double s2_simp_f2(double* x, double* params)
  {
    long double t = x[0];
    long double tau = params[0];
    long double sigma = params[1];

    return (double) (std::erf(t/std::sqrt(2.)/sigma) -
                     std::exp(-t/tau + sigma*sigma/2/tau/tau) * std::erfc((sigma*sigma-t*tau)/(std::sqrt(2.)*sigma*tau)));

  }

  double s2_simp_f1(double* x, double* params)
  {
    double t = x[0];
    double tau = params[0];
    double T = params[1];
    double sigma = params[2];
  
    double xx[1];
    xx[0] = t-T;

    double subparams[2];
    subparams[0] = tau;
    subparams[1] = sigma;
  

  
    return 1./(2.*T)*(s2_simp_f2(x, subparams) - s2_simp_f2(xx, subparams));
  }

  double s2_simp(double* x, double* params)
  {
    double tau1 = params[0];
    double tau2 = params[1];
    double p = params[2];
    double T = params[3];
    double sigma = params[4];

    double subparams1[3];
    subparams1[0] = tau1;
    subparams1[1] = T;
    subparams1[2] = sigma;
  
    double subparams2[3];
    subparams2[0] = tau2;
    subparams2[1] = T;
    subparams2[2] = sigma;

    return (  p    * s2_simp_f1(x,subparams1) +
              (1.-p) * s2_simp_f1(x,subparams2) );
  }

  double s2pulseshape(double* x, double* params)
  {
    //double tau1 = params[0];
    //double tau2 = params[1];
    //double p = params[2];
    //double T = params[3];
    //double sigma = params[4];
    double amp = params[5];
    double t0 = params[6];
    double offset = params[7];

    double tt[1];
    tt[0] = x[0] - t0;
  
    return (offset + amp*s2_simp(tt, params));
  }

}


#endif 
