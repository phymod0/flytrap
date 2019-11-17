#ifndef EDB
#define EDB


#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <functional>


namespace EDB
{

template<typename Entry>
struct Result
{
        ID id;
        Entry& data;
        Result(Entry& entry, ID id) : data(entry), id(id) { }
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
//      - Database iterator
//      - Query iterator
