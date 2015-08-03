/*

  Defines the od::EventData data product. Will be a 'super-product'
  which contains all the information from other products. This
  product will be written to a separate output ROOT TTree in a
  separate output file. High-level analysis is done primarily
  on this object.

  AFan 2015-02-06

 */


#ifndef darkart_ODProducts_ODEventData_hh
#define darkart_ODProducts_ODEventData_hh

#include "ODEventInfo.hh"
#include "RunInfo.hh"
#include "ChannelData.hh"
//#include "ChannelWFs.hh"  //We don't save waveforms
//#include "SumWF.hh"       //We don't save waveforms
#include "Cluster.hh"

#include "ROI.hh"
#include "Slider.hh"
#include "TotalCharge.hh"

#include <vector>
#include <iostream>
#include <string>

// Forward declaration
namespace darkart {
  namespace od {
    struct ODEventData;
  }
}

struct darkart::od::ODEventData
{
  std::string darkart_version;
  darkart::od::ODEventInfo event_info;
  std::vector<darkart::od::ChannelData> channel_data;
  int nclusters_lsv;
  int nclusters_wt;
  std::vector<darkart::od::Cluster> clusters_lsv;
  std::vector<darkart::od::Cluster> clusters_wt;

  darkart::od::ROI roi_lsv;
  darkart::od::ROI roi_wt;
  darkart::od::Slider slider_lsv;
  darkart::od::Slider slider_wt;
  darkart::od::TotalCharge total_charge;
  void clear();
  
};

inline void darkart::od::ODEventData::clear()
{
  darkart_version = "";
  event_info = darkart::od::ODEventInfo();
  channel_data.clear();
  nclusters_lsv = 0;
  nclusters_wt = 0;
  clusters_lsv.clear();
  clusters_wt.clear();
}
                                        



#endif
