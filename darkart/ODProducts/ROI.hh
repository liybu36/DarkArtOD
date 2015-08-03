/*

  2015-02-06 Y. Guardincerri

  ROI standing for "Region Of Interest" to hold information about the times where the roi are acquired.
  This holds values for each event contratry to ROICfg which stores information per Run.

 */

#ifndef darkart_ODProducts_ROI_hh
#define darkart_ODProducts_ROI_hh

#include <vector>

namespace darkart {
  namespace od {


    struct ROI
    {
      ROI()
        : id_vec()
        , charge_vec()
        , max_multiplicity_vec()
      {}
      
      std::vector<int> id_vec;
      std::vector<float> charge_vec;
      std::vector<int> max_multiplicity_vec;
    }; 
  }//od
}//darkart

#endif
