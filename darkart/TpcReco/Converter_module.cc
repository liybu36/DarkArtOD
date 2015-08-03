////////////////////////////////////////////////////////////////////////
// Class:       Converter
// Module Type: producer
// File:        Converter_module.cc
//
// Generated at Thu Mar 21 20:32:50 2013 by James Kowalkowski using artmod
// from art v1_03_03.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/InputTag.h"

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/Fragments.hh"
#include "artdaq-core/Data/detail/RawFragmentHeader.hh"

#include "darksidecore/Data/V172xFragment.hh"
#include "darksidecore/Data/V1495Fragment.hh"
#include "darksidecore/Data/Config.hh"
#include "darksidecore/ArtServices/DBInterface.hh"

#include "darkart/Products/EventInfo.hh"
#include "darkart/Products/Channel.hh"
#include "darkart/Products/Pmt.hh"
#include "darkart/Products/Waveform.hh"
#include "darkart/Products/WaveformInfo.hh"
#include "darkart/TpcReco/converter_algs.hh"
#include "darkart/Products/ProductAssns.hh"
#include "darkart/TpcReco/utilities.hh"


#include <map>
#include <utility>
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

//----------------------------------------------------------------------------
// Class Converter is an EDProducer that creates several products: 
// 1. EventInfo object to hold identifier info for the event
// 2. Channel object to hold identifier info for each channel
// 3. Pmt object to hold relevant info for each PMT; stored in art::Run
// 4. Waveforms object to hold the raw waveforms; stored separately to be able
//    to turn off saving full waveforms to output files.
//----------------------------------------------------------------------------

namespace darkart {
  class Converter;
}


class darkart::Converter : public art::EDProducer {
public:
  explicit Converter(fhicl::ParameterSet const & p);
  virtual ~Converter();

  void produce(art::Event & e) override;
  void beginRun(art::Run & r) override;

private:
  art::InputTag v1720_tag_;
  art::InputTag v1495_tag_;
  std::vector<int> skip_channels_;
  
  ConverterAlgs algs;

  bool read_SPEmeans_from_file;
  std::string spemeans_file;
  std::string db_table_version;
  int laser_run;

};


//----------------------------------------------------------------------------
// The implementation of the module member functions begins here.

darkart::Converter::Converter(fhicl::ParameterSet const & ps ) :
  v1720_tag_(ps.get<std::string>("v1720_tag")),
  v1495_tag_(ps.get<std::string>("v1495_tag")),
  skip_channels_(ps.get<std::vector<int> >("skip_channels", std::vector<int>())),
  read_SPEmeans_from_file(ps.get<bool>("read_SPEmeans_from_file", false)),
  spemeans_file(ps.get<std::string>("spemeans_file","")),
  db_table_version(ps.get<std::string>("db_table_version")),
  laser_run(ps.get<int>("laser_run", -999))
{
  produces<darkart::WaveformVec>();
  produces<darkart::WaveformInfoVec>();
  produces<darkart::EventInfo>();
  produces<darkart::ChannelVec>();
  produces<darkart::WaveformAssns>();
  produces<darkart::WaveformInfoAssns>();
  produces<darkart::PmtVec, art::InRun>();
}


//----------------------------------------------------------------------------
darkart::Converter::~Converter()
{ }


