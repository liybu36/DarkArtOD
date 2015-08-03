#ifndef darkart_ODProducts_ChannelData_hh
#define darkart_ODProducts_ChannelData_hh

#include <vector>
#include <list>
#include <sstream>
#include <string>

//Forward declarations
namespace darkart {
  namespace od {
    struct ChannelData;
    typedef std::vector<ChannelData> ChannelDataVec;
  }
}


struct darkart::od::ChannelData
{
  ChannelData()
    : chassis(-1)
    , board(-1)
    , channel(-1)
    , channel_id(-1)
    , channel_type(0)
    , is_enabled(false)
    , npulses(-1)
    , pulses()
/*    , pulse_id(-2)
    , start_sample(-1)
    , start_ns
    , peak_sample
    , peak_ns
    , size_sample
    , pedestal_mean
    , pedestal_rms
    , integral
    , saturation_correction
    , amplitude
    , is_saturated*/
  {}

  int chassis;
  int board;

  int channel; //specific to the board
  int channel_id; //global ID
  int channel_type; //0: default 1: LSV, 2: WT, -1: disabled LSV, -2: disable WT
  bool is_enabled;

  int npulses;
  
  struct Pulse {
    Pulse()
    : pulse_id(-1)
    , start_sample(-1)
    , start_ns(-1.e+6)
    , peak_sample(-1)
    , peak_ns(-1.e+6)
    , peak_amplitude(-1.)
    , size_sample(-1)
    , pedestal_mean(-10.)
    , pedestal_rms(-10.)
    , integral(-1000.)
    , offset(-20.)
    , saturation_correction(-1)
    , saturation_width(-1)
    , is_saturated(true) 
    {}

    int pulse_id;
    int start_sample; 	// sample in the fADC
    double start_ns; 	// basically start_sample/sample_rate_GHz-trigger_time_ns
    int peak_sample; 	// sample corresponding to the mimimum volt waveform (maximum spe waveform)
    double peak_ns; 	// time corresponding to peak_sample; mainly used for laser runs
    float peak_amplitude; // minimum amplitude in V (maximum in spe) of the waveform
    int size_sample; 	// size of the waveform in sample 
    float pedestal_mean; // in V
    float pedestal_rms;  // in V
    float integral;	// in V*s or spe
    float offset; 	// average amplitude of the waveform (integral/length); used in pedestal calibration runs;Will not be changed in Saturation.
    float saturation_correction; // same units of integral variable
    float saturation_width;
    bool is_saturated;

    //the following two function are helpful for debugging
    static std::string const PrintVarNames(){ 
      //static, because it is just a string, no variable content is printed
      //update if the variable content of this class/struct changes 
      std::string strBuffer(""); 
      strBuffer="Pulse: (pulse_id, start_sample, start_ns , peak_sample, peak_ns, size_sample, pedestal_mean, pedestal_rms, integral, saturation_correction, amplitude, is_saturated)"; 
      return strBuffer; 
    }
    
    std::string const PrintVarContent(){ //update if the variable content of this class/struct changes 
      std::stringstream sstrBuffer; 
      sstrBuffer << "Pulse: (" << pulse_id << ", " << start_sample << ", " << start_ns << ", " << peak_sample << ", " << peak_ns << ", " << peak_amplitude << ", " << size_sample << ", " << pedestal_mean << ", " << pedestal_rms << ", "<< integral << ", " << offset << ", " << saturation_correction << ", " << is_saturated << ")"; 
      return sstrBuffer.str(); 
    }
    
  };

  typedef std::vector<Pulse> pulses_t;

  static std::string const PrintVarNames(){ //static, because it is just a string, no variable content is printed
     std::stringstream sstrBuffer(""); 
     sstrBuffer << "ChannelData: (chassis, board, channel, channel_id, npulses, is_enabled, pulse vector: [index] " << Pulse::PrintVarNames() << "[, ...])"; 
     return sstrBuffer.str();
  }
  
  std::string const PrintVarContent(){ //update if the variable content of this class/struct changes 
      std::stringstream sstrBuffer; 
      sstrBuffer << "ChannelData: (" << chassis << ", " << board << ", " << channel << ", " << channel_id << ", " << npulses << ", " << is_enabled << ", "; 
      if(pulses.size()){
	for(unsigned i=0;i<pulses.size();++i) {
	  sstrBuffer << "[" << i <<  "] " << pulses.at(i).PrintVarContent();
	  if(i<pulses.size()-1) sstrBuffer << ", ";
	}
      } else {
	sstrBuffer << "[no pulses]"; 
      }

     
      sstrBuffer << ")";
      return sstrBuffer.str(); 
    }
  


  pulses_t pulses;
};




#endif
