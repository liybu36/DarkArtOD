/*

  2015-03-24 X. Xiang

  This product stores total charge in both neutron veto and water tank,
  as well as channel charge

 */

#ifndef darkart_ODProducts_TotalCharge_hh
#define darkart_ODProducts_TotalCharge_hh

#include <vector>

namespace darkart {
  namespace od {


    struct TotalCharge
    {
      TotalCharge()
        : lsv_total_charge()
        , wt_total_charge()
        , chan_charge_vec()
      {}
      
      float lsv_total_charge;
      float wt_total_charge;
      std::vector<float> chan_charge_vec;
    }; 
  }//od
}//darkart

#endif
