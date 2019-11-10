#ifndef EDB
#define EDB


#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <functional>

#include "require.hpp"


using CErrors::require;


typedef const uint8_t EDBData[];

template<typename Entry>
constexpr size_t EDBSize();

template<typename Entry>
void EDBSerializer(const Entry& entry, EDBData& data);

template<typename Entry>
Entry EDBDeserializer(const EDBData& data);


namespace EDB
{
typedef uint32_t ID;


template<typename Entry>
class Result;

template<typename Entry>
class Query;

template<typename Entry>
class Database
{
        typedef struct { uint32_t entryCount, entrySize; } Header;
private:
        FILE* fd;
        bool copied;
        void writeInteger(uint32_t n);
        uint32_t readInteger();
        void writeHeader(const Header& header);
        Header readHeader();
        bool initialized();
        void initialize();
public:
        Database(const char* filename);
        Database(const Database<Entry>& db);
        Database<Entry>& operator=(const Database<Entry>& db);
        ~Database();
        Result<Entry> get(ID id);
        Result<Entry> get(const Query<Entry>& query);
        void put(const Result<Entry>& result);
        void put(const Query<Entry>& query);
        ID putNew(const Entry& entry);
        void erase(ID id);
        void erase(const Result<Entry>& result);
        void erase(const Query<Entry>& query);
        Query<Entry> query();
        Query<Entry> query(const Query<Entry>& query);
};

template<typename Entry>
void Database<Entry>::writeInteger(uint32_t n)
{
        for (int i = 0; i < 4; ++i) {
                require(fputc(n & 0xFF, fd) != EOF);
                n >>= 8;
        }
}

template<typename Entry>
uint32_t Database<Entry>::readInteger()
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
void Database<Entry>::writeHeader(const Database::Header& header)
{
        require(fseek(fd, 0, SEEK_SET) == 0);
        writeInteger(header.entryCount);
        writeInteger(header.entrySize);
}

template<typename Entry>
typename Database<Entry>::Header Database<Entry>::readHeader()
{
        require(fseek(fd, 0, SEEK_SET) == 0);
        const uint32_t entryCount = readInteger();
        const uint32_t entrySize = readInteger();
        return { entryCount, entrySize };
}

template<typename Entry>
bool Database<Entry>::initialized()
{
        long file_sz;
        require(fseek(fd, 0, SEEK_END) == 0);
        require((file_sz = ftell(fd)) >= 0);
        return file_sz > 0;
}

template<typename Entry>
void Database<Entry>::initialize()
{
        const uint32_t entryCount = 0;
        const uint32_t entrySize = EDBSize<Entry>();
        writeHeader({ entryCount, entrySize });
}

template<typename Entry>
Database<Entry>::Database(const char* filename) : copied(false)
{
        require(fd = fopen(filename, "w+"));
        if (not initialized())
                initialize();
}

template<typename Entry>
Database<Entry>::Database(const Database<Entry>& db) : copied(true)
{
        fd = db.fd;
}

template<typename Entry>
Database<Entry>& Database<Entry>::operator=(const Database<Entry>& db)
{
        if (not copied)
                require(fclose(fd) == 0);
        fd = db.fd;
        copied = true;
        return *this;
}

template<typename Entry>
Database<Entry>::~Database()
{
        if (not copied)
                fclose(fd);
}


template<typename T>
class Result
{
private:
public:
};

template<typename Entry>
class Query
{
        typedef std::function<bool(const Entry&)> KeepFn;
        typedef std::function<bool(const Entry&, const Entry&)> OrderFn;
        typedef std::function<void(Entry&)> ModifyFn;
        typedef std::function<void(const Result<Entry>&)> ResultHandler;
private:
        Database<Entry>& db;
public:
        Query(Database<Entry>& db);
        Query(const Query<Entry>& query);
        Query<Entry>& operator=(const Query<Entry>& query);
        ~Query();
        Query<Entry> filter(const KeepFn& keep);
        Query<Entry> sort(const OrderFn& isGreater);
        std::vector<Result<Entry> > get();
        void put();
        void erase();
        void transform(const ModifyFn& modify);
        void forEach(const ResultHandler& resultHandler);
};

template<typename T_src, typename T_dest>
class Migrator
{
private:
public:
};
}


template<> constexpr size_t EDBSize<int>()
{
        return 4;
}


int main(void)
{
        EDB::Database<int> db("helloworld.edb");
        return 0;
}


#endif /* EDB */

// TODO: 
//      - Separate into multiple headers
//      - Decide on vector return or whether to have get at all
//      - Final query operations in one pass?
