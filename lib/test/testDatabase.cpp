/////////////////////////////////////////////////////////////////////
//////////////////// INCLUDE FILES TO TEST BELOW ////////////////////
/////////////////////////////////////////////////////////////////////

#include "../src/edb/edb.hpp"
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

	uint32_t n = 0;
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

	EDB::DatabaseHandle<HelloWorld> db(filename);
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
		EDB::DatabaseHandle<HelloWorld> db(filename);
		HelloWorld hw;
		db.putNew(hw);
	} catch (const std::exception& e) {
		std::cout << "Got exception: " << e.what() << std::endl;
	}

	EDB::DatabaseHandle<HelloWorld> db(filename);
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
		const size_t size = getRandom<size_t>(10, 1000);

		EDB::DatabaseHandle<HelloWorld> db(filename);

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

TEST_DEFINE(databaseGetAndPut, result)
{
	TEST_AUTONAME(result);

	const std::string filename = "helloworld.db";

	{
		const size_t size = getRandom<size_t>(10, 1000);

		EDB::DatabaseHandle<HelloWorld> db(filename);

		srand(getpid());

		// Write initial entries
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

		// Modify random entries with put()
		for (size_t i = 0; i < size; ++i) {
			if (getRandom(0, 10) % 3 != 0) {
				continue;
			}
			HelloWorld& tempHw = hwArray[i];
			EDB::Result<HelloWorld> res = db.get(newIds[i]);
			res.data.letter1 = tempHw.letter1 = getRandom('a', 'z');
			res.data.letter2 = tempHw.letter2 = getRandom('p', 't');
			res.data.num1 = tempHw.num1 = getRandom(0, 100);
			res.data.num2 = tempHw.num2 = getRandom(2, 60);
			res.data.num3 = tempHw.num3 = getRandom(100, 1000);
			db.put(res);
		}

		bool correctData = true;

		// Read and verify
		for (size_t i = 0; i < size; ++i) {
			const EDB::ID entryId = newIds[i];
			const HelloWorld hw = hwArray[i];

			const EDB::Result<HelloWorld> res = db.get(entryId);

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

		test_check(result,
			   "Entries read from get as written through putNew",
			   correctData);
	}

	remove(filename.c_str());
}

TEST_DEFINE(databaseErase, result)
{
	TEST_AUTONAME(result);

	const std::string filename = "helloworld.db";

	{
		const size_t size = getRandom<size_t>(10, 1000);

		EDB::DatabaseHandle<HelloWorld> db(filename);

		srand(getpid());

		// Write initial entries
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

		using EraseType =
		    enum { ERASE_BY_ID,
			   ERASE_BY_RESULT,
			   DONT_ERASE,
		    };
		std::vector<EraseType> idxEraseType(size);
		// Erase random entries
		for (size_t i = 0; i < size; ++i) {
			const EDB::ID id = newIds[i];
			const int random = getRandom(0, 100) % 7;
			if (random < 3) {
				idxEraseType[i] = DONT_ERASE;
				continue;
			}
			if (random < 5) {
				idxEraseType[i] = ERASE_BY_ID;
				db.erase(id);
			} else {
				idxEraseType[i] = ERASE_BY_RESULT;
				EDB::Result<HelloWorld> res = db.get(id);
				db.erase(res);
			}
		}

		bool undeletedExist = true;
		bool erasedById = true;
		bool erasedByResult = true;

		// Verify deletion
		auto isDeleted = [&](EDB::ID entryId) {
			try {
				db.get(entryId);
				return false;
			} catch (const std::runtime_error& e) {
				/* TODO: compare with what() */
				return true;
			}
		};
		for (size_t i = 0; i < size; ++i) {
			const EDB::ID entryId = newIds[i];
			const EraseType eraseType = idxEraseType[i];
			const bool deleted = isDeleted(entryId);

			undeletedExist =
			    undeletedExist and
			    (eraseType != DONT_ERASE or not deleted);

			erasedById = erasedById and
				     (eraseType != ERASE_BY_ID or deleted);

			erasedByResult = erasedByResult and
					 (eraseType != ERASE_BY_ID or deleted);
		}

		test_check(result, "Unerased entries still exist",
			   undeletedExist);
		test_check(result, "Entries deleted by ID are deleted",
			   erasedById);
		test_check(result, "Entries deleted by result are deleted",
			   erasedByResult);
	}

	remove(filename.c_str());
}

TEST_DEFINE(databaseIteration, result)
{
	TEST_AUTONAME(result);

	const std::string filename = "helloworld.db";

	{
		const size_t size = getRandom<size_t>(10, 1000);

		EDB::DatabaseHandle<HelloWorld> db(filename);

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

		bool correctIteration = true;
		bool correctIds = true;

		// Read and verify
		size_t i = 0;
		for (const EDB::Result<HelloWorld>& res : db) {
			const EDB::ID id = newIds[i];
			const HelloWorld hw = hwArray[i];
			correctIteration =
			    correctIteration and res.data.letter1 == hw.letter1;
			correctIteration =
			    correctIteration and res.data.letter2 == hw.letter2;
			correctIteration =
			    correctIteration and res.data.num1 == hw.num1;
			correctIteration =
			    correctIteration and res.data.num2 == hw.num2;
			correctIteration =
			    correctIteration and res.data.num3 == hw.num3;
			correctIteration =
			    correctIteration and
			    memcmp(res.data.greeting, hw.greeting, 5) == 0;
			correctIds = correctIds and id == res.id;
			++i;
		}

		test_check(result, "Iteration returns correct data",
			   correctIteration);
		test_check(result, "Iteration goes over IDs in correct order",
			   correctIds);
	}

	remove(filename.c_str());
}

TEST_START(createNewDatabase, doNotOverwriteExistingDatabase, putNewAndGet,
	   databaseGetAndPut, databaseErase, databaseIteration)
