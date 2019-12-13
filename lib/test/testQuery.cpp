/////////////////////////////////////////////////////////////////////
//////////////////// INCLUDE FILES TO TEST BELOW ////////////////////
/////////////////////////////////////////////////////////////////////

#include "../src/edb/database.hpp"
#include "../src/edb/query.hpp"
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

EDB::Database<HelloWorld> createRandomDatabase(const std::string& dbPath)
{
	const size_t size = getRandom<size_t>(10, 1000);
	EDB::Database<HelloWorld> db(dbPath);

	srand(getpid());

	// Write entries
	for (size_t i = 0; i < size; ++i) {
		HelloWorld hw;
		hw.letter1 = getRandom('a', 'z');
		hw.letter2 = getRandom('p', 't');
		hw.num1 = getRandom(0, 100);
		hw.num2 = getRandom(2, 60);
		hw.num3 = getRandom(100, 1000);
		std::vector<char> chars = getRandomNIntegers<char>(5);
		memcpy(hw.greeting, &chars[0], 5);
		db.putNew(hw);
	}

	return db;
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

TEST_DEFINE(databaseEntryInclusion, result)
{
	TEST_AUTONAME(result);

	const std::string filename = "helloworld.db";
	EDB::Database<HelloWorld> db = createRandomDatabase(filename);

	const EDB::Query<HelloWorld>::KeepFn filterFn =
	    [](const HelloWorld& hw) -> bool {
		return hw.num3 >= 200 and hw.num3 <= 800;
	};

	EDB::Query<HelloWorld> query = db.query().filter(filterFn);

	bool correctIncluded = true;
	bool correctExcluded = true;

	HelloWorld hw;
	hw.num3 = 0;
	for (hw.num3 = 0; hw.num3 < 200; ++hw.num3) {
		correctExcluded = correctExcluded and not query.includes(hw);
	}
	for (hw.num3 = 200; hw.num3 <= 800; ++hw.num3) {
		correctIncluded = correctIncluded and query.includes(hw);
	}
	for (hw.num3 = 801; hw.num3 < 1000; ++hw.num3) {
		correctExcluded = correctExcluded and not query.includes(hw);
	}

	test_check(result, "Inclusion is complete", correctIncluded);
	test_check(result, "Inclusion is sound", correctExcluded);

	remove(filename.c_str());
}

TEST_DEFINE(databaseEntryFilter, result)
{
	TEST_AUTONAME(result);

	const std::string filename = "helloworld.db";
	EDB::Database<HelloWorld> db = createRandomDatabase(filename);

	const EDB::Query<HelloWorld>::KeepFn filterFn1 =
	    [](const HelloWorld& hw) -> bool {
		return hw.num1 >= 0 and hw.num1 <= 60;
	};

	const EDB::Query<HelloWorld>::KeepFn filterFn2 =
	    [](const HelloWorld& hw) -> bool {
		return hw.num2 >= 10 and hw.num2 <= 50;
	};

	const EDB::Query<HelloWorld>::KeepFn filterFn3 =
	    [](const HelloWorld& hw) -> bool {
		return hw.num3 >= 200 and hw.num3 <= 600;
	};

	EDB::Query<HelloWorld> query =
	    db.query().filter(filterFn1).filter(filterFn2).filter(filterFn3);

	bool correctFiltered = true;
	bool correctFilteredOut = true;

	for (const EDB::Result<HelloWorld>& res : db) {
		const HelloWorld& hw = res.data;
		const bool shouldInclude =
		    filterFn1(hw) and filterFn2(hw) and filterFn3(hw);
		const bool isIncluded = query.includes(hw);
		correctFiltered =
		    correctFiltered and (not shouldInclude or isIncluded);
		correctFilteredOut =
		    correctFilteredOut and (shouldInclude or not isIncluded);
	}

	test_check(result, "Filtering is complete", correctFiltered);
	test_check(result, "Filtering is sound", correctFilteredOut);

	remove(filename.c_str());
}

TEST_DEFINE(databaseEntrySort, result)
{
	TEST_AUTONAME(result);

	const std::string filename = "helloworld.db";
	EDB::Database<HelloWorld> db = createRandomDatabase(filename);

	const EDB::Query<HelloWorld>::OrderFn order1 =
	    [](const HelloWorld& hwl, const HelloWorld& hwr) -> int {
		return hwl.num1 - hwr.num1;
	};

	const EDB::Query<HelloWorld>::OrderFn order2 =
	    [](const HelloWorld& hwl, const HelloWorld& hwr) -> int {
		return hwl.num2 - hwr.num2;
	};

	const EDB::Query<HelloWorld>::OrderFn order3 =
	    [](const HelloWorld& hwl, const HelloWorld& hwr) -> int {
		return hwl.num3 - hwr.num3;
	};

	EDB::Query<HelloWorld> query =
	    db.query().sort(order1).sort(order2).sort(order3);

	bool correctSortOrder = true;

	bool isFirst = true;
	HelloWorld prevHw;
	for (const EDB::Result<HelloWorld>& res : query.fetch()) {
		const HelloWorld& hw = res.data;
		if (isFirst) {
			isFirst = false;
			prevHw = hw;
			continue;
		}

		int ord1 = order1(prevHw, hw);
		int ord2 = order2(prevHw, hw);
		int ord3 = order3(prevHw, hw);
		if (ord3 > 0) {
			correctSortOrder = false;
			break;
		}
		if (ord3 < 0) {
			prevHw = hw;
			continue;
		}
		if (ord2 > 0) {
			correctSortOrder = false;
			break;
		}
		if (ord2 < 0) {
			prevHw = hw;
			continue;
		}
		if (ord1 > 0) {
			correctSortOrder = false;
			break;
		}
		if (ord1 <= 0) {
			prevHw = hw;
			continue;
		}
	}

	test_check(result, "Multiple query sorts result in correct order",
		   correctSortOrder);

	remove(filename.c_str());
}

TEST_START(databaseEntryInclusion, databaseEntryFilter, databaseEntrySort)
