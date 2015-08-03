/*

  2015-02-06 Y. Guardincerri

  ROICfg standing for "Region Of Interest Configuration" to hold information about the times where the roi are acquired.
  This holds values once per run contratry to ROI which stores information per event.

 */

#ifndef darkart_ODProducts_ROICfg_hh
#define darkart_ODProducts_ROICfg_hh

#include <vector>

namespace darkart {
  namespace od {


    struct ROICfg
    {
      ROICfg()
        : id_vec()
        , start_ns_vec()
        , end_ns_vec()
        , label_vec()
      {}
      
      std::vector<int> id_vec;
      std::vector<float> start_ns_vec;
      std::vector<float> end_ns_vec;
      std::vector<int> label_vec;
    }; 
  }//od
}//darkart

#endif
