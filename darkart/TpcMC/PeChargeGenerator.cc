#include "PeChargeGenerator.hh"
#include "fhiclcpp/ParameterSet.h"
#include "art/Utilities/Exception.h"
#include "cetlib/exception.h"
#include "TMath.h"

using namespace darkart;


MC::PeChargeGenerator::PeChargeGenerator(fhicl::ParameterSet const& p, std::string mode)
  : _mode(mode)
  , _gausExp(p.get<fhicl::ParameterSet>("GAUSEXP"))
  , _line(p.get<fhicl::ParameterSet>("LINE"))
  , _gausConst(p.get<fhicl::ParameterSet>("GAUSCONST"))
  , _gausGaus(p.get<fhicl::ParameterSet>("GAUSGAUS"))
  , _gaus(p.get<fhicl::ParameterSet>("GAUS"))
{}

double MC::PeChargeGenerator::fire(TRandom3* ran)
{
  if      ( _mode == "GAUSEXP"   ) return _gausExp.fire(ran);
  else if ( _mode == "LINE"      ) return _line.fire(ran);
  else if ( _mode == "GAUSCONST" ) return _gausConst.fire(ran);
  else if ( _mode == "GAUSGAUS"  ) return _gausGaus.fire(ran);
  else if ( _mode == "GAUS"      ) return _gaus.fire(ran);
  else throw cet::exception("MODE") << "Unrecognized mode: "<<_mode << "\n";
}

double MC::PeChargeGenerator::mean()
{
  if      ( _mode == "GAUSEXP"   ) return _gausExp.mean();
  else if ( _mode == "LINE"      ) return _line.mean();
  else if ( _mode == "GAUSCONST" ) return _gausConst.mean();
  else if ( _mode == "GAUSGAUS"  ) return _gausGaus.mean();
  else if ( _mode == "GAUS"      ) return _gaus.mean();
  else throw cet::exception("MODE") << "Unrecognized mode: "<<_mode << "\n";
}

