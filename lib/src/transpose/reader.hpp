#ifndef TRANSPOSE_READER
#define TRANSPOSE_READER


#include <stdint.h>
#include <stddef.h>


namespace Transpose
{

class Reader
{
private:
        const char* data;
public:
        Reader(const char* data);
        Reader(const unsigned char* data);

        template<typename T>
        Reader& operator>>(T& val);
        Reader& operator>>(int8_t& val);
        Reader& operator>>(int16_t& val);
        Reader& operator>>(int32_t& val);
        Reader& operator>>(int64_t& val);
        Reader& operator>>(uint8_t& val);
        Reader& operator>>(uint16_t& val);
        Reader& operator>>(uint32_t& val);
        Reader& operator>>(uint64_t& val);
};

template<typename T>
Reader& Reader::operator>>(T& val)
{
        val.readTranspose(*this);
        return *this;
}

}


#endif /* TRANSPOSE_READER */
