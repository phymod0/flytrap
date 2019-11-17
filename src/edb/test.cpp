#include <iostream>
#include <string>

#include "edb.hpp"


template<> constexpr size_t EDBSize<int>()
{
        return 4;
}


int main(void)
{
        EDB::Database<int> db("helloworld.edb");
        return 0;
}