double MC::PeChargeGenerator::sigma()
{
  if      ( _mode == "GAUSEXP"   ) return _gausExp.sigma();
  else if ( _mode == "LINE"      ) return _line.sigma();
  else if ( _mode == "GAUSCONST" ) return _gausConst.sigma();
  else if ( _mode == "GAUSGAUS"  ) return _gausGaus.sigma();
  else if ( _mode == "GAUS"      ) return _gaus.sigma();
  else throw cet::exception("MODE") << "Unrecognized mode: "<<_mode << "\n";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// GAUSSIAN + EXPONENTIAL
MC::PeChargeGenerator::GausExp::GausExp(fhicl::ParameterSet const& p)
  : _gaus_mean(p.get<double>("gausMean"))
  , _gaus_sigma(p.get<double>("gausSigma"))
  , _exp_prob(p.get<double>("expProb"))
  , _exp_slope(p.get<double>("expSlope"))
{
  _charge_func = new TF1("gaus_exp_charge_func", charge, 0., 10.*_gaus_mean, 4);
  _charge_func->SetParameters(_gaus_mean, _gaus_sigma, _exp_prob, _exp_slope);
  _charge_func->SetNpx(10000);
}

double MC::PeChargeGenerator::GausExp::charge(double* x, double* par)
{
  double q = x[0]; //Charge
  double gauss_mean = par[0];
  double gauss_sigma = par[1];
  double exp_prob = par[2];
  double exp_slope = par[3];

  double result;
  if (q <= 0)
    result = 0;
  else
    result = (exp_prob*TMath::Exp(-q/exp_slope)/exp_slope) +
      (1-exp_prob)*TMath::Gaus(q, gauss_mean, gauss_sigma, true);

  return result;
}

double MC::PeChargeGenerator::GausExp::fire(TRandom3*)
{
  return _charge_func->GetRandom();
}

double MC::PeChargeGenerator::GausExp::mean()
{
  return _charge_func->Mean(0, 20*_gaus_mean);
}

double MC::PeChargeGenerator::GausExp::sigma()
{
  return TMath::Sqrt(_charge_func->Variance(0, 20*_gaus_mean));
}



//-----------------------------------------------------------------------------
// SINGLE VALUE
MC::PeChargeGenerator::Line::Line(fhicl::ParameterSet const& p)
  : _value(p.get<double>("value"))
{}

double MC::PeChargeGenerator::Line::fire(TRandom3*)
{
  return _value;
}

double MC::PeChargeGenerator::Line::mean()
{
  return _value;
}

double MC::PeChargeGenerator::Line::sigma()
{
  return 0;
}

//-----------------------------------------------------------------------------
// GAUSSIAN-CONSTANT
MC::PeChargeGenerator::GausConst::GausConst(fhicl::ParameterSet const& p)
  : _gaus_mean(p.get<double>("gausMean"))
  , _gaus_sigma(p.get<double>("gausSigma"))
  , _underamp_prob(p.get<double>("underampProb"))
  , _const_charge(p.get<double>("constCharge"))
{
  _charge_func = new TF1("ser_charge_function", MC::PeChargeGenerator::GausConst::charge, 0., 10*_gaus_mean, 4);
  _charge_func->SetParameters(_gaus_mean, _gaus_sigma, _underamp_prob, _const_charge);
  _charge_func->SetNpx(10000);
}

double MC::PeChargeGenerator::GausConst::charge(double* x, double* par)
{
  double q = x[0]; //Charge

  double gaus_mean = par[0];
  double gaus_sigma = par[1];
  double underamp_prob = par[2];
  double const_charge = par[3];
  
  double result;
  if ( q <= 0)
    result = 0;
  else if ( q < const_charge)
    result = (1-underamp_prob)*TMath::Gaus(const_charge, gaus_mean, gaus_sigma, true);
  else
    result = (1-underamp_prob)*TMath::Gaus(q, gaus_mean, gaus_sigma, true);
  
  return result;
}

double MC::PeChargeGenerator::GausConst::fire(TRandom3*)
{
  return _charge_func->GetRandom();
}

double MC::PeChargeGenerator::GausConst::mean()
{
  return _charge_func->Mean(0, 20*_gaus_mean);
}

double MC::PeChargeGenerator::GausConst::sigma()
{
  return TMath::Sqrt(_charge_func->Variance(0, 20*_gaus_mean));
}


//-----------------------------------------------------------------------------
// GAUSSIAN + GAUSSIAN
MC::PeChargeGenerator::GausGaus::GausGaus(fhicl::ParameterSet const& p)
  : _gaus_mean(p.get<double>("gausMean"))
  , _gaus_sigma(p.get<double>("gausSigma"))
  , _underamp_prob(p.get<double>("underampProb"))
  , _underamp_mean(p.get<double>("underampMean"))
{
  _charge_func = new TF1("gaus_gaus_charge_func", charge, 0., 10.*_gaus_mean, 4);
  _charge_func->SetParameters(_gaus_mean, _gaus_sigma, _underamp_prob, _underamp_mean);
  _charge_func->SetNpx(10000);
}

double MC::PeChargeGenerator::GausGaus::charge(double* x, double* par)
{
  double q = x[0]; //Charge

  double gaus_mean = par[0];
  double gaus_sigma = par[1];
  double underamp_prob = par[2];
  double underamp_mean = par[3];

  double result;
  if ( q <= 0)
    result = 0;
  else
    result = (underamp_prob)*TMath::Gaus(q, underamp_mean, gaus_sigma*(underamp_mean/gaus_mean), true) +
      (1-underamp_prob)*TMath::Gaus(q, gaus_mean, gaus_sigma, true);

  return result;
}

double MC::PeChargeGenerator::GausGaus::fire(TRandom3*)
{
  return _charge_func->GetRandom();
}

double MC::PeChargeGenerator::GausGaus::mean()
{
  return _charge_func->Mean(0, 20*_gaus_mean);
}

double MC::PeChargeGenerator::GausGaus::sigma()
{
  return TMath::Sqrt(_charge_func->Variance(0, 20*_gaus_mean));
}

//-----------------------------------------------------------------------------
// GAUSSIAN
MC::PeChargeGenerator::Gaus::Gaus(fhicl::ParameterSet const& p)
  : _gaus_mean(p.get<double>("gausMean"))
  , _gaus_sigma(p.get<double>("gausSigma"))
{
  _charge_func = new TF1("gaus_charge_func", charge, 0., 10.*_gaus_mean, 2);
  _charge_func->SetParameters(_gaus_mean, _gaus_sigma);
  _charge_func->SetNpx(10000);
}

double MC::PeChargeGenerator::Gaus::charge(double* x, double* par)
{
  double q = x[0]; //Charge

  double gaus_mean = par[0];
  double gaus_sigma = par[1];
    
  double result;
  if ( q <= 0)
    result = 0;
  else
    result = TMath::Gaus(q, gaus_mean, gaus_sigma, true);

  return result;
}

double MC::PeChargeGenerator::Gaus::fire(TRandom3*)
{
  return _charge_func->GetRandom();
}

double MC::PeChargeGenerator::Gaus::mean()
{
  return _charge_func->Mean(0, 20*_gaus_mean);
}

double MC::PeChargeGenerator::Gaus::sigma()
{
  return TMath::Sqrt(_charge_func->Variance(0, 20*_gaus_mean));
}
