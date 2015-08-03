/*

  2015-02-03 Y. Guardincerri

  RunInfo class to hold run-level configuration information.

  RunInfo has 2 memebrs of type DetectorConfig. These store useful global information respectevely for LSV and WT

  It also has data members to store configuration settings for the ROIs and Sliders

 */

#ifndef darkart_ODProducts_RunInfo_hh
#define darkart_ODProducts_RunInfo_hh

#include <vector>
#include "ROICfg.hh"
#include "SliderCfg.hh"

namespace darkart {
  namespace od {


    struct RunInfo
    {
      RunInfo()
      : run_id(-1)
      , n_chassis(-1)
      , n_scopes(4,-1)
      , chHCD()
      , LSVCfg()
      , WTCfg()
      , roicfg_lsv()
      , roicfg_wt()
      , slidercfg_lsv()
      , slidercfg_wt()
      {}

      struct ChConfData
      {
        ChConfData()
        : channel_type(0)
        , sample_rate(-1.)
        , record_length(-1)
        , reference_pos(-1)
        , is_enabled(false)
        , vertical_range(-1)
        , vertical_offset(-1)
        , zs_enabled(-1)
        , zs_threshold(-1)
        , zs_minwidth(-1)
        , zs_pre(-1)
        , zs_post(-1)
        {}

	int channel_type; //0: default, 1: LSV, 2: WT, -1: disabled LSV, -2: disabled WT
        double sample_rate;
        int record_length;
        double reference_pos;
        bool is_enabled;
        double vertical_range;
        double vertical_offset;
        double zs_enabled;
        double zs_threshold;
        double zs_minwidth;
        double zs_pre;
        double zs_post;
      };
 
      struct DetectorCfg{
        DetectorCfg()
        : same_configuration(false)
        , sample_rate_Hz(-1.)
        , record_length(-1)
        , reference_pos(-1.)
        {}
        
        bool same_configuration;
        double sample_rate_Hz;
        int record_length;
        double reference_pos; 
      };
 
      int run_id;
      int n_chassis;
      std::vector<int> n_scopes;
      std::vector<ChConfData> chHCD;
      DetectorCfg LSVCfg;
      DetectorCfg WTCfg;
      darkart::od::ROICfg roicfg_lsv;
      darkart::od::ROICfg roicfg_wt;
      darkart::od::SliderCfg slidercfg_lsv;
      darkart::od::SliderCfg slidercfg_wt;
      
      // return time of the sample in ns with respect to the trigger
      double sampleTonsLSV(int samp) { return LSVCfg.same_configuration ? (samp - LSVCfg.reference_pos*LSVCfg.record_length)/(LSVCfg.sample_rate_Hz/1.e+9) : -9999999.; };
      double sampleTonsWT(int samp) { return WTCfg.same_configuration ? (samp - WTCfg.reference_pos*WTCfg.record_length)/(WTCfg.sample_rate_Hz/1.e+9) : -9999999.; };

    };

  }//od
}//darkart


#endif
