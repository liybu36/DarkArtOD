/* Helper class for DS50 raw data handling.
 * This is a pure virual class; 
 * Child classes for bzip and dat (non zip) files should be provided
 * */

#ifndef __OD_RAWDATA_FILE_T
#define __OD_RAWDATA_FILE_T

#include <fstream>
#include <string>

class ODRawdataFile_t
{
  public:
    ODRawdataFile_t();
    ~ODRawdataFile_t();
    
    //File Functions
    virtual void open(const std::string& ){ };
    virtual bool is_open() const = 0;
    virtual void close() { };
    virtual bool eof() const = 0;
    
    //ERROR Functions
    bool get_error() const { return _error; }
    int get_error_code() const { return _error_code; }
    const std::string& get_error_description() const { return _error_description; }
    
    //Size Function
    size_t get_file_size() const { return _file_size; }
    
    //Read Functions
    bool read_bool(int);
    unsigned char read_u8(int);
    void read_u8(unsigned char*, int);
    unsigned short read_u16(int);
    unsigned read_u32(int);
    short read_i16(int);
    void read_i16(short*, int);
    int read_i32(int);
    double read_dbl(int);
    std::string read_str();
	
  protected:
    size_t _file_size;
    size_t _byte_read;
    size_t _byte_read_tot;
    
    enum _var_type{ _u8, _u16, _u32, _i16 , _i32, _dbl, _str, _bool };
    size_t binread (void*, _var_type, int);

    virtual void specialread(void*, size_t) {};
    
    //ERROR Handling
    bool _error;
    int _error_code;
    std::string _error_description;
    
    void generate_error(int, const std::string&);
};

#endif
