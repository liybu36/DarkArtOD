/*

  Definition of the EventInfo class
  
  AFan 2013-06-30

 */


#ifndef darkart_Products_EventInfo_hh
#define darkart_Products_EventInfo_hh

#include <stdint.h>
#include <iostream>

namespace darkart
{
  // Holds identifier info for each event
  struct EventInfo
  {
    // Constructor
    // Use the initializer list to set default values for all member data
    EventInfo():
      run_id(-1),
      subrun_id(-1),
      event_id(-1),
      status(NORMAL),
      //trigger_count(-1),
      timestamp_sec(0),
      timestamp_usec(0),
      dt_usec(0),
      event_time_usec(0),
      nchans(-1),
      saturated(false),
      gps_coarse(0),
      gps_fine(0),
      gps_pps(0),
      total_inhibit_time_us(0),
      incremental_inhibit_time_20ns(0),
      live_time_20ns(0),
      trigger_type(0),
      tpc_he_events_prescale(0),
      tpc_digital_sum(0),
      trigger_pattern(0),
      trigger_multiplicity(0),
      trigger_multiplicity_bot(0),
      trigger_multiplicity_top(0)
    { }

    // All the identifier info we want to store for each event
    int               run_id; //id of the run this event is from
    int            subrun_id; //id of the subrun this event is from
    int             event_id; //id of this event
    uint16_t          status; //flag denoting event status
    //int        trigger_count; //number of triggers (including unaccepted triggers)
    uint64_t   timestamp_sec; //unix timestamp for this event
    uint64_t  timestamp_usec; //usec part of the unix timestamp for this event
    uint64_t         dt_usec; //time since the last event in microseconds
    uint64_t event_time_usec; //time since run start in microseconds
    int               nchans; //physical channels that lit up
    bool           saturated; //true if any channel hit the limit of its digitizer range

    uint32_t gps_coarse; //GPS Coarse Time counter
    uint32_t gps_fine; //GPS Fine Time counter
    uint32_t gps_pps;

    uint32_t total_inhibit_time_us; //Total trigger inhibit time (uS)
    uint32_t incremental_inhibit_time_20ns; //Trigger inhibit time for the previous trigger (20 ns)
    uint32_t live_time_20ns; //Live time for the current trigger (20 ns)

    uint32_t trigger_type; //enum trigger_types
                           //{ tpc_high = 1, laser = 2, external = 3, pulser = 4, random = 5, tpc_low = 6, tpc = 7, veto_ls = 8, veto_cw = 9, };
                           //these are defined in ds50::V1495Fragment	
    uint32_t tpc_he_events_prescale; //prescale value for high energy events
    uint32_t tpc_digital_sum; //total sum of hits for trigger

    uint64_t trigger_pattern; //bitwise record of trigger pattern
    int      trigger_multiplicity;
    int      trigger_multiplicity_bot;
    int      trigger_multiplicity_top;
    
    enum STATUS_FLAGS { NORMAL=0,
                        ID_MISMATCH=1,
                        BAD_TIMESTAMP=2,
                        TRIGGER_MISMATCH=3
                        /*enter more here*/ };

    bool isValid() const {
      return (run_id>-1 && subrun_id>-1 && event_id>-1 && nchans>-1 && !saturated);
    }

    // AWW 24.11.13
    inline void print(int verbosity = 0) const {
      std::cout << "Event Info:"                                                 << std::endl;
      std::cout << "----------------------------------------"                    << std::endl;
      std::cout << "Run ID:                     " << run_id                      << std::endl;
      std::cout << "Subrun ID:                  " << subrun_id                   << std::endl;
      std::cout << "Event ID:                   " << event_id                    << std::endl;
      std::cout << "Timestamp     (s):          " << timestamp_sec               << std::endl;
      std::cout << "Event Time   (us):          " << event_time_usec             << std::endl;
      std::cout << "dt (us):                    " << dt_usec                     << std::endl;
      std::cout << "gps_pps    (20ns):          " << gps_pps                     << std::endl;
      std::cout << "gps_coarse    (s):          " << gps_coarse                  << std::endl;
      std::cout << "gps_fine   (20ns):          " << gps_fine                    << std::endl;
      std::cout << "Number of Channels:         " << nchans                      << std::endl;
      std::cout << "Saturated:                  " << saturated                   << std::endl;

      if(verbosity > 0){
        std::cout << "Status:                     " << status                      << std::endl;
        std::cout << "Timestamp (us):             " << timestamp_usec              << std::endl;
        std::cout << "Total Trigger Inhibit Time: " << total_inhibit_time_us       << std::endl; }
      if(verbosity > 1){
        /* placeholder */ }

      std::cout << std::endl;
    }

  };
}


#endif
