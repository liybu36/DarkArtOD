// common utilitiy functions for OD reconstruction

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "darkart/ODReco/Tools/Utilities.hh"
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <math.h>

darkart::od::Utilities::Utilities(fhicl::ParameterSet const & p,
				  art::ActivityRegistry &)
  : _lsv_sample_rate_Hz(-1)
  , _lsv_record_length(-1)
  , _lsv_reference_pos(-1)
  , _wt_sample_rate_Hz(-1)
  , _wt_record_length(-1)
  , _wt_reference_pos(-1)
  , _prompt_offset_ns(p.get<double>("prompt_offset_ns",-999))
{ 
  std::cout << "Utilities" << std::endl;

  //check input parameters from fhicl
  if (_prompt_offset_ns == -999)
    throw cet::exception("Utilities")<<"Error: in FHiCL file, prompt_offset_ns is not specified \n";
}

DEFINE_ART_SERVICE(darkart::od::Utilities)



/*
darkart::od::GlobalDetectorConfig::GlobalDetectorConfig()
{
  _lsv_same_configuration = false;
  _wt_same_configuration = false;
  _lsv_sample_rate = -1.;
  _wt_sample_rate = -1.;
  _lsv_record_length = -1;
  _wt_record_length = -1;
  _lsv_reference_pos = -1.;
  _wt_reference_pos = -1.;
}

darkart::od::GlobalDetectorConfig::~GlobalDetectorConfig()
{

}

void darkart::od::GlobalDetectorConfig::setLSVConfig(bool lsv_same_configuration, double lsv_sample_rate, int lsv_record_length, double lsv_reference_pos)
{
  _lsv_same_configuration = lsv_same_configuration;
  if ( _lsv_same_configuration ){
    _lsv_sample_rate = lsv_sample_rate;
    _lsv_record_length = lsv_record_length;
    _lsv_reference_pos = lsv_reference_pos;
  }
}

void darkart::od::GlobalDetectorConfig::setWTConfig(bool wt_same_configuration, double wt_sample_rate, int wt_record_length, double wt_reference_pos)
{
  _wt_same_configuration = wt_same_configuration;
  if ( _wt_same_configuration ){
    _wt_sample_rate = wt_sample_rate;
    _wt_record_length = wt_record_length;
    _wt_reference_pos = wt_reference_pos;
  }
}
*/
