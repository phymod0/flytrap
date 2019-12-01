#include <iostream>
#include <string>

#include "edb.hpp"


struct HelloWorld {
        char letter1, letter2;
        int num1, num2, num3;
        char greeting[5];
};


/* TODO: implement serializer class */


template<> void EDBSerializer<HelloWorld>(const HelloWorld& hw, char* data)
{
}


template<> HelloWorld EDBDeserializer<HelloWorld>(const char* data)
{
}


template<> constexpr size_t EDBSize<HelloWorld>()
{
        return sizeof(char)
                + sizeof(char)
                + sizeof(int)
                + sizeof(int)
                + sizeof(int)
                + 5*sizeof(char);
}


int main(void)
{
        EDB::Database<int> db("HelloWorld.edb");

        return 0;
}
