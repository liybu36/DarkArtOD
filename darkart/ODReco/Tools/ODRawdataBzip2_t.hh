#ifndef __OD_RAWDATA_BZIP2_T
#define __OD_RAWDATA_BZIP2_T

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include "ODRawdataFile_t.hh"

class ODRawdataBzip2_t: public ODRawdataFile_t
{
  public:
	  ODRawdataBzip2_t();
    ~ODRawdataBzip2_t();
    
    //File Functions
    void open(const std::string&);
    bool is_open() const { return ((_file!=NULL) && (_file!=0) && (_b_file_exist)); }
    void close();
    bool eof() const { return _eof; }
    
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
    FILE* _file;
    bool _eof;
    bool _b_file_exist;
    
    void specialread(void*, size_t);
    
};

#endif
