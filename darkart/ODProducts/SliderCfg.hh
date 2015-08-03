/*

  2015-02-06 Y. Guardincerri

  SliderCfg standing for "Slider Configuration" to hold information about the times where the sliders are acquired.
  This holds values once per run contratry to Slider which stores information per event.
 */

#ifndef darkart_ODProducts_SliderCfg_hh
#define darkart_ODProducts_SliderCfg_hh

#include <vector>

namespace darkart {
  namespace od {


    struct SliderCfg
    {
      SliderCfg()
        : id_vec()
        , start_ns_vec()
        , end_ns_vec()
        , width_ns_vec()
        , label_vec()
      {}
      
      std::vector<int> id_vec;
      std::vector<float> start_ns_vec;
      std::vector<float> end_ns_vec;
      std::vector<float> width_ns_vec;
      std::vector<int> label_vec;
    }; 
  }//od
}//darkart

#endif
