#ifndef darkart_VetoPMT_h
#define darkart_VetoPMT_h

// -*- C++ -*-
//
/*

*/
//

#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/UserInteraction/UserInteraction.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace darkart {

  typedef float LocArray[111][2];

  class VetoPMT 
  {
  public:
    VetoPMT(const fhicl::ParameterSet &, art::ActivityRegistry &);

    float theta(size_t index) const;
    float phi(size_t index) const;

  private:
    LocArray locs_;
  };
}

DECLARE_ART_SERVICE(darkart::VetoPMT, LEGACY)
#endif 

// Local Variables:
// mode: c++
// End:
