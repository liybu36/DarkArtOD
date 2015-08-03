/*

  This file is necessary to create the dictionary used to build
  the ROOT tree. This is necessary for every class that is an
  instantiation of a class template (e.g., std::vector<darkart::BaselineData>
  is an instantiation of the class template std::vector). Note
  this is necessary for the art::Wrapper instantiation for each
  top-level data product.

  For most data products, you may mimic the pattern of the
  BaselineData object below:
  1. wrapper for the object
  2. vector of objects
  3. wrapper for the vector of objects
  4. (as necessary) wrapper for the Assns product

  For many cases, the wrapper for the bare object (1) is not
  necessary; the vector of objects will be used. But add an entry
  for (1) anyway to avoid possible nasty debugging situations.

  Make sure to #include the corresponding header file.

 */
#include "darkart/ODProducts/RunInfo.hh"
#include "darkart/ODProducts/ODEventInfo.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"
#include "darkart/ODProducts/SumWF.hh"
#include "darkart/ODProducts/Cluster.hh"
#include "darkart/ODProducts/ROICfg.hh"
#include "darkart/ODProducts/ROI.hh"
#include "darkart/ODProducts/SliderCfg.hh"
#include "darkart/ODProducts/Slider.hh"
#include "darkart/ODProducts/ODEventData.hh"
#include "darkart/ODProducts/TotalCharge.hh"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include <vector>
#include <list>
#include <utility>

template class art::Wrapper<darkart::od::ODEventInfo>;

template class art::Wrapper<darkart::od::RunInfo::ChConfData>;
template class std::vector<darkart::od::RunInfo::ChConfData>;
template class art::Wrapper<darkart::od::RunInfo::DetectorCfg>;
template class std::vector<darkart::od::RunInfo::DetectorCfg>;
template class art::Wrapper<darkart::od::RunInfo>;

template class art::Wrapper<darkart::od::ChannelData::Pulse>;
template class std::vector<darkart::od::ChannelData::Pulse>;

template class art::Wrapper<darkart::od::ChannelData>;
template class std::vector<darkart::od::ChannelData>;
template class art::Wrapper<std::vector<darkart::od::ChannelData> >;

template class art::Wrapper<darkart::od::ChannelWFs::PulseWF>;
template class std::vector<darkart::od::ChannelWFs::PulseWF>;

template class art::Wrapper<darkart::od::ChannelWFs>;
template class std::vector<darkart::od::ChannelWFs>;
template class art::Wrapper<std::vector<darkart::od::ChannelWFs> >;

template class std::vector<std::pair<int, int > >;

template class art::Wrapper<darkart::od::SumWFSample>;
template class std::vector<darkart::od::SumWFSample>;
template class art::Wrapper<std::vector<darkart::od::SumWFSample> >;

template class art::Wrapper<darkart::od::Cluster>;
template class std::vector<darkart::od::Cluster>;
template class art::Wrapper<std::vector<darkart::od::Cluster> >;

template class art::Wrapper<darkart::od::ROICfg>;

template class art::Wrapper<darkart::od::ROI>;

template class art::Wrapper<darkart::od::SliderCfg>;

template class art::Wrapper<darkart::od::Slider>;

template class art::Wrapper<darkart::od::ODEventData>;

template class art::Wrapper<darkart::od::TotalCharge>;
