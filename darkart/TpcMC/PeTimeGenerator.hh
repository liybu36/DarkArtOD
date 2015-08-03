/////////////////////////////////////////////////////////////////////////
//
//  2014-10-22 AFan
//
//  Helper class for generating PE times according to various time profiles.
//
//  Laser:
//  - Usually generate PE at fixed time
//  - Add some timing jitter to be more realistic
//  - Jitter is gaussian
//
//  S1:
//  - Usual two-component ratio
//  - Specify fast/slow components and fast fraction
//
//  S2:
//  - S1 convolved with uniform distribution
//  - TODO: add form with Gaussian smearing; import from separate header
//
/////////////////////////////////////////////////////////////////////////


#ifndef darkart_TpcMC_PeTimeGenerator_hh
#define darkart_TpcMC_PeTimeGenerator_hh

#include <string>
#include "TRandom3.h"
#include "TF1.h"
#include "fhiclcpp/ParameterSet.h"

namespace darkart
{
  namespace MC
  {
    class PeTimeGenerator
    {
    public:
      PeTimeGenerator(fhicl::ParameterSet const& p, std::string signal_type);

      double fire(TRandom3*);

    private:
      
      std::string _signal_type;

      //---------------------------------------
      class Laser {
      public:
	Laser(fhicl::ParameterSet const&);
	double fire(TRandom3*);
      private:
	double _mean;
	double _sigma;
      } _laser;

      //---------------------------------------
      class S1 {
      public:
	S1(fhicl::ParameterSet const&);
	double fire(TRandom3*);
      private:
	double _tau1;
	double _tau2;
	double _p;
      } _s1;

      //---------------------------------------      
      class S2 {
      public:
	S2(fhicl::ParameterSet const&);
	double fire(TRandom3*);
      private:
	double _tau1;
	double _tau2;
	double _p;
	double _T;
        double _sigma;
	//static double time(double* x, double* par);
	TF1* _time_func;
      } _s2;

    }; //PeTimeGenerator

  } //MC namespace
} //darkart namespace


#endif
