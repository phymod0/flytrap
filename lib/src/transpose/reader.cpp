#include "reader.hpp"

using std::size_t;

template <typename PrimitiveInteger>
static constexpr const unsigned char* readInteger(const unsigned char* data,
						  PrimitiveInteger val)
{
	constexpr unsigned int halfShift = 4;
	constexpr unsigned int tailShift = 8 * ((sizeof val) - 1);

	for (size_t i = 0; i < sizeof val; ++i) {
		val = (static_cast<uint64_t>(val) >> halfShift) >> halfShift;
		val |= static_cast<uint64_t>(*(data++)) << tailShift;
	}
	return data;
}

namespace Transpose
{

Reader::Reader(const char* data)
    : data(reinterpret_cast<const unsigned char*>(data))
{
}

Reader::Reader(const unsigned char* data) : data(data) {}

Reader& Reader::operator>>(int8_t& val)
{
	data = readInteger<uint8_t>(data, val);
	return *this;
}

Reader& Reader::operator>>(int16_t& val)
{
	data = readInteger<uint16_t>(data, val);
	return *this;
}

Reader& Reader::operator>>(int32_t& val)
{
	data = readInteger<uint32_t>(data, val);
	return *this;
}

Reader& Reader::operator>>(int64_t& val)
{
	data = readInteger<uint64_t>(data, val);
	return *this;
}

Reader& Reader::operator>>(uint8_t& val)
{
	data = readInteger(data, val);
	return *this;
}

Reader& Reader::operator>>(uint16_t& val)
{
	data = readInteger(data, val);
	return *this;
}

Reader& Reader::operator>>(uint32_t& val)
{
	data = readInteger(data, val);
	return *this;
}

Reader& Reader::operator>>(uint64_t& val)
{
	data = readInteger(data, val);
	return *this;
}

} // namespace Transpose
