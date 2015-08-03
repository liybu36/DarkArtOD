#ifndef darkart_ODReco_utilities_hh
#define darkart_ODReco_utilities_hh

#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "darkart/ODProducts/SumWF.hh"

#include <iostream>

namespace darkart{
  namespace od{

    class Utilities {
    public:
      Utilities(const fhicl::ParameterSet &, art::ActivityRegistry &);

      // Calculate channel hardware coordinates
      int getGlobalChannelID(int chassis, int board, int channel) const { return 64*chassis + 4*board + channel; }
      int getGlobalScopeID(int chassis, int board) const { return 16*chassis + board; }

      // Get a pointer to this function
      Utilities* getPointer() { return this; }

      // Set a SumWF to its default values
      void zeroSumWF(darkart::od::SumWF & wf, int detector)
      {
	double reference_pos = this->getReferencePos(detector);
	double record_length = this->getRecordLength(detector);
	double sample_rate_GHz = this->getSampleRateGHz(detector);
	double wf_size = record_length;

	// If this detector is not enabled (i.e. record length == -1)
	// Zero the sumwf with a small number of samples, so that everything
	// built on the sumwf will come out blank, but not crash processing
	if(record_length == -1) wf_size = 10;
	wf.resize(wf_size);
	for (int samp = 0; samp<wf_size; ++samp) {
	  wf[samp].amplitude = 0;
	  wf[samp].ch_pulse_start_pairs.clear();
	  wf[samp].ch_pulse_end_pairs.clear();
	  wf[samp].amplitude_discr = 0;
	  wf[samp].sample = samp;
	  wf[samp].sample_ns = (samp-reference_pos*record_length)/(sample_rate_GHz);
	}
      }

      // Convert channel number to type (up to a sign, since it does not check if enabled)
      int getChannelType(int ch) const { return ch <= 127 ? 1 : 2; }

      // Convert between sample number and time relative to trigger
      double lsvSampleToNs(int sample) const
      { return (sample-_lsv_reference_pos*_lsv_record_length)/(_lsv_sample_rate_Hz*HzToGHz); }
      int lsvNsToSample(const double& ns) const
      { return ns*_lsv_sample_rate_Hz*HzToGHz+_lsv_reference_pos*_lsv_record_length; }
      double wtSampleToNs(int sample) const
      { return (sample-_wt_reference_pos*_wt_record_length)/(_wt_sample_rate_Hz*HzToGHz); }
      int wtNsToSample(const double& ns) const
      { return ns*_wt_sample_rate_Hz*HzToGHz+_wt_reference_pos*_wt_record_length; }
      double sampleToNs(int sample, int detector) const 
      { 
	if(abs(detector) == 1)
	  return lsvSampleToNs(sample);
	if(abs(detector) == 2)
	  return wtSampleToNs(sample);
	return -1;
      }
      int nsToSample(double ns, int detector) const
      { 
	if(abs(detector) == 1)
	  return lsvNsToSample(ns);
	if(abs(detector) == 2)
	  return wtNsToSample(ns);
	return -1;
      }

      //convert between time wrt trigger and time wrt prompt offset
      double lsvTriggerNsToPromptNs(const double& ns) const
      { return ns - _prompt_offset_ns; }
      double lsvPromptNsToTriggerNs(const double& ns) const
      { return ns + _prompt_offset_ns; }
      double wtTriggerNsToPromptNs(const double& ns) const
      { return ns - _prompt_offset_ns; }
      double wtPromptNsToTriggerNs(const double& ns) const
      { return ns + _prompt_offset_ns; }
      double triggerNsToPromptNs(const double& ns, const int& detector) const
      { 
	if(abs(detector) == 1)
	  return lsvTriggerNsToPromptNs(ns);
	if(abs(detector) == 2)
	  return wtTriggerNsToPromptNs(ns);
	return -1;
      }
      double promptNsToTriggerNs(const double& ns, const int& detector) const
      { 
	if(abs(detector) == 1)
	  return lsvPromptNsToTriggerNs(ns);
	if(abs(detector) == 2)
	  return wtPromptNsToTriggerNs(ns);
	return -1;
      }      

