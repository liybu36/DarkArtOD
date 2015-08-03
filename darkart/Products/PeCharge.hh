/*
  Definition of the PeCharge class.
  
  2014-10-22 AFan
 */

#ifndef darkart_Products_PeCharge_hh
#define darkart_Products_PeCharge_hh

#include <vector>

namespace darkart
{
  namespace MC
  {
    struct PeCharge;
    typedef std::vector<PeCharge> PeChargeVec;
    typedef std::vector<PeChargeVec> PeChargeVecVec;
  }
}

struct darkart::MC::PeCharge
{
  double charge;
  double time;

  PeCharge() :
    charge(0),
    time(0)
  { }
};

inline bool operator== (darkart::MC::PeCharge const& a, darkart::MC::PeCharge const& b)
{
  return (a.charge == b.charge &&
	  a.time == b.time);
}



#endif
