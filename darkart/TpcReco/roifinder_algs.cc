#include "darkart/TpcReco/common_algs.hh"
#include "darkart/TpcReco/roifinder_algs.hh"

#include <iostream>
#include <cmath>
#include <numeric>

//________________________________________________________________________________
bool darkart::RoiFinderParams::isNotEmpty( int region ) const
{
  bool not_empty = true;
  if( roi_times.at(region).size() == 0 ) not_empty = false;

  return not_empty;
}


//_____________________________________________________________________________
darkart::RoiFinderParams::RoiFinderParams(std::vector< std::vector< double > > rt):
  roi_times(rt)
{ }


//-----------------------------------------------------------------------------
void darkart::findRoi(darkart::Waveform const& bswfm,
                      darkart::Waveform const& intWfm,
		      darkart::RoiVec & roiVec,
		      RoiFinderParams const& params)
{
  for (int i=0; i<params.numRegions(); i++) {
    const int RegionNum = i;
    if (params.isNotEmpty(RegionNum)) {
      darkart::Roi roi;
      int start_samp=bswfm.TimeToSample( params.roi_times.at(RegionNum).at(0) , true);//convert begin/end time to wfm index
      int end_samp=bswfm.TimeToSample( params.roi_times.at(RegionNum).at(1) , true);
      
      double max=bswfm.wave[start_samp];
      double max_t=bswfm.SampleToTime(start_samp);
      double min=bswfm.wave[start_samp];
      double min_t=bswfm.SampleToTime(start_samp);
      
      for(int samp=start_samp ; samp<end_samp ; ++samp){//cycle from starting location of samp to end
        if(bswfm.wave[samp]>max){ //find max amp and time
          max=bswfm.wave[samp];
          max_t=bswfm.SampleToTime(samp);
        }
        if(bswfm.wave[samp]<min){//find min amp and time
          min=bswfm.wave[samp];
          min_t=bswfm.SampleToTime(samp);
        }
      }
      
      
      //from params
      roi.region_number = RegionNum;
      roi.start_time = params.roi_times.at( RegionNum ).at(0);  
      roi.end_time = params.roi_times.at( RegionNum ).at(1);
      roi.start_index = start_samp;
      roi.end_index = end_samp;
      //from 'analysis'
      roi.max = max;
      roi.max_time = max_t; 
      roi.min = min;
      roi.min_time = min_t;
      roi.integral = integral(intWfm , start_samp, end_samp);
      //and we're done

      roiVec.push_back(roi);
    }
  }//end loop over regions
}
