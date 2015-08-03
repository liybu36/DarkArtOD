#ifndef __OD_RAWDATA_DAT_T
#define __OD_RAWDATA_DAT_T

#include <fstream>
#include <string>
#include "ODRawdataFile_t.hh"

class ODRawdataDat_t: public ODRawdataFile_t
{
  public:
    ODRawdataDat_t();
    ~ODRawdataDat_t();
  
    //File Functions
    void open(const std::string&);
    bool is_open() const { return _file.is_open(); }
    void close();
    bool eof() const { return ( (_byte_read_tot>=_file_size) || (_file.eof()) ); }
    
    //Size Function
    size_t get_file_size() const { return _file_size; }
    
    //Read Functions
    unsigned char read_u8(int);
    void read_u8(unsigned char*, int);
    
    unsigned short read_u16(int);
    unsigned read_u32(int);
    short read_i16(int);
    void read_i16(short*, int);
    int read_i32(int);
    double read_dbl(int);
    std::string read_str(int);
    bool read_bool(int);
	
  private:
    std::ifstream _file;
    
    void specialread(void*, size_t);
    
};

#endif
