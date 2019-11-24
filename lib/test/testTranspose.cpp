/////////////////////////////////////////////////////////////////////
//////////////////// INCLUDE FILES TO TEST BELOW ////////////////////
/////////////////////////////////////////////////////////////////////


#include "transpose/transpose.hpp"
#include "transpose/reader.cpp"


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


#include "ctest.h"


class GData {
public:
    unsigned char a, b, c, d;
    GData(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
        : a(a), b(b), c(c), d(d)
    { }
    void readTranspose(Transpose::Reader& reader)
    {
        reader >> a >> b >> c >> d;
    }
    void writeTranspose(Transpose::Writer& writer) const
    {
        writer << a << b << c << d;
    }
};


class HelloData {
public:
    int g;
    GData gData;
    HelloData(int g, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
        : g(g), gData(a, b, c, d)
    { }
    void readTranspose(Transpose::Reader& reader)
    {
        reader >> g >> gData;
    }
    void writeTranspose(Transpose::Writer& writer) const
    {
        writer << g << gData;
    }
};


/*
void testReader()
{
    unsigned char data[8];
    Transpose::Writer writer = Transpose::Writer(&data[0]);
    HelloData hd = HelloData(0x12344321, 0xAB, 0xBC, 0xCD, 0xDA);
    writer << hd;
    printf("0x%.2x 0x%.2x 0x%.2x 0x%.2x\n", data[0], data[1], data[2], data[3]);
    printf("0x%.2x 0x%.2x 0x%.2x 0x%.2x\n", data[4], data[5], data[6], data[7]);
}
*/


TEST_DEFINE(correctPrimitiveReads, result)
{
    TEST_AUTONAME(result);

    unsigned char data[] = "\x08\x00\x00\x00\x12\x23\x34\x45";
    Transpose::Reader reader = Transpose::Reader(&data[0]);

    int32_t n;
    unsigned char a, b, c, d;
    reader >> n >> a >> b >> c >> d;
    test_check(result, "Correct integer read", n == 8);
    test_check(result, "Correct characters read",
            a == 0x12 && b == 0x23 && c == 0x34 && d == 0x45);
}


TEST_DEFINE(correctNonprimitiveReads, result)
{
    TEST_AUTONAME(result);

    HelloData hd = HelloData(0, 0, 0, 0, 0);
    unsigned char data[] = "\x08\x00\x00\x00\x12\x23\x34\x45";
    Transpose::Reader reader = Transpose::Reader(&data[0]);

    reader >> hd;
    test_check(result, "Correct class read",
            hd.g == 8 && hd.gData.a == 0x12 && hd.gData.b == 0x23 &&
            hd.gData.c == 0x34 && hd.gData.d == 0x45);
}


TEST_START(
    correctPrimitiveReads,
    correctNonprimitiveReads,
)