//----------------------------------------------------------------------------
void darkart::Converter::beginRun(art::Run & r)
{

  // Access the database and fill in pmt object here

  std::unique_ptr<darkart::PmtVec> pmtVec(new darkart::PmtVec);

  // 2013-11-09 -- It appears the database still has old pmt geometries. Hard
  // code the positions until the db is updated.
  std::map<int, double> xpos, ypos, zpos, area, qe;

  xpos.emplace( 0,    0); ypos.emplace( 0,  -4); zpos.emplace( 0,-1); qe.emplace( 0,.327);
  xpos.emplace( 1,-1.73); ypos.emplace( 1,  -3); zpos.emplace( 1,-1); qe.emplace( 1,.334);
  xpos.emplace( 2,-3.46); ypos.emplace( 2,  -2); zpos.emplace( 2,-1); qe.emplace( 2,.356);
  xpos.emplace( 3,-3.46); ypos.emplace( 3,   0); zpos.emplace( 3,-1); qe.emplace( 3,.345);
  xpos.emplace( 4,-3.46); ypos.emplace( 4,   2); zpos.emplace( 4,-1); qe.emplace( 4,.325);
  xpos.emplace( 5,-1.73); ypos.emplace( 5,   1); zpos.emplace( 5,-1); qe.emplace( 5,.337);
  xpos.emplace( 6,-1.73); ypos.emplace( 6,  -1); zpos.emplace( 6,-1); qe.emplace( 6,.342);
  xpos.emplace( 7,    0); ypos.emplace( 7,  -2); zpos.emplace( 7,-1); qe.emplace( 7,.352);
  xpos.emplace( 8, 1.73); ypos.emplace( 8,  -3); zpos.emplace( 8,-1); qe.emplace( 8,.325);
  xpos.emplace( 9, 3.46); ypos.emplace( 9,  -2); zpos.emplace( 9,-1); qe.emplace( 9,.336);
  xpos.emplace(10, 1.73); ypos.emplace(10,  -1); zpos.emplace(10,-1); qe.emplace(10,.357);
  xpos.emplace(11,    0); ypos.emplace(11,   0); zpos.emplace(11,-1); qe.emplace(11,.368);
  xpos.emplace(12,    0); ypos.emplace(12,   2); zpos.emplace(12,-1); qe.emplace(12,.341);
  xpos.emplace(13,-1.73); ypos.emplace(13,   3); zpos.emplace(13,-1); qe.emplace(13,.336);
  xpos.emplace(14,    0); ypos.emplace(14,   4); zpos.emplace(14,-1); qe.emplace(14,.347);
  xpos.emplace(15, 1.73); ypos.emplace(15,   3); zpos.emplace(15,-1); qe.emplace(15,.314);
  xpos.emplace(16, 1.73); ypos.emplace(16,   1); zpos.emplace(16,-1); qe.emplace(16,.362);
  xpos.emplace(17, 3.46); ypos.emplace(17,   0); zpos.emplace(17,-1); qe.emplace(17,.355);
  xpos.emplace(18, 3.46); ypos.emplace(18,   2); zpos.emplace(18,-1); qe.emplace(18,.339);
  xpos.emplace(19,-3.46); ypos.emplace(19,  -2); zpos.emplace(19, 1); qe.emplace(19,.375);
  xpos.emplace(20,-1.73); ypos.emplace(20,  -3); zpos.emplace(20, 1); qe.emplace(20,.355);
  xpos.emplace(21,    0); ypos.emplace(21,  -4); zpos.emplace(21, 1); qe.emplace(21,.348);
  xpos.emplace(22, 1.73); ypos.emplace(22,  -3); zpos.emplace(22, 1); qe.emplace(22,.333);
  xpos.emplace(23, 3.46); ypos.emplace(23,  -2); zpos.emplace(23, 1); qe.emplace(23,.329);
  xpos.emplace(24, 1.73); ypos.emplace(24,  -1); zpos.emplace(24, 1); qe.emplace(24,.333);
  xpos.emplace(25,    0); ypos.emplace(25,  -2); zpos.emplace(25, 1); qe.emplace(25,.348);
  xpos.emplace(26,-1.73); ypos.emplace(26,  -1); zpos.emplace(26, 1); qe.emplace(26,.322);
  xpos.emplace(27,-3.46); ypos.emplace(27,   0); zpos.emplace(27, 1); qe.emplace(27,.354);
  xpos.emplace(28,-3.46); ypos.emplace(28,   2); zpos.emplace(28, 1); qe.emplace(28,.357);
  xpos.emplace(29,-1.73); ypos.emplace(29,   1); zpos.emplace(29, 1); qe.emplace(29,.347);
  xpos.emplace(30,    0); ypos.emplace(30,   0); zpos.emplace(30, 1); qe.emplace(30,.353);
  xpos.emplace(31, 1.73); ypos.emplace(31,   1); zpos.emplace(31, 1); qe.emplace(31,.345);
  xpos.emplace(32, 3.46); ypos.emplace(32,   0); zpos.emplace(32, 1); qe.emplace(32,.339);
  xpos.emplace(33, 3.46); ypos.emplace(33,   2); zpos.emplace(33, 1); qe.emplace(33,.313);
  xpos.emplace(34, 1.73); ypos.emplace(34,   3); zpos.emplace(34, 1); qe.emplace(34,.333);
  xpos.emplace(35,    0); ypos.emplace(35,   2); zpos.emplace(35, 1); qe.emplace(35,.334);
  xpos.emplace(36,-1.73); ypos.emplace(36,   3); zpos.emplace(36, 1); qe.emplace(36,.328);
  xpos.emplace(37,    0); ypos.emplace(37,   4); zpos.emplace(37, 1); qe.emplace(37,.336);




  //try to read SPE means & sigmas from art::Run (may exist if reading MC file)
  art::Handle<double> speMeanHandle; r.getByLabel("makeCharge:SPEmean", speMeanHandle);
  art::Handle<double> speSigmaHandle; r.getByLabel("makeCharge:SPEsigma", speSigmaHandle);
  if (speMeanHandle.isValid() && speSigmaHandle.isValid()) {
    LOG_INFO("SPEMEAN") << "\nReading SPE mean and sigma directly from file.\n" << std::endl;
    for (int ch_id=0; ch_id<38; ++ch_id) {
      darkart::Pmt pmt;
      pmt.channel_id = ch_id;
      pmt.spe_mean = *speMeanHandle;
      pmt.spe_sigma = *speSigmaHandle;
      pmt.laser_run = -1;
      pmt.photocathode_x = xpos[ch_id];
      pmt.photocathode_y = ypos[ch_id];
      pmt.photocathode_z = zpos[ch_id];
      pmt.qe = qe[ch_id];
      if (pmt.photocathode_r()>3.8)
        pmt.photocathode_area = .712;
      else
        pmt.photocathode_area = 1.;
      pmtVec->push_back(pmt);

    }
    r.put(std::move(pmtVec));
    return;
  }
  
  
  if (!read_SPEmeans_from_file) {
    art::ServiceHandle<ds50::DBInterface> dbi;
    int laser_run_to_use = laser_run;
    if (laser_run == -999)
      laser_run_to_use = dbi->latest_run("dark_art.laser_calibration", db_table_version);
    ds50::db::result res = dbi->run(laser_run_to_use, "dark_art.laser_calibration", db_table_version);

    LOG_INFO("Database") << "\nLoading SPE calibration info from run " << res.get<int>("run",0) << "\n"<<std::endl;
  
    const size_t ncells = res.cell_elements("channel_id", 0);
    for (size_t i=0; i<ncells; i++) {

      int ch_id = res.get<int>("channel_id", 0, i);
      double spe_mean = res.get<double>("ser_mean", 0, i);
      double spe_sigma = res.get<double>("ser_sigma", 0, i);
      
      // check if values in db are NaN; if so, set value to -1
      if (spe_mean != spe_mean) {
        LOG_ERROR("Database")<<"Entry ser_mean is NaN in cell "<<i
                             <<" (channel "<<ch_id<<")"<<std::endl;
        spe_mean = -1;
      }
      if (spe_sigma != spe_sigma) {
        LOG_ERROR("Database")<<"Entry ser_sigma is NaN in cell "<<i
                             <<" (channel "<<ch_id<<")"<<std::endl;
        spe_sigma = -1;
      }
      
      
      if (!util::skipChannel(ch_id, skip_channels_)) {
        darkart::Pmt pmt;
        pmt.channel_id = ch_id;
        pmt.spe_mean = spe_mean;
        pmt.spe_sigma = spe_sigma;
        pmt.laser_run = laser_run_to_use;
        pmt.photocathode_x = xpos[ch_id];
        pmt.photocathode_y = ypos[ch_id];
        pmt.photocathode_z = zpos[ch_id];
        pmt.qe = qe[ch_id];
        if (pmt.photocathode_r()>3.8)
          pmt.photocathode_area = .712;
        else
          pmt.photocathode_area = 1.;
        pmtVec->push_back(pmt);
      }
    }
  }


  else { //read_SPEmeans_from_file
    std::map<int, double> spe_means;
    int ch;
    double spe_mean;
    std::ifstream txt;
    std::string line;
    // These are SPE means produced by daqman's laserrun from run 5183.
    //txt.open("/ds50/app/user/aldenf/work/build/darkart/spemeans.txt",std::fstream::in);
    txt.open(spemeans_file.c_str(), std::fstream::in);
    if (txt.is_open() && txt.good()) {
      while(getline(txt, line)) {
        std::istringstream ss1(line);
        std::string temp;
        getline(ss1, temp, ' ');
        std::stringstream ss2(temp);
        ss2 >> ch;
        getline(ss1, temp, ' ');
        std::istringstream ss3(temp);
        ss3 >> spe_mean;
        spe_means.insert( std::pair<int, double>(ch, spe_mean) );
      }
    }
  

  //assuming that neither spe_sigma nor laser run number is available through the text file.
    for (int i=0; i<40; i++) {
      if (!util::skipChannel(i, skip_channels_)) {
        darkart::Pmt pmt;
        pmt.channel_id = i;
        if (spe_means.find(i) == spe_means.end()){
          pmt.spe_mean = 1.;
	  //pmt.spe_sigma = 0.; 
	  //pmt.laser_run = laser_run_to_use;
        } else {
          pmt.spe_mean = spe_means[i];
	  //pmt.spe_sigma = 0.; 
	  //pmt.laser_run = laser_run_to_use;
        }
        pmt.photocathode_x = xpos[i];
        pmt.photocathode_y = ypos[i];
        pmt.photocathode_z = zpos[i];
        pmtVec->push_back(pmt);
      }
    }
  }





  


  
  // put the pmt object into the Run
  r.put(std::move(pmtVec));

}




