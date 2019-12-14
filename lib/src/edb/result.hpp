#ifndef EDB_RESULT
#define EDB_RESULT


#ifndef INCLUDED_BY_EDB
#error Do not include result.hpp directly
#endif /* INCLUDED_BY_EDB */


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
