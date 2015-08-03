#ifndef darkart_ODReco_utilities_hh
#define darkart_ODReco_utitilies_hh

#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/UserInteraction/UserInteraction.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"

namespace darkart{
  namespace od{

    class Utilities {
    public:
      Utilities(const fhicl::ParameterSet &, art::ActivityRegistry &);

      // Calculate channel hardware coordinates
      int getGlobalChannelID(int chassis, int board, int channel) const { return 64*chassis + 4*board + channel; }
      int getGlobalScopeID(int chassis, int board) const { return 16*chassis + board; }

      // Convert between sample number and time with repsect to trigger
      
      double lsvSampleToNs(int sample) const
      { return (sample-_lsv_reference_pos*_lsv_record_length)/(_lsv_sample_rate_Hz*HzToGHz); }
      int lsvNsToSample(const double& ns) const
      { return ns*_lsv_sample_rate_Hz*HzToGHz+_lsv_reference_pos*_lsv_record_length; }
      double wtSampleToNs(int sample) const
      { return (sample-_wt_reference_pos*_wt_record_length)/(_wt_sample_rate_Hz*HzToGHz); }
      int wtNsToSample(const double& ns) const
      { return ns*_wt_sample_rate_Hz*HzToGHz+_wt_reference_pos*_wt_record_length; }
      double sampleToNs(int sample, int detector) 
      { 
	if(abs(detector) == 1)
	  return lsvSampleToNs(sample);
	if(abs(detector) == 2)
	  return wtSampleToNs(sample);
	return -1;
      }
      int nsToSample(const double& ns, int detector) const
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
      
      //convert between sample number with time wrt prompt offset
      double lsvSampleToPromptNs (const int& samp) const
      { return lsvTriggerNsToPromptNs( lsvSampleToNs(samp) ); }
      int lsvPromptNsToSample (const double& ns) const 
      { return lsvNsToSample( lsvPromptNsToTriggerNs(ns) ); }

      // Getters and setters
      const double& getLSVSamplerateHz()  const { return _lsv_sample_rate_Hz; }
      const double& getLSVSampleRateGHz() const { return _lsv_sample_rate_Hz * HzToGHz; }
      const double& getLSVRecordLength()  const { return _lsv_record_length; }
      const double& getLSVReferencePos()  const { return _lsv_reference_pos; }

      const double& getWTSamplerateHz()  const { return _wt_sample_rate_Hz; }
      const double& getWTSampleRateGHz() const { return _wt_sample_rate_Hz * HzToGHz; }
      const double& getWTRecordLength()  const { return _wt_record_length; }
      const double& getWTReferencePos()  const { return _wt_reference_pos; }

      void setLSVSampleRateHz(const double& r) { _lsv_sample_rate_Hz = r; }
      void setLSVRecordLength(const double& l) { _lsv_record_length = l; }
      void setLSVReferencePos(const double& p) { _lsv_reference_pos = p; }

      void setWTSamplerateHz(const double& r) { _wt_sample_rate_Hz = r; }
      void setWTRecordLength(const double& l) { _wt_record_length = l; }
      void setWTReferencePos(const double& p) { _wt_reference_pos = p; }

      // Useful constants
      const double HzToGHz = 1.e-9;
      const double GHzToHz = 1.e9;

    private:
      double _lsv_sample_rate_Hz;
      double _lsv_record_length;
      double _lsv_reference_pos;

      double _wt_sample_rate_Hz;
      double _wt_record_length;
      double _wt_reference_pos;      
    };
  }   
}
DECLARE_ART_SERVICE(darkart::od::Utilities, LEGACY)
#endif
