/*

  2015-02-06 Y. Guardincerri

  SliderCfg standing for "Slider" to hold information about the times where the sliders are acquired.
  This holds values for each event contratry to SliderCfg which stores information per Run.
 */

#ifndef darkart_ODProducts_Slider_hh
#define darkart_ODProducts_Slider_hh

#include <vector>

namespace darkart {
  namespace od {


    struct Slider
    {
      Slider()
        : id_vec()
        , charge_vec()
        , time_ns_vec()
        , max_multiplicity_vec()
      {}
      
      std::vector<int> id_vec;
      std::vector<float> charge_vec;
      std::vector<float> time_ns_vec;
      std::vector<int> max_multiplicity_vec;
    }; 
  }//od
}//darkart

#endif
