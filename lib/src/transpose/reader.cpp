#include "reader.hpp"

using std::size_t;

template <typename UnsignedInteger>
static constexpr UnsignedInteger readUnsignedInteger(const unsigned char* data)
{
	constexpr unsigned int byteShift = 8;
	constexpr unsigned int tailShift = 8 * (sizeof(UnsignedInteger) - 1);

	uintmax_t val = 0;
	for (size_t i = 0; i < sizeof(UnsignedInteger); ++i) {
		val >>= byteShift;
		val |= static_cast<uintmax_t>(*(data++)) << tailShift;
	}
	return val;
}

namespace Transpose
{

Reader::Reader(const char* data)
    : data(reinterpret_cast<const unsigned char*>(data))
{
}

Reader::Reader(const unsigned char* data) : data(data) {}

Reader& Reader::operator>>(char& val)
{
	val = readUnsignedInteger<unsigned char>(data);
	data += sizeof val;
	return *this;
}

Reader& Reader::operator>>(int8_t& val)
{
	val = readUnsignedInteger<uint8_t>(data);
	data += sizeof val;
	return *this;
}

Reader& Reader::operator>>(int16_t& val)
{
	val = readUnsignedInteger<uint16_t>(data);
	data += sizeof val;
	return *this;
}

Reader& Reader::operator>>(int32_t& val)
{
	val = readUnsignedInteger<uint32_t>(data);
	data += sizeof val;
	return *this;
}

Reader& Reader::operator>>(int64_t& val)
{
	val = readUnsignedInteger<uint64_t>(data);
	data += sizeof val;
	return *this;
}

Reader& Reader::operator>>(uint8_t& val)
{
	val = readUnsignedInteger<uint8_t>(data);
	data += sizeof val;
	return *this;
}

Reader& Reader::operator>>(uint16_t& val)
{
	val = readUnsignedInteger<uint16_t>(data);
	data += sizeof val;
	return *this;
}

Reader& Reader::operator>>(uint32_t& val)
{
	val = readUnsignedInteger<uint32_t>(data);
	data += sizeof val;
	return *this;
}

Reader& Reader::operator>>(uint64_t& val)
{
	val = readUnsignedInteger<uint64_t>(data);
	data += sizeof val;
	return *this;
}

} // namespace Transpose
