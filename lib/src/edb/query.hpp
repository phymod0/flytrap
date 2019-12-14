#ifndef EDB_QUERY
#define EDB_QUERY


#ifndef INCLUDED_BY_EDB
#error Do not include query.hpp directly
#endif /* INCLUDED_BY_EDB */


#include "common.hpp"

#include <functional>
#include <list>


/* TODO:
 *      - Query iterators
 *      - Multi-put
 *      - Transformation
 *      - Function composition instead of lists
 */


namespace EDB
{
template <typename Entry> struct Result;
template <typename Entry> class Database;


template <typename Entry> class Query
{
      public:
	using KeepFn = std::function<bool(const Entry&)>;
	using OrderFn = std::function<int(const Entry&, const Entry&)>;

      private:
	Database<Entry> db;
	std::list<KeepFn> keepers;
	std::list<OrderFn> orders;
	bool keep(const Entry& entry);
	int order(const Entry& left, const Entry& right);

      public:
	explicit Query(Database<Entry> db);

	bool includes(const Entry& entry);

	Query<Entry> filter(KeepFn keep);
	Query<Entry> sort(OrderFn order);

	std::list<Result<Entry>> fetch();
	void erase();
};


template <typename Entry> bool Query<Entry>::keep(const Entry& entry)
{
	for (const KeepFn& keeper : keepers) {
		if (not keeper(entry)) {
			return false;
		}
	}
	return true;
}


template <typename Entry>
int Query<Entry>::order(const Entry& left, const Entry& right)
{
	for (const OrderFn& suborder : orders) {
		int ord = suborder(left, right);
		if (ord == 0) {
			continue;
		}
		return ord;
	}
	return 0;
}


template <typename Entry> Query<Entry>::Query(Database<Entry> db) : db(db) {}


template <typename Entry> bool Query<Entry>::includes(const Entry& entry)
{
	return keep(entry);
}


template <typename Entry> Query<Entry> Query<Entry>::filter(KeepFn keep)
{
	keepers.push_back(keep);
	return *this;
}


template <typename Entry> Query<Entry> Query<Entry>::sort(OrderFn order)
{
	orders.push_front(order);
	return *this;
}


template <typename Entry> std::list<Result<Entry>> Query<Entry>::fetch()
{
	std::list<Result<Entry>> results;

	for (const Result<Entry>& result : db) {
		if (keep(result.data)) {
			results.push_back(result);
		}
	}

	results.sort(
	    [&](const Result<Entry>& left, const Result<Entry>& right) {
		    return order(left.data, right.data) < 0;
	    });

	return results;
}


template <typename Entry> void Query<Entry>::erase() { db.erase(*this); }
} // namespace EDB


#endif /* EDB_QUERY */
