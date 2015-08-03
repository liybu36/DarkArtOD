/////////////////////////////////////////////////////////////////////////
//
//  2014-10-22 AFan
//
//  Helper class for generating PE charges from various spectra.
//
//  GausExp
//  - Usual gaussian + exponential
//  
//  Line
//  - All PE have the same charge, defined as pdf mean of Default
// 
//  GausConst
//  GausGaus
//  Gaus
//
//  Note: Even though all these implementations use TF1::GetRandom(), 
//  we keep the TRandom3 interface in case new modes want to use it.
//  See PeTimeGenerator for an example.
/////////////////////////////////////////////////////////////////////////


#ifndef PeChargeGenerator_hh
#define PeChargeGenerator_hh

#include <string>
#include "TRandom3.h"
#include "TF1.h"
#include "fhiclcpp/ParameterSet.h"

namespace darkart
{
  namespace MC
  {
    class PeChargeGenerator
    {
    public:
      PeChargeGenerator(fhicl::ParameterSet const& p, std::string mode);

      double fire(TRandom3*);
      double mean();
      double sigma();

    private:
      
      std::string _mode;

      //------------------------------------------
      class GausExp {
      public:
	GausExp(fhicl::ParameterSet const&);
	double fire(TRandom3*);
        double mean();
        double sigma();
      private:
	double _gaus_mean;
	double _gaus_sigma;
	double _exp_prob;
	double _exp_slope;
	static double charge(double* x, double* par);
        TF1* _charge_func;

      } _gausExp;


      //------------------------------------------
      class Line {
      public:
	Line(fhicl::ParameterSet const&);
	double fire(TRandom3*);
        double mean();
        double sigma();
      private:
	double _value;
      } _line;

      //------------------------------------------
      class GausConst {
      public:
	GausConst(fhicl::ParameterSet const&);
	double fire(TRandom3*);
        double mean();
        double sigma();
      private:
	double _gaus_mean;
	double _gaus_sigma;
	double _underamp_prob;
	double _const_charge;
	static double charge(double* x, double* par);
	TF1* _charge_func;
      } _gausConst;


      //------------------------------------------
      class GausGaus {
      public:
	GausGaus(fhicl::ParameterSet const&);
	double fire(TRandom3*);
        double mean();
        double sigma();
      private:
	double _gaus_mean;
	double _gaus_sigma;
	double _underamp_prob;
	double _underamp_mean;
	static double charge(double* x, double* par);
	TF1* _charge_func;
      } _gausGaus;


      //------------------------------------------
      class Gaus {
      public:
	Gaus(fhicl::ParameterSet const&);
	double fire(TRandom3*);
        double mean();
        double sigma();
      private:
	double _gaus_mean;
	double _gaus_sigma;
	static double charge(double* x, double* par);
        TF1* _charge_func;
      } _gaus;

    }; //PeChargeGenerator

  } //MC namespace
} //darkart namespace


#endif
