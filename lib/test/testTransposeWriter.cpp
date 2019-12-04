/////////////////////////////////////////////////////////////////////
//////////////////// INCLUDE FILES TO TEST BELOW ////////////////////
/////////////////////////////////////////////////////////////////////

#include "../src/transpose/transpose.hpp"
#include "../src/transpose/writer.cpp"

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

#include "ctest.h"

#include <cstring>

// TODO: Test every primitive type

class GData
{
      public:
	unsigned char a, b, c, d;
	GData(unsigned char a, unsigned char b, unsigned char c,
	      unsigned char d)
	    : a(a), b(b), c(c), d(d)
	{
	}
	void readTranspose(Transpose::Reader& reader)
	{
		reader >> a >> b >> c >> d;
	}
	void writeTranspose(Transpose::Writer& writer) const
	{
		writer << a << b << c << d;
	}
};

class HelloData
{
      public:
	int g;
	GData gData;
	HelloData(int g, unsigned char a, unsigned char b, unsigned char c,
		  unsigned char d)
	    : g(g), gData(a, b, c, d)
	{
	}
	void readTranspose(Transpose::Reader& reader) { reader >> g >> gData; }
	void writeTranspose(Transpose::Writer& writer) const
	{
		writer << g << gData;
	}
};

TEST_DEFINE(correctPrimitiveWrites, result)
{
	TEST_AUTONAME(result);

	unsigned char data[16];
	unsigned char expectedData[] = "\x08\x00\x00\x00"
				       "\x12\x23\x34\x45"
				       "\x01\x23\x45\x67\x89\xAB\xCD\xEF";
	Transpose::Writer writer = Transpose::Writer(&data[0]);

	int32_t n = 8;
	unsigned char a = 0x12, b = 0x23, c = 0x34, d = 0x45;
	uint64_t m = 0xEFCDAB8967452301;
	writer << n << a << b << c << d << m;
	test_check(result, "Correct integer write",
		   memcmp(data, expectedData, 4) == 0);
	test_check(result, "Correct characters written",
		   memcmp(data + 4, expectedData + 4, 4) == 0);
	test_check(result, "Correct long integer write",
		   memcmp(data + 8, expectedData + 8, 8) == 0);
}

TEST_DEFINE(correctClassWrites, result)
{
	TEST_AUTONAME(result);

	unsigned char data[4];
	unsigned char expectedData[] = "\xAB\xBC\xCD\xDA";
	Transpose::Writer writer = Transpose::Writer(&data[0]);

	GData gd = GData(0xAB, 0xBC, 0xCD, 0xDA);
	writer << gd;
	test_check(result, "Correct class write",
		   memcmp(data, expectedData, 4) == 0);
}

TEST_DEFINE(correctSubclassWrites, result)
{
	TEST_AUTONAME(result);

	unsigned char data[8];
	unsigned char expectedData[] = "\x21\x43\x34\x12\x12\x23\x34\x45";
	Transpose::Writer writer = Transpose::Writer(&data[0]);

	HelloData hd = HelloData(0x12344321, 0x12, 0x23, 0x34, 0x45);
	writer << hd;
	test_check(result, "Correct subclass write",
		   memcmp(data, expectedData, sizeof data) == 0);
}

TEST_START(correctPrimitiveWrites, correctClassWrites, correctSubclassWrites, )
