#ifndef TRANSPOSE_READER
#define TRANSPOSE_READER


#include <cstdint>


namespace Transpose
{
class Reader
{
      private:
	const unsigned char* data;

      public:
	explicit Reader(const char* data);
	explicit Reader(const unsigned char* data);

	template <typename T> Reader& operator>>(T& val);
	Reader& operator>>(char& val);
	Reader& operator>>(int8_t& val);
	Reader& operator>>(int16_t& val);
	Reader& operator>>(int32_t& val);
	Reader& operator>>(int64_t& val);
	Reader& operator>>(uint8_t& val);
	Reader& operator>>(uint16_t& val);
	Reader& operator>>(uint32_t& val);
	Reader& operator>>(uint64_t& val);
};


template <typename T> Reader& Reader::operator>>(T& val)
{
	val.readTranspose(*this);
	return *this;
}
} // namespace Transpose


#endif /* TRANSPOSE_READER */
