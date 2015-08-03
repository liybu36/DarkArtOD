#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;

/*
#pragma link C++ struct darkart::EventInfo+;
#pragma link C++ struct darkart::Channel+;
#pragma link C++ struct darkart::ChannelData+;
#pragma link C++ struct darkart::Channel::ChannelID+;
#pragma link C++ struct darkart::Baseline+;
#pragma link C++ struct darkart::Pmt+;
#pragma link C++ struct darkart::WaveformInfo+;
#pragma link C++ struct darkart::Pulse+;
#pragma link C++ struct darkart::PulseData+;
#pragma link C++ struct darkart::Pulse::PulseID+;
#pragma link C++ struct darkart::PulseParam+;
#pragma link C++ struct darkart::PositionParam+;
#pragma link C++ struct darkart::Roi+;
#pragma link C++ struct darkart::EventData+;
#pragma link C++ struct darkart::Spe+;
*/
#pragma link C++ struct darkart::od::ODEventInfo+;
#pragma link C++ struct darkart::od::RunInfo+;
#pragma link C++ struct darkart::od::RunInfo::ChConfData+;
#pragma link C++ struct darkart::od::RunInfo::DetectorCfg+;
#pragma link C++ struct darkart::od::ChannelData+;
#pragma link C++ struct darkart::od::ChannelData::Pulse+;
//#pragma link C++ struct darkart::od::ChannelWFs+;
#pragma link C++ struct darkart::od::TotalCharge+;
#pragma link C++ struct darkart::od::SumWF+;
#pragma link C++ struct darkart::od::ROI+;
#pragma link C++ struct darkart::od::ROICfg+;
#pragma link C++ struct darkart::od::Slider+;
#pragma link C++ struct darkart::od::SliderCfg+;
#pragma link C++ struct darkart::od::Cluster+;
#pragma link C++ struct darkart::od::Cluster::Shape+;
#pragma link C++ struct darkart::od::Cluster::PMTDistribution+;
#pragma link C++ struct darkart::od::ODEventData+;

//#pragma link C++ class xylocator+;
//#pragma link C++ class xy_pmtdata+;

#pragma link C++ class std::vector < std::vector<int> >+;
#pragma link C++ class std::vector < std::vector<float> >+;
#pragma link C++ class std::vector < std::vector<double> >+;
#pragma link C++ class std::vector < std::vector<bool> >+;

#endif
