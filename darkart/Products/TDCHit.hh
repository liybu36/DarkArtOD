/*

  Defines the TDCHit data product. This will containing
  information from the V1190 TDCs regarding timing information
  for each TDC

  shawest 2013-10-20
*/

#ifndef darkart_Products_TDCHit_hh
#define darkart_Products_TDCHit_hh

#include <vector>

namespace darkart {
  struct TDCHit;
  typedef std::vector<TDCHit> TDCHitVec;
}

struct darkart::TDCHit
{
  bool found_leading;   //< Did we find a good leading edge?
  bool found_trailing;  //< Did we find a good trailing edge?
  int tdc;              //< TDC ID
  int bunch_id;         //< Not sure what this is...
  int event_count;      //< Number of events
  int event_id;         //< Event number
  int channel_id;       //< Channel the pulse appeared on
  float resolution_ns;  //< TDC Resolution in ns
  double leading_time;  //< Time of the leading edge
  double trailing_time; //< Time of the trailing edge
  double width;         //< Pulse width (leading_time - trailing_time)

  TDCHit():
    found_leading(false), found_trailing(false),
    tdc(-1),bunch_id(-1),
    event_count(-1), event_id(-1),
    channel_id(-1), resolution_ns(0.8),
    leading_time(-1), trailing_time(-1), 
    width(-1)
  {}
};

inline bool operator== (darkart::TDCHit const& a, darkart::TDCHit const& b)
{
  return ( a.found_leading == b.found_leading &&
	   a.found_trailing == b.found_trailing &&
	   a.tdc == b.tdc &&
	   a.bunch_id = b.bunch_id &&
	   a.event_count == b.event_count &&
	   a.event_id == b.event_id &&
	   a.channel_id == b.channel_id &&
	   a.resolution_ns == b.resolution_ns &&
	   a.leading_time == b.leading_time &&
	   a.trailing_time == b.trailing_time &&
	   a.width == b.width
	   );
}

inline bool isValid(darkart::TDCHit const& a)
{
  return ( a.found_leading == true &&
	   a.found_trailing == true &&
	   a.tdc >= 0 &&
	   a.bunch_id >= 0 &&
	   a.event_count >= 0 &&
	   a.event_id >= 0 &&
	   a.channel_id >= 0 &&
	   a.resolution_ns > 0 &&
	   a.leading_time >= 0 &&
	   a.trailing_time > a.leading_time &&
	   a.width > 0
	   );
}

#endif