//----------------------------------------------------------------------------
void darkart::Converter::produce(art::Event & e)
{
    
  // Get handles to V1720 data block.
  // 2013-07-12 AFan -- At the moment, we're not using the V1724 block. Since
  // I'm not sure how it should be implemented when we do use it, I'm leaving it
  // out for now.

  auto v1720 = e.getValidHandle<artdaq::Fragments>(v1720_tag_);
  
  // Make our products, which begin empty.
  std::unique_ptr<darkart::WaveformVec>       rawWfmVec(new darkart::WaveformVec);
  std::unique_ptr<darkart::WaveformInfoVec>   wfmInfoVec(new darkart::WaveformInfoVec);
  std::unique_ptr<darkart::EventInfo>         eventInfo(new darkart::EventInfo);
  std::unique_ptr<darkart::ChannelVec>        chVec(new darkart::ChannelVec);
  std::unique_ptr<darkart::WaveformAssns>     wfmAssns(new darkart::WaveformAssns);
  std::unique_ptr<darkart::WaveformInfoAssns> wfmInfoAssns(new darkart::WaveformInfoAssns);


  // Populate the Waveforms object with the raw waveform data and the channel
  // object with info about each channel
  algs.convertFragments( *v1720, *rawWfmVec, *wfmInfoVec, *chVec, skip_channels_ );
  
  
  // Populate the eventInfo object. Some of the event-level identifier info is
  // stored in the V1495 block. 2013-07-24 -- For now (DS50), there is only one V1495
  // Fragment, so read only the front() of the Fragments vector.
  auto v1495 = e.getValidHandle<artdaq::Fragments>(v1495_tag_);
  if ( v1495->front().hasMetadata()){
    *eventInfo = algs.fillEventInfo( v1495->front(), *chVec );
  }
  eventInfo->subrun_id = static_cast<int>(e.subRun());
  if (static_cast<uint>(eventInfo->event_id) != e.event())
    LOG_ERROR("Event ID") << "V1495 trigger counter does not match art::Event::event()."<<std::endl;
  
  // Fill the Assns object for the raw waveforms
  for (size_t idx=0; idx<chVec->size(); ++idx) {
    chVec->at(idx).channelID.channel_index = idx;

    if (!rawWfmVec->at(idx).isValid() || !wfmInfoVec->at(idx).isValid())
      LOG_WARNING("RawData")<<"Inserting object in non-valid state for channel "<<chVec->at(idx).channel_id();
    
    util::createAssn(*this, e,
                     *chVec, chVec->at(idx),
                     *rawWfmVec, rawWfmVec->at(idx),
                     *wfmAssns);

    util::createAssn(*this, e,
                     *chVec, chVec->at(idx),
                     *wfmInfoVec, wfmInfoVec->at(idx),
                     *wfmInfoAssns);
  }


  // Put our complete products into the Event.
  e.put(std::move(rawWfmVec));
  e.put(std::move(wfmInfoVec));
  e.put(std::move(eventInfo));
  e.put(std::move(chVec));
  e.put(std::move(wfmAssns));
  e.put(std::move(wfmInfoAssns));

}


DEFINE_ART_MODULE(darkart::Converter)
