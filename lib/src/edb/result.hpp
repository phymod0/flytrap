#ifndef EDB_RESULT
#define EDB_RESULT

#include "common.hpp"

namespace EDB
{

template <typename Entry> struct Result {
	ID id;
	Entry& data;
	Result(Entry& entry, ID id) : data(entry), id(id) {}
};

} // namespace EDB

#endif /* EDB_RESULT */
