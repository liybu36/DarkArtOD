// 2014-11-07 AFan
// 
// Source module to read OD raw data. 
//
// Based on R. Kutschke's Source00 example source module.
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <boost/utility.hpp>


#include "art/Framework/IO/Sources/Source.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Sources/SourceHelper.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/IO/Sources/put_product_in_principal.h"
#include "art/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "darkart/ODReco/Tools/ODRawdataFile_t.hh"
#include "darkart/ODReco/Tools/ODRawdataDat_t.hh"
#include "darkart/ODReco/Tools/ODRawdataBzip2_t.hh"

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "darkart/ODReco/Tools/Utilities.hh"

#include "darkart/ODProducts/RunInfo.hh"
#include "darkart/ODProducts/ODEventInfo.hh"

#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"

#define OD_DEF_FILE_VERSION_SIZE 4
#define OD_MAX_GLOBAL_CHANNEL 256

namespace darkart {

  class ODReaderDetail : private boost::noncopyable {

  public:

    // The signatures and return types of the c'tor and the three member functions
    // are prescribed by the art::Source class template.
    ODReaderDetail(const fhicl::ParameterSet&,
                         art::ProductRegistryHelper&,
                   const art::SourceHelper&);

    void readFile(std::string const& filename,
                  art::FileBlock*& fb);

    bool readNext(art::RunPrincipal*    const& inR,
                  art::SubRunPrincipal* const& inSR,
                  art::RunPrincipal*&          outR,
                  art::SubRunPrincipal*&       outSR,
                  art::EventPrincipal*&        outE);

    void closeCurrentFile();

  private:

    // Control the level of printout.
    int _verbosity;

    // The data products will appear in the art::Event with the following module label:
    std::string _myModuleLabel;

    // Helper object needed by the member function managePrincipals.
    art::SourceHelper _pm;

    // Used to identify boundaries between runs and subruns.
    art::SubRunID _lastSubRunID;

    // The currently active file.
    std::string _currentFileName;

    // A helper function used to manage the principals.
    // This is boilerplate that does not change if you change the data products.
    void managePrincipals ( int runNumber,
                            int subRunNumber,
                            int eventNumber,
                            art::RunPrincipal*&    outR,
                            art::SubRunPrincipal*& outSR,
                            art::EventPrincipal*&  outE);

    int openFile(const std::string& filename);
    int getSubRunNumber(const std::string& filename);

    // Access the Utilities service
    art::ServiceHandle<darkart::od::Utilities> _utils;

    const std::string composeFileNameSubRun(const std::string& filename, const int runpart);

    // RunInfo to store informations about the run
    darkart::od::RunInfo _run_info;
 
    // Helper OD reader object
    ODRawdataFile_t* _rawdata_file;
    char _version[OD_DEF_FILE_VERSION_SIZE];
    unsigned long _u4_run_number;
    int _parallel_process;
    int _chassis_shift;
    std::vector<int> _disabled_channels;
    std::vector<double> _conversion_gain;
    std::vector<double> _conversion_offset;

    // away from bus 2 correction: 10 ns correction for sone digitizers
    bool _away_bus_correction;

    // bad time alignment and dt variables
    bool _first_event;
    double _past_abs_t[64];
    double _ev_dt[64];
    double _last_gps_tstamp_sec;

    // additional pulse boundaries
    double _min_start_time_ns;
    double _max_start_time_ns;

    // Other useful parameters specified by user
    int _starting_event;
    int _print_event_id; // 0 to not print, other print every n event ids

  }; //ODReaderDetail class definition

  ODReaderDetail::ODReaderDetail(const fhicl::ParameterSet&        pset,
                                       art::ProductRegistryHelper& rh,
                                 const art::SourceHelper&        pm)
    : _verbosity( pset.get<int>("detail.verbosity", 0) )
    , _myModuleLabel("ODReader")
    , _pm(pm)
    , _lastSubRunID()
    , _currentFileName()
    , _run_info()
    , _rawdata_file(NULL)
    , _u4_run_number(0)
    , _parallel_process  ( pset.get<int>("detail.parallel_process", 0) )
    , _chassis_shift     ( pset.get<int>("detail.chassis_shift", 0) )
    , _disabled_channels ( pset.get<std::vector<int> >("detail.disabled_channels", {}) )
    , _conversion_gain({})
    , _conversion_offset({})
    , _away_bus_correction ( pset.get<int>("detail.away_bus_correction", 1) )
    , _first_event(true)
    , _last_gps_tstamp_sec(0)
    , _min_start_time_ns ( pset.get<double>("detail.min_start_time_ns", -1.) ) 
    , _max_start_time_ns ( pset.get<double>("detail.max_start_time_ns", -1.) )
    , _starting_event    ( pset.get<int>("detail.starting_event"   , 0  ) )
    , _print_event_id    ( pset.get<int>("detail.print_event_id"   , 0  ) )
  {
    std::cout << "TEST" << std::endl;
    // This is the analog of the call to produces<T> in a producer module.
    // Same pattern is used for art::InRun and art::InSubRun objects.
    rh.reconstitutes<int,art::InEvent>(_myModuleLabel);
    rh.reconstitutes<darkart::od::ODEventInfo, art::InEvent>(_myModuleLabel);
    rh.reconstitutes<darkart::od::ChannelDataVec, art::InEvent>(_myModuleLabel);
    rh.reconstitutes<darkart::od::ChannelWFsVec, art::InEvent>(_myModuleLabel);
    rh.reconstitutes<darkart::od::RunInfo, art::InRun>(_myModuleLabel);

    if ( _verbosity > 0 ){ std::cout << "ODReaderDetail: constructor" << std::endl; }
  }

