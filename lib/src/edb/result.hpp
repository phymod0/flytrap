#ifndef EDB_RESULT
#define EDB_RESULT


#include "common.hpp"


namespace EDB
{
template <typename Entry> struct Result {
	Entry data;
	ID id;
	Result(Entry entry, ID id) : data(entry), id(id) {}
};
} // namespace EDB


#endif /* EDB_RESULT */
