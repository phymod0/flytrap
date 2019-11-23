#include "reader.hpp"


namespace Transpose
{

Reader::Reader(const char* data)
        : data(data)
{ }


Reader::Reader(const unsigned char* data)
        : data((const char*)data)
{ }


#define RSHIFT_BYTE(val) (val >>= 7, val >>= 1)
#define READ_POS(val) (8*((sizeof val) - 1))
#define READ_BYTE(val, data) \
        (RSHIFT_BYTE(val), val |= ((int64_t)*(data++)) << READ_POS(val))


Reader& Reader::operator>>(int8_t& val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                READ_BYTE(val, data);
        return *this;
}


Reader& Reader::operator>>(int16_t& val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                READ_BYTE(val, data);
        return *this;
}


Reader& Reader::operator>>(int32_t& val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                READ_BYTE(val, data);
        return *this;
}


Reader& Reader::operator>>(int64_t& val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                READ_BYTE(val, data);
        return *this;
}


Reader& Reader::operator>>(uint8_t& val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                READ_BYTE(val, data);
        return *this;
}


Reader& Reader::operator>>(uint16_t& val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                READ_BYTE(val, data);
        return *this;
}


Reader& Reader::operator>>(uint32_t& val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                READ_BYTE(val, data);
        return *this;
}


Reader& Reader::operator>>(uint64_t& val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                READ_BYTE(val, data);
        return *this;
}


#undef READ_BYTE
#undef READ_POS
#undef RSHIFT_BYTE

}