  //----------------------------------------------------------------
  void ODReaderDetail::readFile(const std::string& filename, art::FileBlock*& fb) {

    if ( _verbosity > 1 ){
      LOG_INFO("ODReader") << "ODReaderDetail: open file " << filename;
    }

    /////////////////////////////////////////////////////////////
    //Declare the RunInfo data product
    //darkart::od::RunInfo runInfo;
    _run_info.chHCD.resize(OD_MAX_GLOBAL_CHANNEL);

    _currentFileName = filename;
   
    // Open the file where the header is;
    // The header is always in the _001  file
    const int open_status = openFile( composeFileNameSubRun(filename, 1) );

    if (open_status == -1)
      throw cet::exception("ODReader") << _rawdata_file->get_error_description() << std::endl; 

    // Read header information.

    // Read bits from the header
    const int filetype_size = 4; 
    char filetype[filetype_size];
    for(int i=0; i<filetype_size; i++)
      filetype[i] = _rawdata_file->read_u8(1);

    if (filetype[0]!='D' || filetype[1]!='S' || filetype[2]!='O' || filetype[3]!='D')
      throw cet::exception("ODReader") << "FATAL: Raw file is not a DSOD file" << std::endl;
    
    for (int i=0; i<OD_DEF_FILE_VERSION_SIZE; i++)
      _version[i] = _rawdata_file->read_u8(1);

    LOG_INFO("ODReader") << "INFO: DSOD file version "
	                 << _version[0] + 0 << "." << _version[1] + 0 << "." << _version[2] + 0 << "." << _version[3] + 0;
    
    // starting from header version 2, run number is in the header
    if (_version[1]>=2){
      const int rn = _rawdata_file->read_i32(1);
      _u4_run_number = (unsigned) rn;
      if ( _verbosity > 1 ) 
        LOG_INFO("ODReader") << "INFO: Run Number of this raw file is " << rn;
      _run_info.run_id = rn;
    }
    else{
      throw cet::exception("DAQVersionTooOld") << "Version is lower than 2" << std::endl;
    }

    // Volt = _conversion_offset + _convesion_gain * Raw
    // these values will be filled in the loop
    _conversion_gain.assign(OD_MAX_GLOBAL_CHANNEL,0.);
    _conversion_offset.assign(OD_MAX_GLOBAL_CHANNEL,0.);

    const int n_chassis = _rawdata_file->read_i32(1);
    _run_info.n_chassis = n_chassis;

    int n_scopes = 0;
    // in header version 1, n_scopes (number of NI-Scopes per chassis) is constant in all chassis
    // and written once, just after n_chassis
    if ( _version[1]==1 ){
      n_scopes = _rawdata_file->read_i32(1);
    }

    // continue reading header: chassis level
    // if chassis_shift (e.g. if running with WT alone), 
    // then fill empty informations about unused chassis
    if (_chassis_shift) 
      LOG_INFO("ODReader") << "INFO: Chassis shift " << _chassis_shift;

    for(int ics=_chassis_shift; ics < (n_chassis + _chassis_shift); ics++){ 
      if ( _verbosity > 2 ){
        LOG_INFO("ODReader") << "INFO: Reading configuration of VETO PXIe Chassis "<< ics << " of " << n_chassis;
      }
      // starting from header version 2, the number of enabled scopes can be different in each chassis;
      // it is written for each chassis, as a u8 
      // after it, the maximum number of enabled channel through in the scope is written, as u8
      // these two numbers are actually never used ...
      // after it, the size of array of scope_configuration is written, as i32 

      if ( _version[1] >= 2 ){
        const int n_en_scopes = _rawdata_file->read_u8(1) + 0;
        const int max_en_ch = _rawdata_file->read_u8(1) + 0;
	n_scopes = _rawdata_file->read_i32(1);
	if ( _verbosity > 2 ){
          LOG_INFO("ODReader") << "INFO: Number of enabled NI-Scopes in this Chassis: " << n_en_scopes;
	  LOG_INFO("ODReader") << "INFO: Max number of enabled channels in this Chassis: " << max_en_ch;
        }
      }

      if ( _verbosity > 2 ){
        LOG_INFO("ODReader") << "INFO: Number of NI-Scopes in this Chassis: " << n_scopes;
      }
      _run_info.n_scopes[ics] = n_scopes;

      // loop on the NI-Scopes within the chassis
      for(int ibd=0; ibd<n_scopes; ibd++){

        const std::string device_name(_rawdata_file->read_str());
        const unsigned short num_enabled_chs = _rawdata_file->read_u8(1);

	const int enabled_chs_size = _rawdata_file->read_i32(1);
	unsigned char* scope_enabled_chs = new unsigned char[enabled_chs_size];
	_rawdata_file->read_u8(scope_enabled_chs, enabled_chs_size);

       const std::string enabled_ch_string (_rawdata_file->read_str());
       const double samplerate = _rawdata_file->read_dbl(1);
       const int recordlength = _rawdata_file->read_i32(1);
       const double referencepos = _rawdata_file->read_dbl(1)/100.;
       const int fetchnumrecords = _rawdata_file->read_i32(1);
       const int maxnumrecords = _rawdata_file->read_i32(1);

       if (_verbosity > 2){
         LOG_INFO("ODReader") << "INFO: Reading configuration of NI-Scope " << ibd << " of " << n_scopes << std::endl;

	 LOG_INFO("ODReader") << "INFO: NI-Scope Device Name: " << device_name 
	                      << "\nINFO: NI-Scope number of enabled channels: " << num_enabled_chs
	  	              << "\nINFO: NI-Scope enabled channels: " << enabled_ch_string
	 		      << "\nINFO: NI-Scope sample rate: " << samplerate << " Samples/s"
			      << "\nINFO: NI-Scope acquisition length: " << recordlength << " samples"
			      << "\nINFO: NI-Scope trigger reference position: " << referencepos
			      << "\nINFO: NI-Scope packaging of records: " << fetchnumrecords
			      << "\nINFO: NI-Scope max # of records to acquire: " << maxnumrecords;
       }

       // loop on channels within the NI-Scopes (almost always 4)
       const int chs_size = _rawdata_file->read_i32(1);
       for(int ich=0; ich<chs_size; ich++){
	 //	 const int gc = darkart::od::getGlobalChannelID(ics, ibd, ich);
	 const int gc = _utils->getGlobalChannelID(ics, ibd, ich);
	 
	 // look if the channel has to be disabled in the reconstruction: from fcl input
         const bool sw_enabled = (std::find(_disabled_channels.begin(), _disabled_channels.end(), gc) == _disabled_channels.end() ); 
	  
         const bool hw_enabled  = _rawdata_file->read_bool(1);
         const double vrange    = _rawdata_file->read_dbl(1);
         const double voffset   = _rawdata_file->read_dbl(1);
	 const double coupling  = _rawdata_file->read_i32(1);
         const double impedance = _rawdata_file->read_u32(1);
         const double bandwidth = _rawdata_file->read_dbl(1);

	 // zero suppression configuration
         const bool   zs_enabled = _rawdata_file->read_bool(1);
         const double zs_threshold = _rawdata_file->read_dbl(1);
         const double zs_minwidth = _rawdata_file->read_dbl(1);
         const double zs_pre = _rawdata_file->read_dbl(1);
         const double zs_post = _rawdata_file->read_dbl(1);

	 // gc from 0 to 127 belog to LSV (channel_type = 1);
	 // gc from 128 to 255 belong to WT (channel_type = 2)  
	 const int detector = gc<=127 ? 1 : 2;
	 // if the channel is disable channel_type = -(channel_type): -1: disabled LSV, -2: disable WT
	 const int channel_type = (hw_enabled && sw_enabled) ? detector : -detector;
	 
	 // filling informations in RunInfo
         _run_info.chHCD.at(gc).channel_type = channel_type;
         _run_info.chHCD.at(gc).sample_rate = samplerate;
         _run_info.chHCD.at(gc).record_length = recordlength;
         _run_info.chHCD.at(gc).reference_pos = referencepos;
         _run_info.chHCD.at(gc).is_enabled = (hw_enabled && sw_enabled);
         _run_info.chHCD.at(gc).vertical_range = vrange;
         _run_info.chHCD.at(gc).vertical_offset = voffset;
         _run_info.chHCD.at(gc).zs_enabled = zs_enabled;
         _run_info.chHCD.at(gc).zs_threshold = zs_threshold;
         _run_info.chHCD.at(gc).zs_minwidth = zs_minwidth;
         _run_info.chHCD.at(gc).zs_pre = zs_pre;
         _run_info.chHCD.at(gc).zs_post = zs_post;

	 // conversion_gain and conversion_offset are the constants used to convert the waveform from i16 to Volt:
	 // volt = conversion_offset + i16 * conversion_gain 
	 _conversion_gain.at(gc) = _rawdata_file->read_dbl(1);
	 _conversion_offset.at(gc) = _rawdata_file->read_dbl(1);

	 // put here the code to store the channel info (once the cointainer/class is provided)

	 if ( _verbosity > 2 ){
           LOG_INFO("ODReader") << "INFO: Reading configuration of Channel "<< ich << " of " << chs_size;

	   LOG_INFO("ODReader") << "INFO: Channel enabled (HW): " << (int) hw_enabled
				<< "\nINFO: Channel enabled (SW): " << (int) sw_enabled 
				<< "\nINFO: Channel vertical range: " << vrange << " Volt"
				<< "\nINFO: Channel offset of the vertical range: " << voffset << " Volt"
				<< "\nINFO: Channel coupling: " << coupling  		// TODO: write the enumeration
				<< "\nINFO: Channel impedance: " << impedance   	// TODO: write the enumeration
				<< "\nINFO: Channel bandwidth: " << bandwidth << " Hz";

	   LOG_INFO("ODReader") << "INFO: Zero Suppression enabled: " << (int) zs_enabled
				<< "\nINFO: Zero Suppression threshold: " << zs_threshold << " Volt"
				<< "\nINFO: Zero Suppression minimum width: " << zs_minwidth << " ns"
				<< "\nINFO: Zero Suppression pre-saving: " << zs_pre << " ns"
				<< "\nINFO: Zero Suppression post-saving: " << zs_post << " ns";

	   LOG_INFO("ODReader") << "INFO: Channel conversion gain: " << _conversion_gain.at(gc) << " Volt/raw"
				<< "\nINFO: Channel conversion offset: " << _conversion_offset.at(gc) << " Volt";
	 }

       } // end loop on channels within NI-Scopes (chs_size)

       delete[] scope_enabled_chs;

      } // end loop on NI-Scopes within PXIe Chassis (n_scopes)

    } // end loop on PXIe Chassis (n_chassis)

    // Check that different detectors have the same samplerate, acquisition gate, reference position and fill RunInfo DetectorCfg for both LSV and WT
    // TODO: retrieve reference channel for checking that all the LSV and all the WT have the same configuration from DB or fcl
    const double lsv_sample_rate = _run_info.chHCD.at(0).sample_rate;
    const double wt_sample_rate  = _run_info.chHCD.at(OD_MAX_GLOBAL_CHANNEL/2).sample_rate;
    const int lsv_record_length  = _run_info.chHCD.at(0).record_length;
    const int wt_record_length   = _run_info.chHCD.at(OD_MAX_GLOBAL_CHANNEL/2).record_length;    
    const double lsv_reference_pos = _run_info.chHCD.at(0).reference_pos;
    const double wt_reference_pos  = _run_info.chHCD.at(OD_MAX_GLOBAL_CHANNEL/2).reference_pos;
    const double lsv_zs_presamples = _run_info.chHCD.at(0).zs_pre;
    const double wt_zs_presamples  = _run_info.chHCD.at(OD_MAX_GLOBAL_CHANNEL/2).zs_pre;
    const double lsv_zs_postsamples= _run_info.chHCD.at(0).zs_post;
    const double wt_zs_postsamples = _run_info.chHCD.at(OD_MAX_GLOBAL_CHANNEL/2).zs_post;
    bool lsv_same_configuration = true;
    bool wt_same_configuration  = true;

    // loop on global channel to find if all channels of LSV (WT) have the same daq configuration
    for (int igc=1; igc<OD_MAX_GLOBAL_CHANNEL; igc++){
      if ( _run_info.chHCD.at(igc).is_enabled ){
        // check global LSV configuration
        if ( _run_info.chHCD.at(igc).channel_type == 1 ){ 
         if ( _run_info.chHCD.at(igc).sample_rate == lsv_sample_rate &&
              _run_info.chHCD.at(igc).record_length == lsv_record_length &&
              _run_info.chHCD.at(igc).reference_pos == lsv_reference_pos ) lsv_same_configuration = true;
         else lsv_same_configuration = false; 
        }
        // check global WT configuration
        if ( _run_info.chHCD.at(igc).channel_type == 2 ){ 
         if ( _run_info.chHCD.at(igc).sample_rate == wt_sample_rate &&
              _run_info.chHCD.at(igc).record_length == wt_record_length &&
              _run_info.chHCD.at(igc).reference_pos == wt_reference_pos ) wt_same_configuration = true;
         else wt_same_configuration = false; 
        }
      }
    }
    // filling RunInfo::DetectorCfg for both LSV and WT 
    _run_info.LSVCfg.same_configuration = lsv_same_configuration;
    _run_info.WTCfg.same_configuration  = wt_same_configuration;
    if ( _run_info.LSVCfg.same_configuration ){
      _run_info.LSVCfg.sample_rate_Hz = lsv_sample_rate;
      _run_info.LSVCfg.record_length  = lsv_record_length;
      _run_info.LSVCfg.reference_pos  = lsv_reference_pos;

      // Set the values in Utilities
      _utils->setLSVSampleRateHz(lsv_sample_rate);
      _utils->setLSVRecordLength(lsv_record_length);
      _utils->setLSVReferencePos(lsv_reference_pos);
      _utils->setLSVZSPresamples(lsv_zs_presamples);
      _utils->setLSVZSPostsamples(lsv_zs_postsamples);
    }
    if ( _run_info.WTCfg.same_configuration ){
      _run_info.WTCfg.sample_rate_Hz = wt_sample_rate;
      _run_info.WTCfg.record_length  = wt_record_length;
      _run_info.WTCfg.reference_pos  = wt_reference_pos;

      // Set the values in Utilities
      _utils->setWTSampleRateHz(wt_sample_rate);
      _utils->setWTRecordLength(wt_record_length);
      _utils->setWTReferencePos(wt_reference_pos);
      _utils->setWTZSPresamples(wt_zs_presamples);
      _utils->setWTZSPostsamples(wt_zs_postsamples);
    }

    // check if the filename to process is the _001 (where the header is)
    // if the subrun file is different from 001 close the 001 subrun file and open the file to be processed
    const int subrun = getSubRunNumber(filename);
    if ( subrun != 1 ){
      // the current _rawdata_file points to the 001 subrun file
      _rawdata_file->close();
      LOG_INFO("ODReader") << "INFO: Closed header subrun file";

      // open the file to be processed
      const int openfile_status = openFile(filename);
      // now _rawdata_file points to filename (the subRun to process)
      if ( openfile_status == -1 )  
        throw cet::exception("ODReader") << "FATAL: cannot open " << filename << std::endl;
    }

    // Evaluate end of file
    if ( _rawdata_file->eof() ){
      LOG_INFO("ODReader") << "INFO: End of file " << filename; 
      // TODO: tell art that file is closed? this is a run with 0 events; are we processing these runs?
    }

    // A FileBlock is the object art uses to maintain state information about the file.
    // art takes ownership of this pointer and will call delete on it at the appropriate time.
    // You should not need to modify this except maybe to step forward the file format version
    // when necessary.
    fb = new art::FileBlock(art::FileFormatVersion(1, "ODReader"), _currentFileName);

  } //readFile()

