#include "writer.hpp"


using std::size_t;


template <typename UnsignedInteger>
static constexpr void writeUnsignedInteger(unsigned char* data,
					   UnsignedInteger val)
{
	constexpr unsigned int byteShift = 8;

	uintmax_t extendedVal = val;
	for (size_t i = 0; i < sizeof(UnsignedInteger); ++i) {
		*(data++) = extendedVal;
		extendedVal >>= byteShift;
	}
}


namespace Transpose
{
Writer::Writer(char* data) : data(reinterpret_cast<unsigned char*>(data)) {}


Writer::Writer(unsigned char* data) : data(data) {}


Writer& Writer::operator<<(char val)
{
	writeUnsignedInteger<unsigned char>(data, val);
	data += sizeof val;
	return *this;
}


Writer& Writer::operator<<(int8_t val)
{
	writeUnsignedInteger<uint8_t>(data, val);
	data += sizeof val;
	return *this;
}


Writer& Writer::operator<<(int16_t val)
{
	writeUnsignedInteger<uint16_t>(data, val);
	data += sizeof val;
	return *this;
}


Writer& Writer::operator<<(int32_t val)
{
	writeUnsignedInteger<uint32_t>(data, val);
	data += sizeof val;
	return *this;
}


Writer& Writer::operator<<(int64_t val)
{
	writeUnsignedInteger<uint64_t>(data, val);
	data += sizeof val;
	return *this;
}


Writer& Writer::operator<<(uint8_t val)
{
	writeUnsignedInteger(data, val);
	data += sizeof val;
	return *this;
}


Writer& Writer::operator<<(uint16_t val)
{
	writeUnsignedInteger(data, val);
	data += sizeof val;
	return *this;
}


Writer& Writer::operator<<(uint32_t val)
{
	writeUnsignedInteger(data, val);
	data += sizeof val;
	return *this;
}


Writer& Writer::operator<<(uint64_t val)
{
	writeUnsignedInteger(data, val);
	data += sizeof val;
	return *this;
}
} // namespace Transpose
