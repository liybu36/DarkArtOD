#ifndef darkart_TpcReco_roifinder_algs_hh
#define darkart_TpcReco_roifinder_algs_hh

#include "darkart/Products/Waveform.hh"
#include "darkart/Products/Roi.hh"

#include <vector>


namespace darkart
{

  struct RoiFinderParams {
    RoiFinderParams() {}
    RoiFinderParams(std::vector< std::vector<double> > rt);
    
    std::vector< std::vector< double> > roi_times; //vector containing range of roi times (microseconds)
    int numRegions() const { return roi_times.size(); }
    bool isNotEmpty( int region ) const;
  };

  
  void findRoi(darkart::Waveform const& bsWfm,
               darkart::Waveform const& intWfm,
               darkart::RoiVec & roiVec,
	       RoiFinderParams const& params);

}


#endif
