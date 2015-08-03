////////////////////////////////////////////////////////////////////////
// Class:       mcWaveform
// Module Type: producer
// File:        mcWaveform_module.cc
//
// Generated at Sat Dec 14 17:08:34 2013 by Alden Fan using artmod
// from cetpkgsupport v1_02_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"


#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/Fragments.hh"
#include "darksidecore/Data/V172xFragment.hh"
#include "darksidecore/ArtServices/DBInterface.hh"


#include <memory>
#include <vector>
#include <iostream>


//----------------------------------------------------------------------------
// This module retrieves the raw baseline waveforms and provides a starting
// point to overlay MC results. It also shows how to access the database to
// get the SER means and widths.
// 
// This is meant only as an example to show how some tasks can be done and so
// is a sort of all-in-one module that uses very simple data structures. As the
// tasks become more complex, it is advisable to split up tasks among more than
// one module, which will likely require more sophisticated data structures.
// You could use DarkArt structures or artdaq or ds50daq structures, or you could
// write new structures. I'm not sure which is most appropriate. This could be
// discussed with Kurt Biery.
//----------------------------------------------------------------------------



namespace darkart {
  class mcWaveform;
}

class darkart::mcWaveform : public art::EDProducer {
public:
  explicit mcWaveform(fhicl::ParameterSet const & p);
  virtual ~mcWaveform();

  void produce(art::Event & e) override;
  void beginRun(art::Run & r) override;


private:

  art::InputTag v1720_tag_;
  std::string db_table_version_;

  std::vector<double> ser_means;
  std::vector<double> ser_sigmas;

};


darkart::mcWaveform::mcWaveform(fhicl::ParameterSet const & p):
  v1720_tag_(p.get<std::string>("v1720_tag")),
  db_table_version_(p.get<std::string>("db_table_version"))
{
  produces<std::vector<std::vector<double> > >();
}

darkart::mcWaveform::~mcWaveform()
{ }

void darkart::mcWaveform::beginRun(art::Run & r)
{
  /////////////////////////////////////////////////////////////////
  // You could open an ifstream here to access the MC results.


  
  /////////////////////////////////////////////////////////////////
  // Access the DB to retrieve the SER means and sigmas.
  art::ServiceHandle<ds50::DBInterface> dbi;

  // Currently, DB can only find the last processed laser run relative to current run.
  // When the next version of ds50daq is released, will be able to retrieve a specific
  // laser run.
  ds50::db::result res = dbi->latest("dark_art.laser_calibration", db_table_version_);
  
  LOG_INFO("Database") << "\nLoading SPE calibration info from run "
                       << res.get<int>("run",0) << "\n"<<std::endl;

  const size_t ncells = res.cell_elements("channel_id", 0);
  for (size_t i=0; i<ncells; i++) {
    int ch_id = res.get<int>("channel_id", 0, i);
    double ser_mean = res.get<double>("ser_mean", 0, i);
    double ser_sigma = res.get<double>("ser_sigma", 0, i);

    // check if values in db are NaN; if so, insert -1
    if (ser_mean != ser_mean) {
      LOG_ERROR("Database")<<"Entry ser_mean is NaN in cell "<<i
                           <<" (channel "<<ch_id<<")"<<std::endl;
      ser_means.push_back(-1);
    }
    else
      ser_means.push_back(ser_mean);
    if (ser_sigma != ser_sigma) {
      LOG_ERROR("Database")<<"Entry ser_sigma is NaN in cell "<<i
                           <<" (channel "<<ch_id<<")"<<std::endl;
      ser_sigmas.push_back(-1);
    }
    else
      ser_sigmas.push_back(ser_sigma);
  }

  // Do something stupid with r because buildtool fatally complains about unused variables.
  std::cout << r.run() << std::endl;
}

void darkart::mcWaveform::produce(art::Event & e)
{
  /////////////////////////////////////////////////////////////////
  // Retrieve the raw waveforms.

  // Get handle to V1720 data block.
  auto v1720 = e.getValidHandle<artdaq::Fragments>(v1720_tag_);

  // Make our data product, which begins empty.
  std::unique_ptr<std::vector<std::vector<double> > > wfmVec(new std::vector<std::vector<double> >());

  std::vector<int> channelIDs;
  
  for (auto const& frag : *v1720) {
    ds50::V172xFragment overlay(frag);
    for (size_t ch_num=0; ch_num<overlay.enabled_channels(); ++ch_num) {
      // ch_num is the number of the channel on the board, NOT the global channel ID.
      // The global channel ID is given by:
      channelIDs.push_back(overlay.board_id()*overlay.enabled_channels() + ch_num);
      
      wfmVec->emplace_back(std::vector<double>());
      auto & wfm = wfmVec->back();

      // Get the waveform for this channel on this board.
      ds50::V172xFragment::adc_type const * begin = overlay.chDataBegin(ch_num);
      ds50::V172xFragment::adc_type const * end   = overlay.chDataEnd(ch_num);
      for ( ; begin != end; ++begin)
        wfm.push_back(*begin);
      
    }//end loop over channels
  }//end loop over fragments


  
  /////////////////////////////////////////////////////////////////
  // Loop through the waveforms to overlay the MC results.
  // Note: the loop is NOT necessarily in channel ID order, and the
  // order is not necessarily the same from event to event.

  for (size_t i=0; i<wfmVec->size(); ++i) {
    int channel_id = channelIDs[i];
    std::vector<double> & wfm = (*wfmVec)[i];
    
    // Overlay MC results.

    // Do something stupid so this example code actually builds.
    std::cout << "The waveform for channel "<<channel_id<<" has "<<wfm.size()<<" samples."<<std::endl;


  }
  
  
  // Put the completed products in the Event.
  e.put(std::move(wfmVec));
}

DEFINE_ART_MODULE(darkart::mcWaveform)
