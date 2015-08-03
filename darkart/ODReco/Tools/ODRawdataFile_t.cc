#include "ODRawdataFile_t.hh"

ODRawdataFile_t::ODRawdataFile_t(){
    _file_size = 0;
    _byte_read = 0;
    _byte_read_tot = 0;

    _error = false;
    _error_code = 0;
    _error_description = "";
}

ODRawdataFile_t::~ODRawdataFile_t(){
}

//ERROR Handling
void ODRawdataFile_t::generate_error(int code, const std::string & descr){
    if(!_error){
        _error = true;
        _error_code = code;
        _error_description = descr;
    }
}

//Read Function
size_t ODRawdataFile_t::binread(void* var, _var_type type, int dim = 1){
    if(this->is_open()){
        size_t size = 0;
        switch (type) {
            case _u8:
                size = sizeof(unsigned char);
                break;
            case _u16:
                size = sizeof(unsigned short);
                break;
            case _u32:
                size = sizeof(unsigned);
                break;
            case _i16:
                size = sizeof(short);
                break;
            case _i32:
                size = sizeof(int);
                break;
            case _dbl:
                size = sizeof(double);
                break;
            case _str:
                size = sizeof(char);
                break;
            case _bool:
                size = sizeof(bool);
                break;
            default:
                generate_error(-3, "Wrong type");
                return -3;
        }
        size *= dim;

        this->specialread(reinterpret_cast<char*>(var), size);

        return size;
    }
    else{
        generate_error(-4, "File is not opened");
        return -4;
    }
                                 }

unsigned char ODRawdataFile_t::read_u8(int dim=1){
    unsigned char temp;
    binread(reinterpret_cast<void*>(&temp), _u8, dim);
    return temp;
}

void ODRawdataFile_t::read_u8(unsigned char* temp, int dim){
    binread(reinterpret_cast<void*>(temp), _u8, dim);
}

unsigned short ODRawdataFile_t::read_u16(int dim=1){
    unsigned short temp;
    binread(reinterpret_cast<void*>(&temp), _u16, dim);
    return temp;
}

unsigned ODRawdataFile_t::read_u32(int dim=1){
    unsigned temp;
    binread(reinterpret_cast<void*>(&temp), _u32, dim);
    return temp;
}

short ODRawdataFile_t::read_i16(int dim=1){
    short temp;
    binread(reinterpret_cast<void*>(&temp), _i16, dim);
    return temp;
}

void ODRawdataFile_t::read_i16(short* temp, int dim){
    binread(reinterpret_cast<void*>(temp), _i16, dim);
}

int ODRawdataFile_t::read_i32(int dim=1){
    int temp;
    binread(reinterpret_cast<void*>(&temp), _i32, dim);
    return temp;
}

double ODRawdataFile_t::read_dbl(int dim=1){
    double temp;
    binread(reinterpret_cast<void*>(&temp), _dbl, dim);
    return temp;
}

std::string ODRawdataFile_t::read_str(){
    int string_size;
    binread(reinterpret_cast<void*>(&string_size), _i32);
    char temp[string_size];
    binread(reinterpret_cast<void*>(&temp[0]), _str, string_size);
    temp[string_size] = 0;
    return (std::string) temp;
}

bool ODRawdataFile_t::read_bool(int dim=1){
    bool temp;
    binread(reinterpret_cast<void*>(&temp), _bool, dim);
    return temp;
}