  void ODReaderDetail::closeCurrentFile() {
    LOG_INFO("ODReader") << "ODReader: close file " << _currentFileName;

    // DO NOT set _currentFileNddame = "";
    _rawdata_file->close();
  } //closeCurrentFile()

  bool ODReaderDetail::readNext(art::RunPrincipal*    const& run_principal,
		  art::SubRunPrincipal* const& subrun_principal,
		  art::RunPrincipal*&          outR,
		  art::SubRunPrincipal*&       outSR,
		  art::EventPrincipal*&        outE)
  {
  
    // Read an event. The run ID, subrun ID, and event ID are
    // set here. You can insert multiple objects into the
    // art::Event -- call art::put_product_in_principal() for
    // each object.

  
    const int runNumber = _u4_run_number;
    int subRunNumber = 1; //TODO: 0? use getSubRunNumber?


    // read next event number if end of file is not reached;
    // if eof reached then comunicate eof to Art
    int eventNumber =  _rawdata_file->read_i32(1) + 1;

    // Evaluate end of file
    // Try to open next subrun if sequential reading
    if ( _rawdata_file->eof() ){
      LOG_INFO("ODReader") << "INFO: End of file " << _currentFileName << " reached ";
      
      // close current file
      closeCurrentFile();
      // if sequential processing (_parallel_process == 0) try to open the next subrun file
      if ( _parallel_process == 0 ){
        const int next_subrun = (getSubRunNumber(_currentFileName)) + 1;
	const std::string next_file_name = (composeFileNameSubRun(_currentFileName, next_subrun));
	const int next_file_status = openFile(next_file_name);
	if ( next_file_status < 0 ){ // next subrun file does not exists; we reached end of run
     	  LOG_INFO("ODReader") << "INFO: No new subrun file to open " << next_file_name; 
	  return false;
	}
	else { // new subrun does exist; read the eventNumber
	  _currentFileName = std::move(next_file_name);
	  eventNumber = _rawdata_file->read_i32(1) + 1;
	}
      } // _parallel_process == 0
      else{ // parallel_process != 0 : do not open other files
        return false;
      }

    } // raw data eof()

    // return value tells art what to do after this event
    // return false says to kill this event (and look for the next file given as input).

    // TODO: handle multiple files a-la-Art - we are probably re-inventing the wheel
    // Tell art that we have reached of this file; there may be more files to come.
    

    int magic = 3;
    std::unique_ptr<darkart::od::ODEventInfo> ei(new darkart::od::ODEventInfo);
    ei->run_id = runNumber;
    ei->subrun_id = subRunNumber;
    ei->event_id = eventNumber;

    if(_print_event_id > 0 && (eventNumber % _print_event_id) == 0)
      std::cout << "Start event " << eventNumber << std::endl;

    std::unique_ptr<darkart::od::ChannelDataVec> ch_data_Vec(new darkart::od::ChannelDataVec); //ch_data - channel raw data
    std::unique_ptr<darkart::od::ChannelWFsVec> ch_wf_Vec(new darkart::od::ChannelWFsVec); //ch_wf - channel waveform

    // resize ch_data_Vec and ch_wf_Vec to the maximum number of OD channels (256);
    // this way, we can later fill the ch_data_Vec "slot" corrisponding to each GlobalChannel
    // the "gaps" in ch_data_Vec are usually due to unplugged channels in NI-Scopes or missing NI-Scopes in NI-PXIe chassis 
    ch_data_Vec->resize(OD_MAX_GLOBAL_CHANNEL);   
    ch_wf_Vec->resize(OD_MAX_GLOBAL_CHANNEL);   

    if (_verbosity > 1)
      LOG_INFO("ODReader") << "Run: " << runNumber << "\tSubRun: " << subRunNumber << "\tEvent: " << eventNumber << "\tMagic: " << magic;

    ei->trigger_type = _rawdata_file->read_u16(1);
    ei->gps_pps = (_version[2]>=2) ? (_rawdata_file->read_u16(1)) : 0;
    ei->gps_fine = _rawdata_file->read_u32(1);
    ei->gps_coarse = _rawdata_file->read_u32(1);
    ei->gps_timestamp_sec = ei->makeGPSTimestamp(ei->gps_pps, ei->gps_fine);


    if (_verbosity > 2)
      LOG_INFO("ODReader") << "TRG ID: "<< ei->trigger_type
                           << "\t1PPS: " << ei->gps_pps
                           << "\tFine Count: " << ei->gps_fine
                           << "\tOne Sec: " << ei->gps_coarse
                           << "\tTimestamp: " << ei->gps_timestamp_sec; 

    // loop on PXIe chassis
    const int n_chassis = _rawdata_file->read_i32(1);
    for(int ics=_chassis_shift; ics < n_chassis + _chassis_shift; ics++){

      bool first_board = true; // used for bad time alignment check
      double last_ev_dt = 0.; // used for bad time alignment check

      const unsigned CHASSIS_ID = _rawdata_file->read_u8(1);
      if (_verbosity > 2)
        LOG_INFO("ODReader") << "Reading PXIe Chassis " << CHASSIS_ID << " of " << n_chassis;

      // loop on NI-Scopes
      const int n_scopes = _rawdata_file->read_i32(1);
      for(int ibd=0; ibd<n_scopes; ibd++){
        const unsigned SCOPE_ID = _rawdata_file->read_u8(1);
	if ( _verbosity > 2 )
          LOG_INFO("ODReader") << "Reading NI-Scope " << SCOPE_ID << " of " << n_scopes;
	      
	// global NI-Scope number
	const int gs = _utils->getGlobalScopeID(ics, ibd);

	const double absolute_t = _rawdata_file->read_dbl(1);
	const double relative_t = _rawdata_file->read_dbl(1);

	if ( _verbosity > 2 )
	  LOG_INFO("ODReader") << "Event absolute_t: " << absolute_t << "\nEvent relative_t: " << relative_t;

	// check bad time alignment
	if (!_first_event){
	  _ev_dt[gs] = _past_abs_t[gs] - absolute_t;

	  if (!first_board){ // The check is performed comparing times from the 0-th NI-Scope, so skip the 0-th 
            const double scope_dt = _ev_dt[gs] - last_ev_dt;
	    if ( scope_dt > 1.e-6 ){
	      ei->bad_time_alignment = 1;
	      if (_verbosity > 1)
	        LOG_INFO("ODReader") << "ERROR: Misalignement in absolute_t at event " << ei->event_id
                                     << " - PXIe Chassis "<< ics << " NI-Scope " << ibd << " : " << scope_dt;
	    }
	    else ei->bad_time_alignment = 0;
	  }

	  last_ev_dt = _ev_dt[gs];
	} // end (!first_event)
	else ei->bad_time_alignment = 0;

	_past_abs_t[gs] = absolute_t;
	first_board = false;
	// end check bad time alignment
	

	// loop on channels
	const int n_channels = _rawdata_file->read_i32(1);
	for(int ich=0; ich<n_channels; ich++){

	  const unsigned CHANNEL_ID = _rawdata_file->read_u8(1);
	  if ( _verbosity > 2 )
            LOG_INFO("ODReader") << "Reading Channel " << CHANNEL_ID << " of " << n_channels;

	  darkart::od::ChannelData ch_data;
	  darkart::od::ChannelWFs ch_wf;
	  ch_data.chassis = ics;
	  ch_data.board = ibd;
	  ch_data.channel = ich;
	  const int gc = _utils->getGlobalChannelID(ics, ibd, ich);
	  ch_data.channel_id = gc;
	  ch_data.channel_type = _run_info.chHCD.at(gc).channel_type; // retrieve channel_type from RunInfo
	  ch_wf.channel_id = gc;

	  // Away from bus2 correction: 10 ns (12 samples @ 1.25 GSamples/s) for NI-Scopes away from bus 2. See manual NI PXIe-1075
	  const double samplerate_Hz = _run_info.chHCD.at(gc).sample_rate; 
	  const int bus2_corr_samples = (_away_bus_correction) ? ( ((ibd<=4) || (ibd>=10) ) ? std::floor(10.*samplerate_Hz*1.e-9) : 0) : 0;

	  // loop on pulses (zero-suppressed)
	  const int n_pulses = _rawdata_file->read_i32(1);
	  ch_data.npulses = n_pulses;
	  for(int ip=0; ip<n_pulses; ip++){
	    if ( _verbosity > 2 )
	      LOG_INFO("ODReader") << "Reading pulse " << ip << " of " << n_pulses;
	    
	    const int pulse_number = _rawdata_file->read_i32(1);
	    const int pulse_start_sample = _rawdata_file->read_i32(1) + bus2_corr_samples;
	    const int record_length = _run_info.chHCD.at(gc).record_length;
	    const bool start_sample_in_record_length = (pulse_start_sample < record_length); 
	    
	    _rawdata_file->read_i32(1); // spare word (for now it just repeats the size)
	    
	    // reading raw pulse waveform
	    int pulse_size = _rawdata_file->read_i32(1);
	    short* pulse_wf_raw = new short[pulse_size];
	    _rawdata_file->read_i16(pulse_wf_raw, pulse_size);
	    
	    if ( _verbosity > 2 )
	      LOG_INFO("ODReader") << "Pulse number:  " << pulse_number << "\nPulse start sample: " << pulse_start_sample << "\nPulse size: " << pulse_size << " samples";
	    
	    bool skip_pulse = false;
	    
	    const bool ch_enabled = _run_info.chHCD.at(gc).is_enabled;
	    if ( ch_enabled == false ){
	      if ( _verbosity > 2 )
		LOG_INFO("ODReader") << "Skipping pulse because channel " << gc << " is disabled" << std::endl;
	      skip_pulse = true;
	    }
	    
	    if ( !start_sample_in_record_length ){
	      if ( _verbosity > 2 )
		LOG_INFO("ODReader") << "Skipping pulse " << pulse_number << " because out of record length";
	      skip_pulse = true;
	    }
	    
	    // start_time_ns is with respect of the trigger. Trigger time is 0 
	    const double reference_pos = _run_info.chHCD.at(gc).reference_pos;
	    const double start_time_ns = (pulse_start_sample - reference_pos * record_length)/(samplerate_Hz*1.e-9);
	    // remove pulses excluded by _min_start_time and _max_start_time
	    if ( _min_start_time_ns != _max_start_time_ns ){
	      if ( (start_time_ns < _min_start_time_ns) || (start_time_ns > _max_start_time_ns) ){
		if ( _verbosity > 2 )
		  LOG_INFO("ODReader") << "Skipping pulse " << pulse_number << " because out of additional pulse boundaries";
		skip_pulse = true;
	      }
	    }
	    
	    if (skip_pulse) { // the pulse is not stored and not passed to next modules
	      --ch_data.npulses;
	      delete [] pulse_wf_raw;
	      continue;
	    }
	    
	    // adjust pulses whose end falls after the boundaries of recordlength due to bus 2 corr
	    const int segfault = (pulse_start_sample + pulse_size - record_length );
	    if ( segfault > 0 ){
	      pulse_size -= segfault;
	    }
	    
	    // filling darkart::od::ChannelData::Pulse informations
	    darkart::od::ChannelData::Pulse pulse;
	    pulse.pulse_id = pulse_number;
	    pulse.start_sample = pulse_start_sample;
	    pulse.start_ns = start_time_ns;
	    pulse.size_sample = pulse_size;
	    
	    darkart::od::ChannelWFs::PulseWF pulse_wf;
	    //WARNING: this is a very minimal filling of pulse and pulseWF -> this needs review
	    pulse_wf.pulse_id = pulse_number;
	    pulse_wf.data.resize(pulse_size);
	    pulse_wf.start_ns = start_time_ns;
	    
	    // loop on the raw waveform and convert it in Volt using conversion_gain and conversion_offset: Volt = conversion_offset + raw*conversion_gain
	    // compute also integral of the pulse and evaluate the minimum of the wf (keep in mind pmt signal is negative)
	    double integral = 0;
	    int peak_sample = 0;
	    float peak_amplitude_min = +5.; // max. digitizer vertical range
	    for (int iwf=0; iwf<pulse_size; iwf++){
	      const float volt_sample = pulse_wf_raw[iwf] * _conversion_gain.at(gc) + _conversion_offset.at(gc);
	      pulse_wf.data.at(iwf) = volt_sample;
	      integral += volt_sample;
	      if ( volt_sample < peak_amplitude_min ){
		peak_sample = iwf;
		peak_amplitude_min = volt_sample;
	      }
	    }
	    pulse.offset = (double) integral/ (double) pulse_size; // DO NOT put this line after integral /= samplerate
	    integral /=  samplerate_Hz;
	    pulse.integral = integral;
	    pulse.peak_sample = peak_sample;
	    pulse.peak_amplitude = peak_amplitude_min;
	    ch_data.pulses.push_back(std::move(pulse));
	    ch_wf.pulse_wfs.push_back(std::move(pulse_wf));
	    
	    delete[] pulse_wf_raw;
	    
	  }  // loop on pulses (n_pulses)
	  
	  if ( _verbosity > 1 ){
            LOG_INFO("ODReader") << ch_data.PrintVarNames();
            LOG_INFO("ODReader") << ch_data.PrintVarContent();
	    LOG_INFO("ODReader") << ch_wf.PrintVarNames();
	    LOG_INFO("ODReader") << ch_wf.PrintVarContent();
	  }
	  ch_data_Vec->at(gc) = std::move(ch_data);
	  ch_wf_Vec->at(gc) = std::move(ch_wf);

	} // end loop on channels within NI-Scopes (n_channels)

      } // end loop on NI-Scopes within PXIe chassis(n_scopes)

    } // end loop on PXIe chassis
	
    // compute dt_us
    double dt_usec = ((_first_event) || (ei->bad_time_alignment>0)) ? -1. : (ei->gps_timestamp_sec - _last_gps_tstamp_sec)*1.e6; 
    // TODO: if dt_usec == 0, use absolute_t to compute dt_usec
    ei->dt_usec = dt_usec;
    _last_gps_tstamp_sec = ei->gps_timestamp_sec;


    // Create an empty data product and then fill it with information read from the input file.
    std::unique_ptr<int> product(new int);
    *product = magic;

    // Create principals as needed. This is should not need to be modified.
    managePrincipals( runNumber, subRunNumber, eventNumber, outR, outSR, outE );

    // Create the RunInfo object from the source module's data member which was filled during readFile()
    if ( _first_event ){
      std::unique_ptr<darkart::od::RunInfo> ri(new darkart::od::RunInfo(_run_info));
      art::put_product_in_principal(std::move(ri), *outR, _myModuleLabel);
    }
          
    // This is the analog of event.put() in a producer module.
    art::put_product_in_principal(std::move(product), *outE, _myModuleLabel);
    art::put_product_in_principal(std::move(ei), *outE, _myModuleLabel);
    art::put_product_in_principal(std::move(ch_data_Vec), *outE, _myModuleLabel);
    art::put_product_in_principal(std::move(ch_wf_Vec), *outE, _myModuleLabel);

    _first_event = false;

    // code to skup events
    if(eventNumber < _starting_event){
      readNext(run_principal,
	       subrun_principal,
	       outR,
	       outSR,
	       outE);
    }

    // Tell art that this is NOT the end of file.
    return true;

  } // readNext()

