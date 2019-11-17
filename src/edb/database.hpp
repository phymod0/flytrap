#ifndef EDB_DATABASE
#define EDB_DATABASE


/*
 * TODO:
 *      - Remove entrySize from the header or use for verification
 *      - Use file descriptors instead of pointers
 *      - Error status instead of exception throwing on not found
 *      - const functions
 *      - readResult and writeResult to avoid code duplication
 */


#include "common.hpp"
#include "result.hpp"
#include "query.hpp"

#include <stdio.h>
#include <string>


namespace EDB
{

template<typename Entry>
class Result;

template<typename Entry>
class Query;

template<typename Entry>
class Database
{
private:
#define DB_HDRSZ 8
        typedef struct { uint32_t entryCount, entrySize; } DBHeader;
#define ENTRY_HDRSZ 4
        typedef struct { uint32_t entryId; } EntryHeader;

        class FileStream {
        private:
                FILE* fd;
                std::string filename;

        public:
                void seekStart();
                void seekAt(long at);
                uint32_t seekAtEntry(uint32_t entryIdx);
                void seekEnd();
                long currentPos();
                long getFileSize();

                FileStream(const std::string& filename);
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
                void writeEntryData(const char* data);
                void readEntryData(char* data);
        };

        FileStream dbStream;

        bool dbFileInitialized();
        void dbInitializeFile();
        ID getEntryId(uint32_t entryIdx);
        uint32_t getEntryCount();
        void setEntryCount(uint32_t entryCount);
        uint32_t getIdIndex(ID find);

public:
        Database(const std::string& filename);

        Result<Entry> get(ID id);
        void put(const Result<Entry>& result);
        ID putNew(const Entry& entry);
        void erase(ID id);
        void erase(const Result<Entry>& result);
        void erase(const Query<Entry>& query);

        friend class iterator {
        private:
                Database<Entry>& db;
                uint32_t entryIdx;
                bool ended;
                bool isEntryIdxValid(uint32_t entryIdx);
        public:
                iterator(Database<Entry>& db);
                iterator& operator++();
                iterator operator++(int);
                Result<Entry> operator*();
                bool operator==(const iterator& it);
                bool operator!=(const iterator& it);
        };
        iterator begin();
        iterator end();

