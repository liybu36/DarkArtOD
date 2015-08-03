#ifndef darkart_ODProducts_ChannelWFs_hh
#define darkart_ODProducts_ChannelWFs_hh

#include <vector>
#include <list>
#include <sstream>
#include <string>

//Forward declarations
namespace darkart {
  namespace od {
    struct ChannelWFs;
    typedef std::vector<ChannelWFs> ChannelWFsVec;
  }
}


struct darkart::od::ChannelWFs
{
  ChannelWFs()
    : channel_id(-1)
    , pulse_wfs()
  {}

  int channel_id;
  
  struct PulseWF {
    int start_ns; //wrt trigger, same as start_ns in pulse vector of ChannelData class
    int pulse_id;
    std::vector<float> data;

    //converts fADC samples wrt to trigger time
    double sampleToTime(const int sample, const double sample_rate_GHz=1.25) const
    { return start_ns + sample/sample_rate_GHz; }

    //converts time in ns to fADC samples.
    //By default the time is checked for being within the range of the pulse.
    int timeToSample(const double time_ns,
                     const bool   checkrange=true,
                     const double sample_rate_GHz=1.25) const
    {
      if(checkrange){
	//truncate to start begin and end of the waveform
	if(time_ns<start_ns) return 0;
	if(time_ns>start_ns+data.size()/sample_rate_GHz) return data.size()-1;
      }
      return (time_ns-start_ns)*sample_rate_GHz;
    }//timeToSample 


    
    //the following two function are helpful for debugging
    static std::string const PrintVarNames(){ 
      //static, because it is just a string, no variable content is printed
      //update if the variable content of this class/struct changes 
      std::string strBuffer(""); 
      strBuffer="PulseWF: (pulse_id, start_ns, [wfs not printed])"; //waveforms would be too much 
      return strBuffer; 
    }
    std::string const PrintVarContent(){ //update if the variable content of this class/struct changes 
      std::stringstream sstrBuffer; 
      sstrBuffer << "PulseWF: (" << pulse_id << ", " << start_ns << ")"; 
      return sstrBuffer.str(); 
    }


  }; //PulseWF

  typedef std::vector<PulseWF> pulseWFs_t;
  static std::string const PrintVarNames(){ //static, because it is just a string, no variable content is printed
     std::stringstream sstrBuffer(""); 
     sstrBuffer << "ChannelWFs: (channel_id, pulse_wfs vector: [(" << PulseWF::PrintVarNames() << "), ...])"; 
     return sstrBuffer.str();
  }
  std::string const PrintVarContent(){ //update if the variable content of this class/struct changes 
      std::stringstream sstrBuffer; 
      sstrBuffer << "ChannelWFs: (" << channel_id << ", "; 
      const int pulse_wf_size = int(pulse_wfs.size());
      if(pulse_wf_size){
	for(int i=0;i<pulse_wf_size;++i) {
	  sstrBuffer << "[" << i <<  "] " << pulse_wfs.at(i).PrintVarContent();
	  if(i<pulse_wf_size-1) sstrBuffer << ", ";
	}
      } else {
	sstrBuffer << "[no pulse_wfs]"; 
      }

     
      sstrBuffer << ")";
      return sstrBuffer.str(); 
    }

  pulseWFs_t pulse_wfs;
};




#endif
