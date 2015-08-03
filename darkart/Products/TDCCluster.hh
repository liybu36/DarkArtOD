/*
Definition of the TDCCluster class
*/
#ifndef darkart_Products_TDCCluster_hh
#define darkart_Products_TDCCluster_hh

#include <vector>

namespace darkart {

    struct TDCCluster {	
      int start_time_ns, stop_time_ns; // the first and latest leading edge time of hits in a cluster
      float mean_time_ns;              // mean leading edge time of hits in a cluster from start time of cluster
      short n_hits, n_long_hits;       // number of all hits in a cluster and number of hits between 20 ns to 200 ns from start time of a cluster
      float long_hits_avg;             // mean time of long hits
      unsigned char multiplicity[8];   // hits in each channel in a cluster
    };

    typedef std::vector<TDCCluster> TDCClusterVec;

    struct TDCClusters {
      TDCClusterVec ls_clusters;
      TDCClusterVec wc_clusters;
    };
}

#endif
