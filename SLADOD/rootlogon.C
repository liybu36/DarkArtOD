#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;
  //#pragma link C++ class darkart::od::EventData;
  //#pragma link C++ class darkart::od;

#endif
{
  gSystem->AddIncludePath("-I\..\/");

  gROOT->ProcessLine(".L ../darkart/Products/EventInfo.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/Channel.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/Baseline.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/Pmt.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/WaveformInfo.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/Pulse.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/PulseParam.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/PositionParam.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/Spe.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/Roi.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/EventData.hh+");
  gROOT->ProcessLine(".L ../darkart/Products/TDCCluster.hh+");


  gROOT->ProcessLine(".L ../darkart/ODProducts/ODEventInfo.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/RunInfo.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/ChannelData.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/ChannelWFs.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/TotalCharge.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/SumWF.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/ROICfg.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/ROI.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/SliderCfg.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/Slider.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/Cluster.hh+");
  gROOT->ProcessLine(".L ../darkart/ODProducts/ODEventData.hh+");

  gROOT->ProcessLine(".L ../darkart/Products/TDCCluster.hh+");

  //  gROOT->ProcessLine(".L xylocator/xy_pmtdata.C+");
  // gROOT->ProcessLine(".L xylocator/xylocator.C+");


}