  // Each time that we encounter a new run, a new subRun or a new event, we need to make a new principal
  // of the appropriate type.  This code does not need to change as the number and type of data products changes.
  void ODReaderDetail::managePrincipals ( int runNumber,
		  			  int subRunNumber,
		  			  int eventNumber,
		  			  art::RunPrincipal*&    outR,
					  art::SubRunPrincipal*& outSR,
					  art::EventPrincipal*&  outE)
  { 
  
    art::Timestamp ts;
    art::SubRunID newID(runNumber, subRunNumber);

    if ( newID.runID() != _lastSubRunID.runID() ){
      // art takes ownership of the object pointed to by outR and will delete it at the appropriate time.
      outR = _pm.makeRunPrincipal(runNumber, ts);
      if ( _verbosity > 1 ){
        std::cout << "ODReaderDetail: making run principal: "
 		  << newID.runID() << " "
		  << _lastSubRunID.runID() 
                  << std::endl;
      }
    }

    if ( newID != _lastSubRunID ){
      // art takes ownership of the object pointed to by outSR and will delete it at the appropriate time.
      outSR = _pm.makeSubRunPrincipal(runNumber, subRunNumber, ts);
      if ( _verbosity > 1 ){
        std::cout << "ODReaderDetail: making subRun principal: "
		  << newID << " "
		  << _lastSubRunID 
                  << std::endl;
      }
    }
    _lastSubRunID = newID;

    // art takes ownership of the object pointed to by outE and will delete it at the appropriate time.
    outE = _pm.makeEventPrincipal(runNumber, subRunNumber, eventNumber, ts, false);

  } // managePrincipals()

