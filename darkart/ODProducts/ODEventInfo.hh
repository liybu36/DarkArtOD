/*

  2014-12-16 S. Davini

  ODEventInfo class to hold event-level configuration information.

 */

#ifndef darkart_ODProducts_ODEventInfo_hh
#define darkart_ODProducts_ODEventInfo_hh

#include <stdint.h>
//#include <float.h>
//#include <cstdint>
//#include <limits.h>
namespace darkart {
  namespace od {

    struct ODEventInfo
    {
      ODEventInfo()
        : run_id(-1)
        , subrun_id(-1)
        , event_id(-1)
        , gps_coarse(0)
        , gps_fine(0)//Can't make this work with UINT_MAX using limits.h and make the -1 in bad_time_alignment work
        , gps_pps(-1)
        , gps_timestamp_sec(-1.)
        , dt_usec(-1.)
        , trigger_type(12)//Can't make this work with UINT_MAX using limits.h and make the -1 in bad_time_alignment work
        , bad_time_alignment(-1)
      {}

      int run_id;
      int subrun_id;
      int event_id;
      
      uint32_t gps_coarse; 	// GPS Coarse Time counter
      uint32_t gps_fine; 	// GPS Fine Time counter. It counts the number of clock cycles (50 MHz) in the second 
      uint16_t gps_pps; 	// 1PPS counter. It counts the number of seconds since the start of the run
      double gps_timestamp_sec; // GPS timestamp in seconds of the event constructected using the 3 gps counters
      double dt_usec; //time since the last event in microsecond
      uint16_t trigger_type; //trigger types. Now it is empty      
      int bad_time_alignment; // 0=good alignment, 1=time missmatch between digitizers
      
      double makeGPSTimestamp(const uint16_t gps_pps, const uint32_t gps_fine) const { return (double(gps_fine)*20.e-9 + double(gps_pps));}
    };

  }//od
}//darkart


#endif
