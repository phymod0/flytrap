/////////////////////////////////////////////////////////////////////
//////////////////// INCLUDE FILES TO TEST BELOW ////////////////////
/////////////////////////////////////////////////////////////////////

#include "../src/transpose/reader.cpp"
#include "../src/transpose/transpose.hpp"

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

#include "ctest.h"

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

TEST_DEFINE(correctPrimitiveReads, result)
{
	TEST_AUTONAME(result);

	unsigned char data[] = "\x08\x00\x00\x00"
			       "\x12\x23\x34\x45"
			       "\x01\x23\x45\x67\x89\xAB\xCD\xEF";
	Transpose::Reader reader = Transpose::Reader(&data[0]);

	int32_t n;
	unsigned char a, b, c, d;
	uint64_t m;
	reader >> n >> a >> b >> c >> d >> m;
	test_check(result, "Correct integer read", n == 8);
	test_check(result, "Correct characters read",
		   a == 0x12 && b == 0x23 && c == 0x34 && d == 0x45);
	test_check(result, "Correct long integer read",
		   m == 0xEFCDAB8967452301);
}

TEST_DEFINE(correctClassReads, result)
{
	TEST_AUTONAME(result);

	unsigned char data[] = "\x12\x23\x34\x45";
	Transpose::Reader reader = Transpose::Reader(&data[0]);

	GData gd = GData(0, 0, 0, 0);
	reader >> gd;
	test_check(result, "Correct class read",
		   gd.a == 0x12 && gd.b == 0x23 && gd.c == 0x34 &&
		       gd.d == 0x45);
}

TEST_DEFINE(correctSubclassReads, result)
{
	TEST_AUTONAME(result);
	unsigned char data[] = "\x08\x00\x00\x00\x12\x23\x34\x45";
	Transpose::Reader reader = Transpose::Reader(&data[0]);

	HelloData hd = HelloData(0, 0, 0, 0, 0);
	reader >> hd;
	test_check(result, "Correct subclass read",
		   hd.gData.a == 0x12 && hd.gData.b == 0x23 &&
		       hd.gData.c == 0x34 && hd.gData.d == 0x45);
}

TEST_START(correctPrimitiveReads, correctClassReads, correctSubclassReads, )
