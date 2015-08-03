#include "ODRawdataBzip2_t.hh"

ODRawdataBzip2_t::ODRawdataBzip2_t()
{
	_file = NULL;
	_eof = false;
	_b_file_exist = false;
	_file_size = 0;
	_byte_read = 0;
	_byte_read_tot = 0;
    
	_error = false;
	_error_code = 0;
	_error_description = "";
}

ODRawdataBzip2_t::~ODRawdataBzip2_t()
{
  pclose(_file);
}

//File Function
void ODRawdataBzip2_t::open(const std::string& path)
{
  // check if the file exists before using pipe and bzcat:
	// _file* after popen will be not null even if the file does not exists 
	std::ifstream itry(path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	if (itry.is_open()) { 
		_b_file_exist = true;
		_file_size = itry.tellg();
		_byte_read = 0;
		_byte_read_tot = 0;
		itry.close();

		_file = popen((std::string("bzcat ")+path).c_str(), "r");
	}
	else _b_file_exist = false;

	if ((_file == 0) || (_file == NULL)){
		perror("popen");
		generate_error(-1, "Can not open file: " + (std::string) path);
	}
	else{
		_eof = false;
		_byte_read = 0;
		_byte_read_tot = 0;
	}

}

void ODRawdataBzip2_t::close()
{
  if (this->is_open()){
    pclose(_file);
  }
  _file = 0;
}


//Read Function
void ODRawdataBzip2_t::specialread(void* var, size_t size)
{
  _byte_read = size;
  _byte_read_tot += _byte_read;

  const size_t result = fread(reinterpret_cast<char*>(var), size , 1, _file);
	
  if (result == 0){
    _eof = true;
  }
  else _eof = false;
        
}


