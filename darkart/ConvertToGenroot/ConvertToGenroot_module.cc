////////////////////////////////////////////////////////////////////////
// Class:       ConvertToGenroot
// Module Type: analyzer
// File:        ConvertToGenroot_module.cc
//
// Generated at Fri Mar 15 13:45:26 2013 by Ben Loer using artmod
// from art v1_03_08.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/Selector.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

#include "artdaq-core/Data/Fragments.hh"

#include "darksidecore/Data/V172xFragment.hh"
#include "darksidecore/Data/V1495Fragment.hh"
#include "darksidecore/Data/Config.hh"

#include <fstream>
#include <time.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <sys/stat.h> //needed for mkdir
#include <zlib.h>
#include <stdexcept>

#include "GenrootRawEvent.h"

using ds50::V172xFragment;

//this is a terrible hack until the headers get updated
/*struct V172xFragmentmetadata {
  typedef uint32_t data_t;
  uint32_t serial_number;
  char roc_firmware[6], amc_firmware[6];
  uint8_t v17xx_model, sample_bits;
  uint16_t sample_rate_MHz;
  uint32_t record_length;
  uint16_t post_trigger;
  uint16_t unused[3];
  static size_t const size_words = 8ul;
};
*/
namespace darkart{
  class ConvertToGenroot;
}

class darkart::ConvertToGenroot : public art::EDAnalyzer {
public:
  explicit ConvertToGenroot(fhicl::ParameterSet const & p);
  virtual ~ConvertToGenroot();

  void analyze(art::Event const & e) override;

  void beginJob() override;
  void endJob() override;
  void beginRun(art::Run const & r) override;
  void endRun(art::Run const & r) override;
  
  //access functions
  
  /// Get the level of gzip compression being used
  int GetCompressionLevel(){ return _compression; }
  /// Check the status of the output file 
  bool IsOK(){ return _ok; }
  /// Get the total number of uncompressed bytes written so far
  long long GetBytesWritten(){ return _bytes_written; }
  
  std::string GetFilename() const { return _filename; }
  void SetFilename(const std::string& name){ _filename = name; }
  
  void SetRunID(uint32_t runid){ _ghead.run_id = runid; }
  uint32_t GetRunID(){ return _ghead.run_id; }

private:
  
  ///Save config file to go with raw data
  void SaveConfigFile();
  void OpenNewFile();
  void CloseCurrentFile();
  void Process(GenrootRawEvent* raw);
  
  uint8_t GetBoardID(artdaq::Fragment const & f)
  { return V172xFragment(f).board_id(); }
  
  std::vector<std::string> _v172x_data_tags; //which blocks to convert
  std::string _filename;
  std::string _directory;
  bool _create_directory;
  std::string _autonamebase;
  int _compression;
  std::ofstream _fout;
  std::ofstream _logout;
  void* _log_messenger;
  //gzFile _fout;
  bool _ok;
  long long _bytes_written;
  uint32_t _max_file_size;
  int _max_event_in_file;
  
  std::stringstream _config_info;
  
  // headers for version control
public:
  static const uint32_t magic_number = 0xdec0ded1; 
  static const uint32_t latest_global_version = 1;
  static const uint32_t latest_event_version = 1;
  struct global_header{
    uint32_t magic_num_check;
    uint32_t global_header_size;
    uint32_t global_header_version;
    uint32_t event_header_size; 
    uint32_t event_header_version;
    uint32_t file_size;
    uint32_t start_time;
    uint32_t end_time;
    uint32_t run_id;
    uint32_t file_index;
    uint32_t nevents;
    uint32_t event_id_min;
    uint32_t event_id_max;
    global_header() : magic_num_check(magic_number), 
		      global_header_size(sizeof(global_header)),
		      global_header_version(1),
		      event_header_size(sizeof(event_header)),
		      event_header_version(1) {}
    
  };
  struct event_header{
    uint32_t event_size; 
    uint32_t event_id;
    uint32_t timestamp;
    uint32_t nblocks;
    event_header() : event_size(sizeof(event_header)){}
    void reset(){ event_size=0; event_id=0; timestamp=0; nblocks=0; }
  };