  // open file according to the file type
  int ODReaderDetail::openFile(const std::string& filename){
    // parsing filename to decide which reading method must be use: .dat or .bizp2
    if ( filename.find(".dat") != std::string::npos && filename.find(".bz2") == std::string::npos ){
      _rawdata_file = new ODRawdataDat_t();
    }
    else if ( filename.find(".bz2") != std::string::npos ){
      _rawdata_file = new ODRawdataBzip2_t();
    }
    else{
      throw cet::exception("ODReader") << "FATAL: unsupported file type " << filename << std::endl;
      return -2;
    }

    // open file
    _rawdata_file->open(filename.c_str());

    // check if the file is open
    if (!_rawdata_file->is_open()){
      return -1;
    }

    LOG_INFO("ODReader") << "INFO: Open file " << filename; 
    
    return 0;
  } // openFile()

  // retrieve subrun number from file name. This function consider that there is only 1 "_" and only 1 "." in the filename
  int ODReaderDetail::getSubRunNumber(const std::string& filename){
    const size_t underscore_pos = filename.find("_");
    const size_t dot_pos = filename.find(".");
    if ( underscore_pos != std::string::npos && dot_pos != std::string::npos && dot_pos > underscore_pos ){
      return atoi( filename.substr(underscore_pos+1,3).c_str() );
    }
    else return -1;
  } // getRunPar
  
  // edit file name making it with the subrun number passed to the function. This function consider that there is only 1 "_" and only 1 "." in the filename
  const std::string ODReaderDetail::composeFileNameSubRun(const std::string& filename, const int subrun){
    // subrun number must be between 1 and 999
    if (subrun<=0 || subrun>999){
      throw cet::exception("ODReader") << "FATAL: subrun number is not between 1 and 999: " << subrun << std::endl;
      return "";
    }
    
    // building filename for subrun
    const size_t underscore_pos = filename.find("_");
    const size_t dot_pos = filename.find(".");
    if ( underscore_pos != std::string::npos && dot_pos != std::string::npos && dot_pos > underscore_pos ){
      const std::string basepath = filename.substr(0,underscore_pos);
      const std::string filetype = filename.substr(dot_pos,filename.length()-dot_pos);
      std::ostringstream filenamesubrun;
      filenamesubrun << basepath << "_" << std::setw(3) << std::setfill('0') << subrun << filetype;
      return filenamesubrun.str().c_str();
    }
    else return "";
  } // composeFileNameSubRun

} // namespace darkart

typedef art::Source<darkart::ODReaderDetail> ODReader;
DEFINE_ART_INPUT_SOURCE(ODReader)
