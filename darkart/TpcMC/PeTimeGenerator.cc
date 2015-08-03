#include "darkart/TpcMC/PeTimeGenerator.hh"
#include "cetlib/exception.h"
#include <cmath>
#include "darkart/TpcMC/s2pulseshape_func.hh"

using namespace darkart;


MC::PeTimeGenerator::PeTimeGenerator(fhicl::ParameterSet const& p, std::string signal_type)
  : _signal_type(signal_type)
  , _laser(p.get<fhicl::ParameterSet>("LASER"))
  , _s1(p.get<fhicl::ParameterSet>("S1"))
  , _s2(p.get<fhicl::ParameterSet>("S2"))
{}

double MC::PeTimeGenerator::fire(TRandom3* ran)
{
  if       ( _signal_type == "LASER" )  return _laser.fire(ran);
  else if  ( _signal_type == "S1"    )  return _s1.fire(ran);
  else if  ( _signal_type == "S2"    )  return _s2.fire(ran);
  else throw cet::exception("MODE") << "Unrecognized mode: "<<_signal_type << "\n";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// LASER
MC::PeTimeGenerator::Laser::Laser(fhicl::ParameterSet const& p)
  : _mean(p.get<double>("mean"))
  , _sigma(p.get<double>("sigma"))
{}

double MC::PeTimeGenerator::Laser::fire(TRandom3* ran)
{
  return ran->Gaus(_mean, _sigma);
}

//-----------------------------------------------------------------------------
// S1
MC::PeTimeGenerator::S1::S1(fhicl::ParameterSet const& p)
  : _tau1(p.get<double>("tau1"))
  , _tau2(p.get<double>("tau2"))
  , _p(p.get<double>("fastFrac"))
{}

double MC::PeTimeGenerator::S1::fire(TRandom3* ran)
{
  if (ran->Uniform() < _p)
    return ran->Exp(_tau1);
  else
    return ran->Exp(_tau2);
}

//-----------------------------------------------------------------------------
// S2
MC::PeTimeGenerator::S2::S2(fhicl::ParameterSet const& p)
  : _tau1(p.get<double>("tau1"))
  , _tau2(p.get<double>("tau2"))
  , _p(p.get<double>("fastFrac"))
  , _T(p.get<double>("T"))
  , _sigma(p.get<double>("sigma"))
{
  _time_func = new TF1("s2_time_func", darkart::s2_simp, -2.*_tau2, 10.*_tau2, 5);
  _time_func->SetParameters(_tau1, _tau2, _p, _T, _sigma);
  _time_func->SetNpx(10000);
}
/*
double MC::PeTimeGenerator::S2::time(double* x, double* par)
{
  long double t = x[0]; //time
  long double t1 = par[0];
  long double t2 = par[1];
  long double p  = par[2];
  long double T  = par[3];
  long double sigma = par[4];
  double result;
  if (t<0)
    result = 0;
  else if (t>=0 && t<T)
    result = 1./T * ( p      * (1.-std::exp(-t/t1)) + 
		      (1.-p) * (1.-std::exp(-t/t2)) );
  else // (t>=T)
    result = 1./T * ( p      * (std::exp(-(t-T)/t1) - std::exp(-t/t1)) +
		      (1.-p) * (std::exp(-(t-T)/t2) - std::exp(-t/t2)) );
  return result;
}
*/
double MC::PeTimeGenerator::S2::fire(TRandom3*)
{
  return _time_func->GetRandom();
}