  struct datablock_header{
    uint32_t total_blocksize_disk;
    uint32_t datasize;
    uint32_t type;
  };
  
  struct event_header_v0{
    uint32_t event_size;
    uint32_t event_id;
    uint32_t timestamp;
  };
  
private:
  global_header _ghead;
};

using darkart::ConvertToGenroot;

ConvertToGenroot::ConvertToGenroot(fhicl::ParameterSet const & p) :
  EDAnalyzer(p),
  _fout(), _ok(true), _bytes_written(0)
{
  _v172x_data_tags = p.get<std::vector<std::string> >("v172x_data_tags");
  _directory = p.get<std::string>("directory",".");
  _create_directory = p.get<bool>("create_directory", true);
  _compression = p.get<int>("compression",Z_BEST_SPEED);
  _max_file_size = p.get<uint32_t>("max_file_size",0x80000000);
  _max_event_in_file = p.get<int>("max_event_in_file",10000);
}

ConvertToGenroot::~ConvertToGenroot()
{
  // Clean up dynamic memory and other resources here.
}

void ConvertToGenroot::analyze(art::Event const & e)
{
  //build up the GenrootRawEvent
  //get handles to the V1720 and V1724 data blocks
  //there might not be a block from each board type in every event, 
  //so have to check for empties
  std::vector<art::Handle<artdaq::Fragments> > raw;
  for(size_t i=0; i<_v172x_data_tags.size(); ++i){
    art::Handle<artdaq::Fragments> tempHandle;
    if( e.getByLabel( _v172x_data_tags[i], tempHandle) )
      raw.push_back(tempHandle);
  }
  
  //if no events were found, nothing to do
  if(raw.empty()) return;
  
  //the timestamps for some stupid reason aren't in the run and event, so
  //we also have to read a V1495 block just to get them
  
  uint32_t timestamp = 0;
  art::Handle<artdaq::Fragments> v1495;
  if( e.getByLabel ("daq", "V1495", v1495) && v1495->size() == 1 &&
      (*v1495)[0].hasMetadata()){
    
    ds50::V1495Fragment::metadata const* md = 
      (*v1495)[0].metadata<ds50::V1495Fragment::metadata>();
    timestamp = md -> trigger_time.tv_sec;
    
    //no timestmaps in the run object either
    if(_ghead.start_time == 0)
      _ghead.start_time = timestamp;
    _ghead.end_time = timestamp;
  }
  
  //create the empty genroot event
  //should eventually get timestamp from event itself
  GenrootRawEvent genevt(e.event(),timestamp,e.run());
  
  //loop through once to get the total data size and config info
  bool fillconfig = _config_info.str().empty();
  if(fillconfig) _config_info<<"V172X_Params (\n";
  
  uint32_t totalsize = 0;
  for(size_t btype=0; btype < raw.size() ; ++btype){
    size_t nboards_thisevent = raw[btype]->size();
    for(size_t idx=0; idx<nboards_thisevent; ++idx){
      const auto& frag((*(raw[btype]))[idx]);
      totalsize += frag.dataSize() * sizeof(artdaq::RawDataType);
      if(fillconfig){
	//need board type, pre/post trigger time, board id
	//board type comes from the block we're in
	//trigger settings come from metadata
	_config_info<<"\tboard"<<(int)GetBoardID(frag)<<" ( "
		    <<" board_type "
		    << raw[btype].provenance()->productInstanceName();
	
	double pre_trigger=0;
		
	if(frag.hasMetadata()){
	  V172xFragment::metadata const* md = 
	    frag.metadata<V172xFragment::metadata>();
	  uint32_t trigger = md->record_length / md->sample_rate_MHz;
	  double post_trigger = trigger * 1.*md->post_trigger/100.;
	  pre_trigger = trigger - post_trigger;
	}
	_config_info<<" pre_trigger_time_us "<<pre_trigger
		    <<" )\n";
      } //end if writing config file
    }
  }
  
  if(fillconfig) _config_info<<")";
  
  size_t blocknum = genevt.AddDataBlock(GenrootRawEvent::CAEN_V172X, totalsize);
  unsigned char* rawbuf = genevt.GetRawDataBlock(blocknum);
  uint32_t offset = 0;
  //loop again to fill the new buffer (using words not bytes)
  for(size_t btype=0; btype < raw.size() ; ++btype){
    size_t nboards_thisevent = raw[btype]->size();
    for(size_t idx=0; idx<nboards_thisevent; ++idx){
      const auto& frag((*(raw[btype]))[idx]);
      std::copy(frag.dataBegin(), frag.dataEnd(),
		((artdaq::RawDataType*)(rawbuf))+offset);
      offset += frag.dataSize();
    }
  }
  //now write out the event
  Process(&genevt);
}

void ConvertToGenroot::beginJob()
{
  // Implementation of optional member function here.
}

void ConvertToGenroot::endJob()
{
  // Implementation of optional member function here.
}

void ConvertToGenroot::beginRun(art::Run const & r)
{
  //reset config info
  _config_info.str("");
  
  //set the filename to Run<runid>
  std::stringstream fname;
  if(_directory == "") _directory = ".";
  fname<<_directory<<"/"<<"Run"<<std::setw(6)<<std::setfill('0')
       <<r.run();
  _filename = fname.str();
  
  //make a directory before starting the file if requested
  if(_create_directory){
    std::string dirpart="", filepart=_filename;
    size_t slash = _filename.find_last_of('/');
    if(slash != std::string::npos){
      dirpart = _filename.substr(0,slash+1);
      filepart = _filename.substr(slash+1);
    }
    //filepart _should_ have suffixes removed...
    dirpart += filepart;
    std::cout<<"ConvertToGenroot: Attempting to create directory "<<dirpart<<std::endl;
    int err = mkdir(dirpart.c_str(),S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    if(err){
	std::cerr<<"ConvertToGenroot ERROR: Unable to create output directory "<<dirpart<<"\n";
	throw std::runtime_error("Unable to create directory");
    }
    _filename = dirpart + "/" + filepart;
  }
  
  //set up the file header
  _ghead.run_id = r.run();
  _ghead.file_index = 0;
  _ghead.start_time = r.beginTime().timeHigh();
  _ghead.end_time = r.endTime().timeHigh();
  //open a new file
  OpenNewFile(); //throws exception if problem
}

void ConvertToGenroot::endRun(art::Run const & r)
{
  // Implementation of optional member function here.
  if(_fout.is_open()){
    CloseCurrentFile();
    std::cout<<"ConvertToGenroot: "<<1.*_bytes_written/1024./1024.
	     <<" MiB saved to "<<_filename<<" for run "<<r.run()
	     <<std::endl;
  }
  if(_bytes_written > 0){
    SaveConfigFile();
  }
}

void ConvertToGenroot::OpenNewFile()
{
  if(_fout.is_open()){
    std::cerr<<"ConvertToGenroot ERROR: Tried to open new file while current file still open!\n";
    CloseCurrentFile();
  }
  
  //set the filename to filename.###.out, where ### is file_index
  std::stringstream fname;
  fname<<_filename<<"."<<std::setw(3)<<std::setfill('0')
       <<_ghead.file_index<<".out";
  std::cout<<"ConvertToGenroot: Opening file "<<fname.str()<<std::endl;
  _fout.open(fname.str().c_str(), std::ios::out|std::ios::binary);
  if(!_fout.is_open()){
    std::cerr<<"ConvertToGenroot ERROR: Unable to open file "<<fname.str()<<" for output!\n";
    _ok = false;
    throw std::runtime_error("Unable to open genroot file");
  }
  //write the "blank" global header 
  //reset nevents
  _ghead.nevents = 0;
  //set min event id to max value so we can set it properly during Process
  _ghead.event_id_min = 0xFFFFFFFF;
  //reset the file_size 
  _ghead.file_size = _ghead.global_header_size;
  
  if(!_fout.write((const char*)(&_ghead), _ghead.global_header_size)){
    std::cerr<<"ConvertToGenroot ERROR:writing header to file "<<fname<<"\n";
    throw std::runtime_error("Unable to write header to genroot file");
  }
  
}

void ConvertToGenroot::CloseCurrentFile()
{
  if(!_fout.is_open())
    return;
  //save the completed global header
  _fout.seekp(0);
  _fout.write((const char*)(&_ghead), _ghead.global_header_size);
  _fout.close();
  //increment the file_index counter
  _ghead.file_index++;
}

void ConvertToGenroot::SaveConfigFile()
{
  //make sure there's something to save
  if(_config_info.str().empty()) return;
  //strip the '.gz' off the end of the file
  std::string cfgfile(_filename);
  if(_filename.rfind(".gz") != std::string::npos)
    cfgfile.resize(cfgfile.size()-3);
  //strip off .out
  if(_filename.rfind(".out") != std::string::npos)
    cfgfile.resize(cfgfile.size()-4);
  cfgfile.append(".cfg");
  
  //ConfigHandler::GetInstance()->SaveToFile((cfgfile+".cfg").c_str());
  std::ofstream fout(cfgfile.c_str());
  if(!(fout<<_config_info.str())){
    std::cerr<<"ConvertToGenroot: ERROR saving config file "<<cfgfile<<"\n";
  }
}

void ConvertToGenroot::Process(GenrootRawEvent* raw)
{
   //compress all of the datablocks into a separate buffer
  //each block has compressed size, including header, uncompressed data size, 
  //and type as header
  //determine the total size of the output buffer
  uint32_t bufsize = sizeof(event_header);
  for(size_t i = 0; i<raw->GetNumDataBlocks(); i++){
    bufsize += compressBound(raw->GetDataBlockSize(i)) + 
      sizeof(datablock_header);
  }
  //zip the data into the buffer
  std::vector<char> buf(bufsize);
  size_t zipsize=sizeof(event_header);
  for(size_t i = 0;i<raw->GetNumDataBlocks(); i++){
    //write the data into a space after the header
    uLong thistransfer = bufsize-zipsize-sizeof(datablock_header);
    int err = 0;
    if(_compression == Z_DEFAULT_COMPRESSION){
      err = compress((Bytef*)(&buf[zipsize+sizeof(datablock_header)]),
		     &thistransfer,
		     raw->GetRawDataBlock(i),
		     raw->GetDataBlockSize(i));
    }
    else{
      err = compress2((Bytef*)(&buf[zipsize+sizeof(datablock_header)]),
		      &thistransfer,
		      raw->GetRawDataBlock(i),
		      raw->GetDataBlockSize(i),
		      _compression);
    }
    if(err != Z_OK){
      std::cerr<<"ConvertToGenroot: Unable to compress event datablocks in memory\n";
      throw std::runtime_error("Unable to compress datablock");

    }
    //write the header
    datablock_header* db_head = (datablock_header*)(&buf[zipsize]);
    db_head->total_blocksize_disk = sizeof(datablock_header)+thistransfer;
    db_head->datasize = raw->GetDataBlockSize(i);
    db_head->type = raw->GetDataBlockType(i);
    zipsize += db_head->total_blocksize_disk;
    
  }
  //set values in the event header
  event_header* ehead = (event_header*)(&buf[0]);
  ehead->event_size = zipsize;
  ehead->event_id = raw->GetID();
  ehead->timestamp = raw->GetTimestamp();
  ehead->nblocks = raw->GetNumDataBlocks();

  //see if we need to make a new file
  if(_ghead.nevents>=(uint32_t)_max_event_in_file || 
     _ghead.file_size + ehead->event_size > _max_file_size){
    CloseCurrentFile();
    OpenNewFile();
  }
  
  //actually write the event
  if(!_fout.write((const char*)(&buf[0]), zipsize)){
    std::cerr<<"ConvertToGenroot: Error occurred when writing event "
	     <<ehead->event_id<<"to disk!\n";
    throw std::runtime_error("Unable to write event to disk");
  }
  _bytes_written += ehead->event_size;
  //update info for global header
  _ghead.nevents++;
  if(_ghead.event_id_min > ehead->event_id)
    _ghead.event_id_min = ehead->event_id;
  _ghead.event_id_max = ehead->event_id;
  _ghead.file_size += ehead->event_size;
  
}

DEFINE_ART_MODULE(darkart::ConvertToGenroot)