        Query<Entry> query();
};

template<typename Entry>
void Database<Entry>::FileStream::seekStart()
{
        require(fseek(fd, 0, SEEK_SET) == 0);
}

template<typename Entry>
void Database<Entry>::FileStream::seekAt(long at)
{
        require(fseek(fd, at, SEEK_SET) == 0);
}

template<typename Entry>
uint32_t Database<Entry>::FileStream::seekAtEntry(uint32_t entryIdx)
{
        constexpr size_t entrySize = ENTRY_HDRSZ + EDBSize<Entry>();
        constexpr size_t headerSize = DB_HDRSZ;
        const long entryPos = headerSize + entrySize*entryIdx;
        seekAt(entryPos);
}

template<typename Entry>
void Database<Entry>::FileStream::seekEnd()
{
        require(fseek(fd, 0, SEEK_END) == 0);
}

template<typename Entry>
long Database<Entry>::FileStream::currentPos()
{
        long result;
        require((result = ftell(fd)) >= 0);
        return result;
}

template<typename Entry>
long Database<Entry>::FileStream::getFileSize()
{
        seekEnd();
        return currentPos();
}

template<typename Entry>
Database<Entry>::FileStream::FileStream(const std::string& filename)
        : filename(filename)
{
        require(fd = fopen(filename.c_str(), "w+"));
}

template<typename Entry>
Database<Entry>::FileStream::FileStream(const FileStream& fs)
        : filename(fs.filename)
{
        const long pos = ftell(fs.fd);
        require(fd = fopen(fs.filename.c_str(), "w+"));
        if (pos >= 0) seekAt(pos);
}

template<typename Entry>
void Database<Entry>::FileStream::operator=(const FileStream& fs)
{
        const long pos = ftell(fs.fd);
        fclose(fd);
        filename = fs.filename;
        require(fd = fopen(fs.filename.c_str(), "w+"));
        if (pos >= 0) seekAt(pos);
        return *this;
}

template<typename Entry>
Database<Entry>::FileStream::~FileStream()
{
        fclose(fd);
}

template<typename Entry>
void Database<Entry>::FileStream::writeInteger(uint32_t n)
{
        for (int i = 0; i < 4; ++i) {
                require(fputc(n & 0xFF, fd) != EOF);
                n >>= 8;
        }
        require(fflush(fd) == 0);
}

template<typename Entry>
uint32_t Database<Entry>::FileStream::readInteger()
{
        uint32_t n;
        for (int i = 0; i < 4; ++i) {
                int c;
                require((c = fgetc(fd)) != EOF);
                n = (n >> 8) | (c << 24);
        }
        return n;
}

template<typename Entry>
void Database<Entry>::FileStream::writeDBHeader(const Database<Entry>::DBHeader& header)
{
        seekStart();
        writeInteger(header.entryCount);
        writeInteger(header.entrySize);
}

template<typename Entry>
typename Database<Entry>::DBHeader Database<Entry>::FileStream::readDBHeader()
{
        seekStart();
        const uint32_t entryCount = readInteger();
        const uint32_t entrySize = readInteger();

        return { entryCount, entrySize };
}

template<typename Entry>
void Database<Entry>::FileStream::writeEntryCount(uint32_t entryCount)
{
        DBHeader dbHeader = readDBHeader();
        dbHeader.entryCount = entryCount;
        writeDBHeader(dbHeader);
}

template<typename Entry>
uint32_t Database<Entry>::FileStream::readEntryCount()
{
        DBHeader dbHeader = readDBHeader();
        return dbHeader.entryCount;
}

template<typename Entry>
void Database<Entry>::FileStream::writeEntryHeader(const Database<Entry>::EntryHeader& header)
{
        writeInteger(header.entryId);
}

template<typename Entry>
typename Database<Entry>::EntryHeader Database<Entry>::FileStream::readEntryHeader()
{
        const ID entryId = readInteger();
        return { entryId };
}

template<typename Entry>
void Database<Entry>::FileStream::writeEntryData(const char* data)
{
        constexpr size_t entrySize = EDBSize<Entry>();
        const size_t nWritten = fwrite(data, 1, entrySize, fd);
        if (nWritten < entrySize)
                throw std::runtime_error("Failed to write entry");
        require(fflush(fd) == 0);
}

template<typename Entry>
void Database<Entry>::FileStream::readEntryData(char* data)
{
        constexpr size_t entrySize = EDBSize<Entry>();
        const size_t nRead = fread(data, 1, entrySize, fd);
        if (nRead < entrySize)
                throw std::runtime_error("Failed to read entry");
}

template<typename Entry>
bool Database<Entry>::dbFileInitialized()
{
        return dbStream.getFileSize() > 0;
}

template<typename Entry>
void Database<Entry>::dbInitializeFile()
{
        constexpr uint32_t entryCount = 0;
        constexpr uint32_t entrySize = EDBSize<Entry>();

        dbStream.writeDBHeader({ entryCount, entrySize });
}

template<typename Entry>
ID Database<Entry>::getEntryId(uint32_t entryIdx)
{
        dbStream.seekAtEntry(entryIdx);
        return dbStream.readEntryHeader().entryId;
}

template<typename Entry>
uint32_t Database<Entry>::getEntryCount()
{
        return dbStream.readEntryCount();
}

template<typename Entry>
void Database<Entry>::setEntryCount(uint32_t entryCount)
{
        return dbStream.writeEntryCount(entryCount);
}

template<typename Entry>
uint32_t Database<Entry>::getIdIndex(ID find)
{
        uint32_t s = 0, e = getEntryCount();
        while (e - s > 1) {
                const uint32_t m = (s + e) / 2;
                const ID id = getEntryId(m);
                if (find < id) e = m; else s = m;
        }
        if (getEntryId(s) == find)
                return s;
        throw std::runtime_error("ID not found in database");
}

template<typename Entry>
Database<Entry>::Database(const std::string& filename)
        : dbStream(filename)
{
        if (not dbFileInitialized()) dbInitializeFile();
}

template<typename Entry>
Result<Entry> Database<Entry>::get(ID id)
{
        dbStream.seekAtEntry(getIdIndex(id));
        dbStream.readEntryHeader();

        constexpr size_t entrySize = EDBSize<Entry>();
        char* entryData = new char[entrySize];
        try {
                dbStream.readEntryData(entryData);
                Entry& entry = EDBDeserializer<Entry>(entryData);
                delete[] entryData;
                return Result<Entry>(entry, id);
        } catch (e) {
                delete[] entryData;
                throw e;
        }
}

template<typename Entry>
void Database<Entry>::put(const Result<Entry>& result)
{
        constexpr entrySize = EDBSize<Entry>();
        const ID entryId = result.id;
        dbStream.seekAtEntry(getIdIndex(entryId));

        dbStream.writeEntryHeader({ entryId });
        char* entryData = new char[entrySize];
        try {
                EDBSerializer<Entry>(result.data, entryData);
                dbStream.writeEntryData(entryData);
                delete[] entryData;
        } catch (e) {
                delete[] entryData;
                throw e;
        }
}

template<typename Entry>
ID Database<Entry>::putNew(const Entry& entry)
{
        constexpr entrySize = EDBSize<Entry>();
        const uint32_t entryCount = getEntryCount();
        const ID last = getEntryId(entryCount - 1);
        const ID newEntryId = last + 1;
        dbStream.seekAtEntry(entryCount);

        dbStream.writeEntryHeader({ newEntryId });
        char* entryData = new char[entrySize];
        try {
                EDBSerializer<Entry>(result.data, entryData);
                dbStream.writeEntryData(entryData);
                setEntryCount(entryCount + 1);
                delete[] entryData;
                return newEntryId;
        } catch (e) {
                delete[] entryData;
                throw e;
        }
}

template<typename Entry>
void Database<Entry>::erase(ID id)
{
        FileStream copyStream(dbStream);
        const uint32_t entryCount = getEntryCount();
        constexpr size_t entrySize = EDBSize<Entry>();
        const uint32_t eraseIdx = getIdIndex(id);
        dbStream.seekAtEntry(eraseIdx);
        copyStream.seekAtEntry(eraseIdx);

        char* entryData = new char[entrySize];
        try {
                copyStream.readEntryHeader();
                copyStream.readEntryData(entryData);
                for (uint32_t copyIdx = eraseIdx + 1; copyIdx < entryCount; ++copyIdx) {
                        dbStream.writeEntryHeader(copyStream.readEntryHeader());
                        copyStream.readEntryData(entryData);
                        dbStream.writeEntryData(entryData);
                }
                delete[] entryData;
        } catch (e) {
                delete[] entryData;
                throw e;
        }

        setEntryCount(entryCount - 1);
}

template<typename Entry>
void Database<Entry>::erase(const Result<Entry>& result)
{
        erase(result.id);
}

template<typename Entry>
void Database<Entry>::erase(const Query<Entry>& query)
{
        FileStream copyStream(dbStream);
        const uint32_t entryCount = getEntryCount();
        constexpr size_t entrySize = EDBSize<Entry>();
        dbStream.seekAtEntry(0);
        copyStream.seekAtEntry(0);

        uint32_t nDeleted = 0;
        char* entryData = new char[entrySize];
        try {
                for (uint32_t copyIdx = 0; copyIdx < entryCount; ++copyIdx) {
                        const EntryHeader entryHeader = copyStream.readEntryHeader();
                        copyStream.readEntryData(entryData);
                        if (query.includes(EDBDeserializer<Entry>(entryData))) {
                                ++nDeleted;
                                continue;
                        }
                        dbStream.writeEntryHeader(entryHeader);
                        dbStream.writeEntryData(entryData);
                }
                delete[] entryData;
        } catch (e) {
                delete[] entryData;
                throw e;
        }

        setEntryCount(entryCount - nDeleted);
}

template<typename Entry>
bool Database<Entry>::iterator::isEntryIdxValid(uint32_t entryIdx)
{
        return entryIdx < db.getEntryCount();
}

template<typename Entry>
typename Database<Entry>::iterator(Database<Entry>& db)
        : db(db), entryIdx(0), ended(false)
{ }

template<typename Entry>
typename Database<Entry>::iterator& Database<Entry>::iterator::operator++()
{
        if (not isEntryIdxValid(++entryIdx))
                ended = true;
        return *this;
}

template<typename Entry>
typename Database<Entry>::iterator Database<Entry>::iterator::operator++(int)
{
        Database<Entry>::iterator prev = *this;
        if (not isEntryIdxValid(++entryIdx))
                ended = true;
        return prev;
}

template<typename Entry>
Result<Entry> Database<Entry>::iterator::operator*()
{
        if (not isEntryIdxValid(entryIdx))
                throw std::runtime_error("Dereferenced invalid iterator");
        db.dbStream.seekAtEntry(entryIdx);

        EntryHeader entryHeader = db.dbStream.readEntryHeader();
        constexpr size_t entrySize = EDBSize<Entry>();
        char* entryData = new char[entrySize];
        try {
                dbStream.readEntryData(entryData);
                Entry& entry = EDBDeserializer<Entry>(entryData);
                delete[] entryData;
                return Result<Entry>(entry, entryHeader.entryId);
        } catch (e) {
                delete[] entryData;
                throw e;
        }
}

template<typename Entry>
bool Database<Entry>::iterator::operator==(const Database<Entry>::iterator& it)
{
        return &db == &it.db and ended == it.ended
                and (ended or entryIdx == it.entryIdx);
}

template<typename Entry>
bool Database<Entry>::iterator::operator!=(const Database<Entry>::iterator& it)
{
        return not operator==(it);
}

template<typename Entry>
Query<Entry> Database<Entry>::query()
{
        return Query<Entry>(*this);
}

}


#undef ENTRY_HDRSZ
#undef DB_HDRSZ


#endif /* EDB_DATABASE */
