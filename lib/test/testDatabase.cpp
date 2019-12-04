/////////////////////////////////////////////////////////////////////
//////////////////// INCLUDE FILES TO TEST BELOW ////////////////////
/////////////////////////////////////////////////////////////////////

#include "../src/edb/database.hpp"
#include "../src/edb/result.hpp"
#include "../src/transpose/reader.cpp"
#include "../src/transpose/reader.hpp"
#include "../src/transpose/writer.cpp"
#include "../src/transpose/writer.hpp"

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

#include "ctest.h"
#include "ctestUtils.hpp"

#include <cstdio>
#include <iostream>
#include <string>
#include <unistd.h>

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

template <> HelloWorld EDBDeserializer<HelloWorld>(const char* data)
{
	HelloWorld entry;
	Transpose::Reader reader(data);
	reader >> entry.letter1 >> entry.letter2;
	reader >> entry.num1 >> entry.num2 >> entry.num3;
	for (char& greetingChar : entry.greeting) {
		reader >> greetingChar;
	}
	return entry;
}

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

	fclose(fd);
	remove(filename.c_str());

	test_check(result, "File size preserved on re-open",
		   fileSize == expectedFileSize);
}

TEST_DEFINE(putNewAndGet, result)
{
	TEST_AUTONAME(result);

	const std::string filename = "helloworld.db";

	{
		const size_t size = getRandom<size_t>(10, 100);

		EDB::Database<HelloWorld> db(filename);

		srand(getpid());

		// Write entries
		std::vector<HelloWorld> hwArray(size);
		std::vector<EDB::ID> newIds;
		for (HelloWorld& hw : hwArray) {
			hw.letter1 = getRandom('a', 'z');
			hw.letter2 = getRandom('p', 't');
			hw.num1 = getRandom(0, 100);
			hw.num2 = getRandom(2, 60);
			hw.num3 = getRandom(100, 1000);
			std::vector<char> chars = getRandomNIntegers<char>(5);
			memcpy(hw.greeting, &chars[0], 5);
			newIds.push_back(db.putNew(hw));
		}

		bool correctIds = true;
		bool correctData = true;

		// Read and verify
		for (size_t i = 0; i < size; ++i) {
			const EDB::ID entryId = newIds[i];
			const HelloWorld hw = hwArray[i];

			const EDB::Result<HelloWorld> res = db.get(entryId);

			correctIds = correctIds and res.id == entryId;

			correctData =
			    correctData and res.data.letter1 == hw.letter1;
			correctData =
			    correctData and res.data.letter2 == hw.letter2;
			correctData = correctData and res.data.num1 == hw.num1;
			correctData = correctData and res.data.num2 == hw.num2;
			correctData = correctData and res.data.num3 == hw.num3;
			correctData =
			    correctData and
			    memcmp(res.data.greeting, hw.greeting, 5) == 0;
		}

		test_check(result, "ID's read from get as returned from putNew",
			   correctIds);
		test_check(result,
			   "Entries read from get as written through putNew",
			   correctData);
	}

	remove(filename.c_str());
}

TEST_START(createNewDatabase, doNotOverwriteExistingDatabase, putNewAndGet)
