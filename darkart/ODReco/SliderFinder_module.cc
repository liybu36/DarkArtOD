////////////////////////////////////////////////////////////////////////
// Class:       SliderFinder
// Module Type: producer
// File:        SliderFinder_module.cc
//
// Generated at Tue Feb 24 11:33:08 2015 by Xin Xiang using artmod
// from cetpkgsupport v1_07_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "darkart/ODReco/Tools/Utilities.hh"
#include "darkart/ODProducts/Slider.hh"
#include "darkart/ODProducts/SliderCfg.hh"
#include "darkart/ODProducts/SumWF.hh"
#include "darkart/ODProducts/RunInfo.hh"

//debug
//#include <fstream>
//#include "darkart/ODProducts/ODEventInfo.hh"

#include <memory>
#include <iostream>


namespace darkart {
  namespace od {
    class SliderFinder;
  }
}

class darkart::od::SliderFinder : public art::EDProducer {
public:
  explicit SliderFinder(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.
  
  // Plugins should not be copied or assigned.
  SliderFinder(SliderFinder const &) = delete;
  SliderFinder(SliderFinder &&) = delete;
  SliderFinder & operator = (SliderFinder const &) = delete;
  SliderFinder & operator = (SliderFinder &&) = delete;
  
  // Required functions.
  void processSlider(float&, float&, float&, const art::ValidHandle<darkart::od::SumWF>, int, int, int, float);
  
  void beginRun(art::Run & r) override;
  void produce(art::Event & e) override;
  
  
private:
  // Declare member data here.
  int _verbosity;
  int _channel_type;
  std::string _input_waveform;  
  std::vector<float> _slider_width_ns;
  std::vector<float> _slider_start_ns;
  std::vector<float> _slider_end_ns;
  std::vector<int> _slider_label;
  float _prompt_offset_ns;

  art::ServiceHandle<darkart::od::Utilities> _utils;
  int record_length;
  //float _lsv_sample_rate_Hz;
  //float _wt_sample_rate_Hz;
  //float _lsv_sample_rate_GHz;
  //float _wt_sample_rate_GHz;
};


darkart::od::SliderFinder::SliderFinder(fhicl::ParameterSet const & p)
  : _verbosity(p.get<int>("verbosity", 0))
  , _channel_type(p.get<int>("channel_type", 0))
  , _input_waveform(p.get<std::string>("input_waveform","none"))
  , _slider_width_ns(p.get<std::vector<float>>("slider_width_ns",{}))
  , _slider_start_ns(p.get<std::vector<float>>("slider_start_ns",{}))
  , _slider_end_ns(p.get<std::vector<float>>("slider_end_ns",{}))
  , _slider_label(p.get<std::vector<int>>("slider_label",{}))
 {
  
  //check input
  if (!(_channel_type == 1 || _channel_type == 2))
    throw cet::exception("SliderFinder")<<"Error: in FHiCL file, channel_type=1 for LSV or channel_type=2 for WT \n";
  if (_slider_width_ns.size() == 0)
    throw cet::exception("SliderFinder")<<"Error: in FHiCL file, slider_with_ns is not specified. \n";
  if (_slider_start_ns.size() == 0)
    throw cet::exception("SliderFinder")<<"Error: in FHiCL file, slider_start_ns is not specified \n";
  if (_slider_end_ns.size() == 0)
    throw cet::exception("SliderFinder")<<"Error: in FHiCL file, slider_end_ns is not specified \n";
  if (_slider_label.size() == 0)
    throw cet::exception("SliderFinder")<<"Error: in FHiCL file, slider_label is not specified \n";
  
  //check the same size
  if ( (_slider_start_ns.size() != _slider_end_ns.size()) || (_slider_end_ns.size() != _slider_label.size()) || _slider_label.size() !=_slider_width_ns.size()  ) {
    throw cet::exception("SliderFinder")<<"Error: slider_start_ns, slider_end_ns, slider_label and slider_width need to have the same size \n";
  }
  
  //check proper start time and end time
  for (unsigned int i=0; i<_slider_start_ns.size(); i++) {
    if (_slider_start_ns.at(i) >= _slider_end_ns.at(i))
      throw cet::exception("SliderFinder")<<"Error: your sldier_start_ns ("<<_slider_start_ns.at(i)<<") is greater or equal to"<<" slider_end_ns ("<<_slider_end_ns.at(i) <<") \n";
  }


  // Call appropriate produces<>() functions here.
  produces<darkart::od::Slider>();
  produces<darkart::od::SliderCfg>();

  if (_verbosity == 2)
    LOG_INFO("SliderFinder") << "DEBUG: test to see if constructor is called. so yes!";
}


void darkart::od::SliderFinder::beginRun(art::Run &)
{
   _prompt_offset_ns = _utils->getPromptOffsetNs();
}


void darkart::od::SliderFinder::produce(art::Event & e)
{
  record_length = _utils->getRecordLength(_channel_type);
  
  //variables will be saved to Slider.hh and SliderCfg.hh
  std::unique_ptr<darkart::od::Slider> slider_ptr(new darkart::od::Slider());
  std::unique_ptr<darkart::od::SliderCfg> slidercfg_ptr(new darkart::od::SliderCfg());
  
  const auto sum_wf_handle = e.getValidHandle<darkart::od::SumWF>(_input_waveform);
  const double first_samp_time_ns = sum_wf_handle->begin()->sample_ns;
  const double last_samp_time_ns = sum_wf_handle->back().sample_ns;

  if (_verbosity == 2)
    LOG_INFO("SliderFinder") << "DEBUG: first_samp_time_ns="<<first_samp_time_ns<<" last_samp_time_ns="<<last_samp_time_ns;

  if (_verbosity == 2) {
    LOG_INFO("SliderFinder") << "DEBUG:  _slider_start_ns.size()="<<_slider_start_ns.size();
    LOG_INFO("SliderFinder") << "DEBUG:  _slider_end_ns.size()="<<_slider_end_ns.size();
  }

  for (size_t idx_slider=0; idx_slider<_slider_start_ns.size(); idx_slider++) {

    //add prompt offset
    const double start_time_ns=_prompt_offset_ns+_slider_start_ns.at(idx_slider);
    const double end_time_ns=_prompt_offset_ns+_slider_end_ns.at(idx_slider);

    //get start sample and end sample
    int start_samp = _utils->nsToSample(start_time_ns, _channel_type);
    if(start_samp < 0) start_samp = 0;
    if(start_samp >= record_length) start_samp = record_length-1;
    int end_samp = _utils->nsToSample(end_time_ns, _channel_type);
    if(end_samp < 0) end_samp = 0;
    if(end_samp >= record_length) end_samp = record_length-1;
   
    const int width_samp = int(_slider_width_ns.at(idx_slider)*_utils->getSampleRateGHz(_channel_type));
    //float* charge_ptr = new float(); float* time_ptr = new float(); float* max_mult_ptr = new float();
    float charge, time, max_mult;
      
        
    if (_verbosity == 2)
      LOG_INFO("SliderFinder") <<"DEBUG: start_samp="<<start_samp<<" end_samp="<<end_samp<<" width_samp="<<width_samp;

    processSlider(charge, time, max_mult, sum_wf_handle, start_samp, end_samp, width_samp, (float) _utils->getSampleRateHz(_channel_type));
  
    if (_verbosity == 2) {
      LOG_INFO("SliderFinder") <<"DEBUG: the max charge "<<charge<<" pe happens at"<<time<<" ns";
    }
    
    slider_ptr->charge_vec.push_back(charge);
    slider_ptr->time_ns_vec.push_back(time);
    slider_ptr->max_multiplicity_vec.push_back(max_mult);
  }


  // fill in SliderCfg
  slidercfg_ptr->start_ns_vec = _slider_start_ns;
  slidercfg_ptr->end_ns_vec = _slider_end_ns;
  slidercfg_ptr->width_ns_vec = _slider_width_ns;
  slidercfg_ptr->label_vec = _slider_label;
  for (unsigned int i=0; i<_slider_start_ns.size(); i++)
    slidercfg_ptr->id_vec.push_back(i);
  

  e.put(std::move( slider_ptr ));
  e.put(std::move( slidercfg_ptr )); 
}



void darkart::od::SliderFinder::processSlider(float& charge_ptr, float& time_ptr, float& max_mult_ptr, art::ValidHandle<darkart::od::SumWF> wf_handle, int start_samp, int end_samp, int width_samp, float sample_rate_Hz)
{
  // Check that everything is within reasonable bounds
  if((start_samp == end_samp) ||
     (start_samp < 0) ||
     (start_samp >= int(wf_handle->size())) ||
     (end_samp < 0) ||
     (end_samp >= int(wf_handle->size()))){
    charge_ptr = -1.;
    time_ptr = -1.;
    max_mult_ptr = -1.;
    return;
  }

  //init max integral as the integral of the first window
  float integral =0;
  for (int i= start_samp; i<start_samp+width_samp; i++){
    if(i >= int(wf_handle->size())) break;
    integral += wf_handle->at(i).amplitude;
  }
  float max_integral = integral;


  //compute running integral and time
  int max_integral_first_samp = start_samp; 
  for (int run_samp = start_samp; run_samp+width_samp < end_samp; run_samp ++ ) {
    
    //add next sample and subtract first sample
    const float first_samp_amp =  wf_handle->at(run_samp).amplitude;
    size_t next_samp = size_t(run_samp + width_samp);
    if(next_samp >= wf_handle->size()) next_samp = wf_handle->size()-1;
    const float next_samp_amp = wf_handle->at(next_samp).amplitude;

    integral -= first_samp_amp;
    integral += next_samp_amp;
    
    if ( integral > max_integral ) {
      max_integral = integral;
      max_integral_first_samp = run_samp+1;
    }
  }
  
  //convert to PE
  max_integral *= 1./(sample_rate_Hz);
  
  //calculate time and max multiplicty corresponding to max_integral
  int max_integral_time_ns = wf_handle->at(int(max_integral_first_samp + width_samp/2)).sample_ns;
  int max_integral_mult= 0;
  for (int i=max_integral_first_samp; i< max_integral_first_samp+width_samp; i++) {
    if (wf_handle->at(i).amplitude_discr>max_integral_mult)
      max_integral_mult = wf_handle->at(i).amplitude_discr;
  }

  charge_ptr = max_integral;
  time_ptr = max_integral_time_ns;
  max_mult_ptr = max_integral_mult; 
}


DEFINE_ART_MODULE(darkart::od::SliderFinder)





