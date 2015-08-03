#include "ODRawdataDat_t.hh"

ODRawdataDat_t::ODRawdataDat_t()
{
  _file_size = 0;
  _byte_read = 0;
  _byte_read_tot = 0;
    
  _error = false;
  _error_code = 0;
  _error_description = "";
}

ODRawdataDat_t::~ODRawdataDat_t()
{
  _file.close();
}

//File Function
void ODRawdataDat_t::open(const std::string& path)
{
  _file.open(path, std::ios::in | std::ios::binary | std::ios::ate);
  if(_file.is_open()){
    _file_size = _file.tellg();
    _file.seekg (0, std::ios::beg);
    _byte_read = 0;
    _byte_read_tot = 0;
  }
  else{ generate_error(-1, "Can not open file: " + path); }
}

void ODRawdataDat_t::close()
{
  if(_file.is_open()){
      _file.close();
  }
}


//Read Function
void ODRawdataDat_t::specialread(void* var, size_t size)
{
  if(_byte_read_tot + size > _file_size){
    generate_error(-2, "No more byte to read in the file");
    return;
  }
        
  _byte_read = size;
  _byte_read_tot += _byte_read;
  _file.read(reinterpret_cast<char*>(var), size);
        
}

