#ifndef EDB_DATABASE
#define EDB_DATABASE

/*
 * TODO:
 *      - Remove entrySize from the header or use for verification
 *      - std::shared_ptr<FILE> fd; instead of FILE* fd; in FileStream
 *      - Error status instead of exception throwing on not found
 *      - const-qualify member functions where possible
 *      - Implement readResult and writeResult to avoid code duplication
 *      - Privatize internal FileStream functions
 *      - Functionality to delete Result<Entry> list
 *      - Move FileStream outside class and move db-specific functions to
 *	  Database as wrappers
 * XXX:
 *	- Find a portable alternative to the re+b file open mode
 * FIXME:
 *	- Use C++ fstreams instead of file pointers then remove openFile +
 *	  possibly FileStream
 */

#include "common.hpp"
#include "query.hpp"
#include "result.hpp"

#include <cstdio>
#include <string>

namespace EDB
{

using CErrors::require;

template <typename Entry> struct Result;
template <typename Entry> class Query;

template <typename Entry> class Database
{
      private:
	static constexpr size_t dbHeaderSize = 8;
	using DBHeader = struct {
		uint32_t entryCount, entrySize;
	};

	static constexpr size_t entryHeaderSize = 4;
	using EntryHeader = struct {
		uint32_t entryId;
	};

	static constexpr size_t entrySize = EDBSize<Entry>();
	using EntryData = std::array<char, entrySize>;

	class FileStream
	{
	      private:
		FILE* openFile(const std::string& filepath);

		FILE* fd;
		std::string filepath;

	      public:
		void seekStart();
		void seekAt(long at);
		void seekAtEntry(uint32_t entryIdx);
		void seekEnd();
		long currentPos();
		long getFileSize();

		explicit FileStream(const std::string& filepath);
		FileStream(const FileStream& fs);
		FileStream& operator=(const FileStream& fs);
		~FileStream();

		void writeInteger(uint32_t n);
		uint32_t readInteger();
		void writeDBHeader(const DBHeader& header);
		DBHeader readDBHeader();
		void writeEntryCount(uint32_t entryCount);
		uint32_t readEntryCount();
		void writeEntryHeader(const EntryHeader& header);
		EntryHeader readEntryHeader();
		void writeEntryData(const EntryData& entryData);
		void readEntryData(EntryData& entryData);
	};

	FileStream dbStream;

	bool dbFileInitialized();
	void dbInitializeFile();
	ID getEntryId(uint32_t entryIdx);
	uint32_t getEntryCount();
	void setEntryCount(uint32_t entryCount);
	uint32_t getIdIndex(ID find);

      public:
	explicit Database(const std::string& filepath);

	Result<Entry> get(ID id);
	void put(const Result<Entry>& result);
	ID putNew(const Entry& entry);
	void erase(ID id);
	void erase(const Result<Entry>& result);
	void erase(const Query<Entry>& query);

	class iterator
	{
		friend class Database;

	      private:
		Database<Entry>& db;
		uint32_t entryIdx;
		bool ended;
		bool isEntryIdxValid(uint32_t entryIdx);
		void end();

	      public:
		explicit iterator(Database<Entry>& db);
		iterator& operator++();
		const iterator operator++(int);
		Result<Entry> operator*();
		bool operator==(const iterator& it);
		bool operator!=(const iterator& it);
	};
	iterator begin();
	iterator end();
	friend class iterator;

	Query<Entry> query();
};

template <typename Entry>
FILE* Database<Entry>::FileStream::openFile(const std::string& filepath)
{
	FILE* fd;
	require((fd = fopen(filepath.c_str(), "ab")) != NULL);   // NOLINT
	fclose(fd);                                              // NOLINT
	require((fd = fopen(filepath.c_str(), "re+b")) != NULL); // NOLINT
	return fd;
}

template <typename Entry> void Database<Entry>::FileStream::seekStart()
{
	require(fseek(fd, 0, SEEK_SET) == 0);
}

template <typename Entry> void Database<Entry>::FileStream::seekAt(long at)
{
	require(fseek(fd, at, SEEK_SET) == 0);
}

template <typename Entry>
void Database<Entry>::FileStream::seekAtEntry(uint32_t entryIdx)
{
	constexpr size_t entryBlockSize = entryHeaderSize + entrySize;
	constexpr size_t headerSize = dbHeaderSize;
	const long entryPos = headerSize + entryBlockSize * entryIdx;
	seekAt(entryPos);
}

template <typename Entry> void Database<Entry>::FileStream::seekEnd()
{
	require(fseek(fd, 0, SEEK_END) == 0);
}

template <typename Entry> long Database<Entry>::FileStream::currentPos()
{
	long result;
	require((result = ftell(fd)) >= 0);
	return result;
}

template <typename Entry> long Database<Entry>::FileStream::getFileSize()
{
	seekEnd();
	return currentPos();
}

template <typename Entry>
Database<Entry>::FileStream::FileStream(const std::string& filepath)
    : fd(openFile(filepath)), filepath(filepath)
{
}

template <typename Entry>
Database<Entry>::FileStream::FileStream(const FileStream& fs)
    : filepath(fs.filepath)
{
	const long pos = ftell(fs.fd);
	require(fd = fopen(fs.filepath.c_str(), "re+b"));
	if (pos >= 0) {
		seekAt(pos);
	}
}

template <typename Entry>
typename Database<Entry>::FileStream&
Database<Entry>::FileStream::operator=(const FileStream& fs)
{
	if (this == &fs) {
		return *this;
	}
	const long pos = ftell(fs.fd);
	fclose(fd);
	filepath = fs.filepath;
	require(fd = fopen(fs.filepath.c_str(), "re+b"));
	if (pos >= 0) {
		seekAt(pos);
	}
	return *this;
}

template <typename Entry> Database<Entry>::FileStream::~FileStream()
{
	fclose(fd);
}

template <typename Entry>
void Database<Entry>::FileStream::writeInteger(uint32_t n)
{
	constexpr uint32_t byteMask = 0xFF;
	constexpr unsigned int byteShift = 8;

	for (int i = 0; i < 4; ++i) {
		require(fputc(n & byteMask, fd) != EOF);
		n >>= byteShift;
	}
	require(fflush(fd) == 0);
}

template <typename Entry> uint32_t Database<Entry>::FileStream::readInteger()
{
	constexpr unsigned int byteShift = 8;
	constexpr unsigned int tailShift = 24;
	constexpr unsigned int eof = EOF;

	uint32_t n;
	for (int i = 0; i < 4; ++i) {
		unsigned int c;
		require((c = fgetc(fd)) != eof);
		n = (n >> byteShift) | (c << tailShift);
	}
	return n;
}

template <typename Entry>
void Database<Entry>::FileStream::writeDBHeader(
    const Database<Entry>::DBHeader& header)
{
	seekStart();
	writeInteger(header.entryCount);
	writeInteger(header.entrySize);
}

template <typename Entry>
typename Database<Entry>::DBHeader Database<Entry>::FileStream::readDBHeader()
{
	seekStart();
	const uint32_t entryCount = readInteger();
	const uint32_t entrySize = readInteger();

	return {entryCount, entrySize};
}

template <typename Entry>
void Database<Entry>::FileStream::writeEntryCount(uint32_t entryCount)
{
	DBHeader dbHeader = readDBHeader();
	dbHeader.entryCount = entryCount;
	writeDBHeader(dbHeader);
}

template <typename Entry> uint32_t Database<Entry>::FileStream::readEntryCount()
{
	DBHeader dbHeader = readDBHeader();
	return dbHeader.entryCount;
}

template <typename Entry>
void Database<Entry>::FileStream::writeEntryHeader(
    const Database<Entry>::EntryHeader& header)
{
	writeInteger(header.entryId);
}

template <typename Entry>
typename Database<Entry>::EntryHeader
Database<Entry>::FileStream::readEntryHeader()
{
	const ID entryId = readInteger();
	return {entryId};
}

template <typename Entry>
void Database<Entry>::FileStream::writeEntryData(const EntryData& entryData)
{
	const size_t nWritten = fwrite(entryData.data(), 1, entrySize, fd);
	if (nWritten < entrySize) {
		throw std::runtime_error("Failed to write entry");
	}
	require(fflush(fd) == 0);
}

template <typename Entry>
void Database<Entry>::FileStream::readEntryData(EntryData& entryData)
{
	const size_t nRead = fread(entryData.data(), 1, entrySize, fd);
	if (nRead < entrySize) {
		throw std::runtime_error("Failed to read entry");
	}
}

template <typename Entry> bool Database<Entry>::dbFileInitialized()
{
	return dbStream.getFileSize() > 0;
}

template <typename Entry> void Database<Entry>::dbInitializeFile()
{
	constexpr uint32_t entryCount = 0;
	dbStream.writeDBHeader({entryCount, entrySize});
}

template <typename Entry> ID Database<Entry>::getEntryId(uint32_t entryIdx)
{
	dbStream.seekAtEntry(entryIdx);
	return dbStream.readEntryHeader().entryId;
}

template <typename Entry> uint32_t Database<Entry>::getEntryCount()
{
	return dbStream.readEntryCount();
}

template <typename Entry>
void Database<Entry>::setEntryCount(uint32_t entryCount)
{
	return dbStream.writeEntryCount(entryCount);
}

template <typename Entry> uint32_t Database<Entry>::getIdIndex(ID find)
{
	uint32_t s = 0;
	uint32_t e = getEntryCount();
	while (e - s > 1) {
		const uint32_t m = (s + e) / 2;
		const ID id = getEntryId(m);
		if (find < id) {
			e = m;
		} else {
			s = m;
		}
	}
	if (getEntryId(s) == find) {
		return s;
	}
	throw std::runtime_error("ID not found in database");
}

template <typename Entry>
Database<Entry>::Database(const std::string& filepath) : dbStream(filepath)
{
	if (not dbFileInitialized()) {
		dbInitializeFile();
	}
}

template <typename Entry> Result<Entry> Database<Entry>::get(ID id)
{
	EntryData entryData;

	dbStream.seekAtEntry(getIdIndex(id));
	dbStream.readEntryHeader();
	dbStream.readEntryData(entryData);
	Entry& entry = EDBDeserializer<Entry>(entryData.data());
	return Result<Entry>(entry, id);
}

template <typename Entry> void Database<Entry>::put(const Result<Entry>& result)
{
	EntryData entryData;

	const ID entryId = result.id;
	dbStream.seekAtEntry(getIdIndex(entryId));
	dbStream.writeEntryHeader({entryId});
	EDBSerializer<Entry>(result.data, entryData.data());
	dbStream.writeEntryData(entryData);
}

template <typename Entry> ID Database<Entry>::putNew(const Entry& entry)
{
	auto createNewEntryId = [&](uint32_t entryCount) -> ID {
		if (entryCount == 0) {
			return 1;
		}
		const ID lastId = getEntryId(entryCount - 1);
		return lastId + 1;
	};

	EntryData entryData;
	const uint32_t entryCount = getEntryCount();
	const ID newEntryId = createNewEntryId(entryCount);

	dbStream.seekAtEntry(entryCount);
	dbStream.writeEntryHeader({newEntryId});
	EDBSerializer<Entry>(entry, entryData.data());
	dbStream.writeEntryData(entryData);
	setEntryCount(entryCount + 1);
	return newEntryId;
}

template <typename Entry> void Database<Entry>::erase(ID id)
{
	EntryData entryData;
	const uint32_t eraseIdx = getIdIndex(id);
	FileStream copyStream(dbStream);

	dbStream.seekAtEntry(eraseIdx);
	copyStream.seekAtEntry(eraseIdx);
	copyStream.readEntryHeader();
	copyStream.readEntryData(entryData);

	const uint32_t entryCount = getEntryCount();
	for (uint32_t copyIdx = eraseIdx + 1; copyIdx < entryCount; ++copyIdx) {
		dbStream.writeEntryHeader(copyStream.readEntryHeader());
		copyStream.readEntryData(entryData);
		dbStream.writeEntryData(entryData);
	}
	setEntryCount(entryCount - 1);
}

template <typename Entry>
void Database<Entry>::erase(const Result<Entry>& result)
{
	erase(result.id);
}

template <typename Entry> void Database<Entry>::erase(const Query<Entry>& query)
{
	EntryData entryData;
	FileStream copyStream(dbStream);

	dbStream.seekAtEntry(0);
	copyStream.seekAtEntry(0);

	const uint32_t entryCount = getEntryCount();
	uint32_t nDeleted = 0;
	for (uint32_t copyIdx = 0; copyIdx < entryCount; ++copyIdx) {
		const EntryHeader entryHeader = copyStream.readEntryHeader();
		copyStream.readEntryData(entryData);
		if (query.includes(EDBDeserializer<Entry>(entryData.data()))) {
			++nDeleted;
			continue;
		}
		dbStream.writeEntryHeader(entryHeader);
		dbStream.writeEntryData(entryData);
	}
	setEntryCount(entryCount - nDeleted);
}

template <typename Entry>
bool Database<Entry>::iterator::isEntryIdxValid(uint32_t entryIdx)
{
	return entryIdx < db.getEntryCount();
}

template <typename Entry> void Database<Entry>::iterator::end()
{
	ended = true;
}

template <typename Entry>
Database<Entry>::iterator::iterator(Database<Entry>& db)
    : db(db), entryIdx(0), ended(false)
{
}

template <typename Entry>
typename Database<Entry>::iterator& Database<Entry>::iterator::operator++()
{
	if (not isEntryIdxValid(++entryIdx)) {
		ended = true;
	}
	return *this;
}

template <typename Entry>
const typename Database<Entry>::iterator
Database<Entry>::iterator::operator++(int)
{
	Database<Entry>::iterator prev = *this;
	if (not isEntryIdxValid(++entryIdx)) {
		ended = true;
	}
	return prev;
}

template <typename Entry> Result<Entry> Database<Entry>::iterator::operator*()
{
	EntryData entryData;

	if (not isEntryIdxValid(entryIdx)) {
		throw std::runtime_error("Dereferenced invalidated iterator");
	}

	db.dbStream.seekAtEntry(entryIdx);
	EntryHeader entryHeader = db.dbStream.readEntryHeader();
	db.dbStream.readEntryData(entryData);
	Entry& entry = EDBDeserializer<Entry>(entryData.data());
	return Result<Entry>(entry, entryHeader.entryId);
}

template <typename Entry>
bool Database<Entry>::iterator::operator==(const Database<Entry>::iterator& it)
{
	return &db == &it.db and ended == it.ended and
	       (ended or entryIdx == it.entryIdx);
}

template <typename Entry>
bool Database<Entry>::iterator::operator!=(const Database<Entry>::iterator& it)
{
	return not operator==(it);
}

template <typename Entry>
typename Database<Entry>::iterator Database<Entry>::begin()
{
	return iterator(*this);
}

template <typename Entry>
typename Database<Entry>::iterator Database<Entry>::end()
{
	iterator it(*this);
	it.end();
	return it;
}

template <typename Entry> Query<Entry> Database<Entry>::query()
{
	return Query<Entry>(*this);
}

} // namespace EDB

#endif /* EDB_DATABASE */
