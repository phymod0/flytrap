/////////////////////////////////////////////////////////////////////
//////////////////// INCLUDE FILES TO TEST BELOW ////////////////////
/////////////////////////////////////////////////////////////////////

#include "../src/edb/database.hpp"

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

#include "ctest.h"

#include "../src/transpose/writer.cpp"
#include "../src/transpose/writer.hpp"

#include <cstdio>
#include <iostream>
#include <string>

struct HelloWorld {
	char letter1, letter2;
	int num1, num2, num3;
	char greeting[5];
};

template <> void EDBSerializer<HelloWorld>(const HelloWorld& entry, char* data)
{
	Transpose::Writer writer(data);
	writer << entry.letter1 << entry.letter2;
	writer << entry.num1 << entry.num2 << entry.num3;
	for (char greetingChar : entry.greeting) {
		writer << greetingChar;
	}
}

// template <> HelloWorld EDBDeserializer<HelloWorld>(const char* data) {}

template <> constexpr size_t EDBSize<HelloWorld>()
{
	return sizeof(char) + sizeof(char) + sizeof(int) + sizeof(int) +
	       sizeof(int) + 5 * sizeof(char);
}

uint32_t readStreamUint32(FILE* fd)
{
	constexpr unsigned int byteShift = 8;
	constexpr unsigned int tailShift = 24;

	uint32_t n;
	for (int i = 0; i < 4; ++i) {
		unsigned int c;
		c = fgetc(fd);
		n = (n >> byteShift) | (c << tailShift);
	}
	return n;
}

TEST_DEFINE(createNewDatabase, result)
{
	TEST_AUTONAME(result);

	const std::string filename = "helloworld.db";

	EDB::Database<HelloWorld> db(filename);
	FILE* fd = fopen(filename.c_str(), "rb");
	uint32_t initialEntryCount = readStreamUint32(fd),
		 entrySize = readStreamUint32(fd);
	long fileSize = (fseek(fd, 0, SEEK_END), ftell(fd));

	fclose(fd);
	remove(filename.c_str());

	test_check(result, "Database file is initialized", fd != nullptr);
	test_check(result, "Database file has the correct initial size",
		   fileSize == sizeof(uint32_t) + sizeof(uint32_t));
	test_check(result, "Database file has the correct initial entry count",
		   initialEntryCount == 0);
	test_check(result, "Database file had the correct initial entry size",
		   entrySize == EDBSize<HelloWorld>());
}

TEST_DEFINE(doNotOverwriteExistingDatabase, result)
{
	TEST_AUTONAME(result);

	const std::string filename = "helloworld.db";

	try {
		EDB::Database<HelloWorld> db(filename);
		HelloWorld hw;
		db.putNew(hw);
	} catch (const std::exception& e) {
		std::cout << "Got exception: " << e.what() << std::endl;
	}

	EDB::Database<HelloWorld> db(filename);
	FILE* fd = fopen(filename.c_str(), "rb");
	long fileSize = (fseek(fd, 0, SEEK_END), ftell(fd));
	long expectedFileSize = 3 * sizeof(uint32_t) + EDBSize<HelloWorld>();

	printf("Comparing %ld and %ld\n", fileSize, expectedFileSize);

	fclose(fd);
	remove(filename.c_str());

	test_check(result, "File size preserved on re-open",
		   fileSize == expectedFileSize);
}

TEST_START(createNewDatabase, doNotOverwriteExistingDatabase)