      //convert between sample number with time wrt prompt offset
      double lsvSampleToPromptNs (const int& samp) const
      { return lsvTriggerNsToPromptNs( lsvSampleToNs(samp) ); }
      int lsvPromptNsToSample (const double& ns) const
      { return lsvNsToSample( lsvPromptNsToTriggerNs(ns) ); }
      double wtSampleToPromptNs (const int& samp) const
      { return wtTriggerNsToPromptNs( wtSampleToNs(samp) ); }
      int wtPromptNsToSample (const double& ns) const
      { return wtNsToSample( wtPromptNsToTriggerNs(ns) ); }
      double sampleToPromptNs (const int& samp, const int& detector) const
      {
	if(abs(detector) == 1)
	  return lsvSampleToPromptNs(samp);
	if(abs(detector) == 2)
	  return wtSampleToPromptNs(samp);
	return -1;
      }
      double promptNsToSample (const double& ns, const int& detector) const
      {
	if(abs(detector) == 1)
	  return lsvPromptNsToSample(ns);
	if(abs(detector) == 2)
	  return wtPromptNsToSample(ns);
	return -1;
      }



      // Getters and setters
      const double& getLSVSampleRateHz()  const { return _lsv_sample_rate_Hz; }
      double  getLSVSampleRateGHz() const { return _lsv_sample_rate_Hz * HzToGHz; }
      const double& getLSVRecordLength()  const { return _lsv_record_length; }
      const double& getLSVReferencePos()  const { return _lsv_reference_pos; }
      const double& getLSVZSPresamples()  const { return _lsv_zs_presamples; }
      const double& getLSVZSPostsamples() const { return _lsv_zs_postsamples;}

      const double& getWTSampleRateHz()  const { return _wt_sample_rate_Hz; }
      double  getWTSampleRateGHz() const { return _wt_sample_rate_Hz * HzToGHz; }
      const double& getWTRecordLength()  const { return _wt_record_length; }
      const double& getWTReferencePos()  const { return _wt_reference_pos; }
      const double& getWTZSPresamples()  const { return _wt_zs_presamples; }
      const double& getWTZSPostsamples() const { return _wt_zs_postsamples;}

      const double& getPromptOffsetNs() const { return _prompt_offset_ns; }

      double getSampleRateHz(int detector) const
      {
	if(abs(detector) == 1)
	  return getLSVSampleRateHz();
	if(abs(detector) == 2)
	  return getWTSampleRateHz();
	return -1;
      }
      double getSampleRateGHz(int detector) const
      {
	if(abs(detector) == 1)
	  return getLSVSampleRateGHz();
	if(abs(detector) == 2)
	  return getWTSampleRateGHz();
	return -1;
      }
      double getRecordLength(int detector) const
      {
	if(abs(detector) == 1)
	  return getLSVRecordLength();
	if(abs(detector) == 2)
	  return getWTRecordLength();
	return -1;
      }
      double getReferencePos(int detector) const
      {
	if(abs(detector) == 1)
	  return getLSVReferencePos();
	if(abs(detector) == 2)
	  return getWTReferencePos();
	return -1;
      }
      double getZSPresamples(int detector) const
      {
	if(abs(detector) == 1)
	  return getLSVZSPresamples();
	if(abs(detector) == 2)
	  return getWTZSPresamples();
	return -1;
      }
      double getZSPostsamples(int detector) const
      {
	if(abs(detector) == 1)
	  return getLSVZSPostsamples();
	if(abs(detector) == 2)
	  return getWTZSPostsamples();
	return -1;
      }

      void setLSVSampleRateHz(const double& r) { _lsv_sample_rate_Hz = r; }
      void setLSVRecordLength(const double& l) { _lsv_record_length = l; }
      void setLSVReferencePos(const double& p) { _lsv_reference_pos = p; }
      void setLSVZSPresamples(const double& s) { _lsv_zs_presamples = s; }
      void setLSVZSPostsamples(const double&s) { _lsv_zs_postsamples= s; }

      void setWTSampleRateHz(const double& r) { _wt_sample_rate_Hz = r; }
      void setWTRecordLength(const double& l) { _wt_record_length = l; }
      void setWTReferencePos(const double& p) { _wt_reference_pos = p; }
      void setWTZSPresamples(const double& s) { _wt_zs_presamples = s; }
      void setWTZSPostsamples(const double&s) { _wt_zs_postsamples= s; }

      void setPromptOffsetNs(const double& p) { _prompt_offset_ns = p; }

      // Useful constants
      const double HzToGHz = 1.e-9;
      const double GHzToHz = 1.e9;

    private:
      double _lsv_sample_rate_Hz;
      double _lsv_record_length;
      double _lsv_reference_pos;
      double _lsv_zs_presamples;  // number of zero suppression presamples
      double _lsv_zs_postsamples; // number of zero suppression postsamples

      double _wt_sample_rate_Hz;
      double _wt_record_length;
      double _wt_reference_pos; 
      double _wt_zs_presamples;  // number of zero suppression presamples
      double _wt_zs_postsamples; // number of zero suppression postsamples     
      
      double _prompt_offset_ns;
    };
  }   
}
  
DECLARE_ART_SERVICE(darkart::od::Utilities, LEGACY)
#endif
