#ifndef EDB_COMMON
#define EDB_COMMON

#include "utils/require.hpp"

#include <cstdint>

template <typename Entry> constexpr size_t EDBSize();
template <typename Entry> void EDBSerializer(const Entry& entry, char* data);
template <typename Entry> Entry EDBDeserializer(const char* data);

namespace EDB
{

using ID = uint32_t;

} // namespace EDB

#endif /* EDB_COMMON */
