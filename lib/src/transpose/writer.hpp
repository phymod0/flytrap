#ifndef TRANSPOSE_WRITER
#define TRANSPOSE_WRITER


#include <cstdint>


namespace Transpose
{
class Writer
{
      private:
	unsigned char* data;

      public:
	explicit Writer(char* data);
	explicit Writer(unsigned char* data);

	template <typename T> Writer& operator<<(const T& val);
	Writer& operator<<(char val);
	Writer& operator<<(int8_t val);
	Writer& operator<<(int16_t val);
	Writer& operator<<(int32_t val);
	Writer& operator<<(int64_t val);
	Writer& operator<<(uint8_t val);
	Writer& operator<<(uint16_t val);
	Writer& operator<<(uint32_t val);
	Writer& operator<<(uint64_t val);
};


template <typename T> Writer& Writer::operator<<(const T& val)
{
	val.writeTranspose(*this);
	return *this;
}
} // namespace Transpose


#endif /* TRANSPOSE_WRITER */
