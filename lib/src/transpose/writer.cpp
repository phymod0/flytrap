#include "writer.hpp"

using std::size_t;

template <typename PrimitiveInteger>
static constexpr unsigned char* writeInteger(unsigned char* data,
					     PrimitiveInteger val)
{
	constexpr unsigned int halfShift = 4;
	constexpr PrimitiveInteger byteMask = 0xFF;

	for (size_t i = 0; i < sizeof val; ++i) {
		*(data++) = val & byteMask;
		val = (static_cast<uint64_t>(val) >> halfShift) >> halfShift;
	}
	return data;
}

namespace Transpose
{

Writer::Writer(char* data) : data(reinterpret_cast<unsigned char*>(data)) {}

Writer::Writer(unsigned char* data) : data(data) {}

Writer& Writer::operator<<(int8_t val)
{
	data = writeInteger<uint8_t>(data, val);
	return *this;
}

Writer& Writer::operator<<(int16_t val)
{
	data = writeInteger<uint16_t>(data, val);
	return *this;
}

Writer& Writer::operator<<(int32_t val)
{
	data = writeInteger<uint32_t>(data, val);
	return *this;
}

Writer& Writer::operator<<(int64_t val)
{
	data = writeInteger<uint64_t>(data, val);
	return *this;
}

Writer& Writer::operator<<(uint8_t val)
{
	data = writeInteger(data, val);
	return *this;
}

Writer& Writer::operator<<(uint16_t val)
{
	data = writeInteger(data, val);
	return *this;
}

Writer& Writer::operator<<(uint32_t val)
{
	data = writeInteger(data, val);
	return *this;
}

Writer& Writer::operator<<(uint64_t val)
{
	data = writeInteger(data, val);
	return *this;
}

} // namespace Transpose
