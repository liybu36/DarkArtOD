/*
  Definition of the PeHit class.
  
  2014-10-20 AFan
 */

#ifndef darkart_Products_PeHit_hh
#define darkart_Products_PeHit_hh

#include <vector>

namespace darkart
{
  namespace MC
  {
    struct PeHit;
    typedef std::vector<PeHit> PeHitVec;
  }
}

struct darkart::MC::PeHit
{
  int pmt;
  double time;

  PeHit() :
    pmt(-1),
    time(-1)
  { }
};

inline bool operator== (darkart::MC::PeHit const& a, darkart::MC::PeHit const& b)
{
  return (a.pmt == b.pmt && 
	  a.time == b.time);
}

#endif
