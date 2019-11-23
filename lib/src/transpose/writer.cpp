#include "writer.hpp"


namespace Transpose
{

Writer::Writer(char* data)
        : data(data)
{ }


Writer::Writer(unsigned char* data)
        : data((char*)data)
{ }


#define SHIFT_BYTE(val) (val >>= 7, val >>= 1)
#define WRITE_BYTE(val, data) \
        (*(data++) = val & 0xFF, SHIFT_BYTE(val))


Writer& Writer::operator<<(int8_t val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                WRITE_BYTE(val, data);
        return *this;
}


Writer& Writer::operator<<(int16_t val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                WRITE_BYTE(val, data);
        return *this;
}


Writer& Writer::operator<<(int32_t val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                WRITE_BYTE(val, data);
        return *this;
}


Writer& Writer::operator<<(int64_t val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                WRITE_BYTE(val, data);
        return *this;
}


Writer& Writer::operator<<(uint8_t val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                WRITE_BYTE(val, data);
        return *this;
}


Writer& Writer::operator<<(uint16_t val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                WRITE_BYTE(val, data);
        return *this;
}


Writer& Writer::operator<<(uint32_t val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                WRITE_BYTE(val, data);
        return *this;
}


Writer& Writer::operator<<(uint64_t val)
{
        for (size_t i = 0; i < sizeof val; ++i)
                WRITE_BYTE(val, data);
        return *this;
}


#undef WRITE_BYTE
#undef SHIFT_BYTE

}
