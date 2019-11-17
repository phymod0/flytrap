#ifndef EDB_COMMON
#define EDB_COMMON


#include "utils/require.hpp"

#include <stdint.h>


using CErrors::require;


template<typename Entry>
constexpr size_t EDBSize();

template<typename Entry>
void EDBSerializer(const Entry& entry, char* data);

template<typename Entry>
Entry EDBDeserializer(const char* data);


namespace EDB
{

typedef uint32_t ID;

}


#endif /* EDB_COMMON */
