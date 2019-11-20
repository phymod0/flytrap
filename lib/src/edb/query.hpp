#ifndef EDB_QUERY
#define EDB_QUERY


#include "common.hpp"

#include <list>
#include <functional>

/* TODO:
 *      - keep and order functions
 *      - Query iterator
 *      - DB iterator begin() and end() methods
 */


namespace EDB
{

template<typename Entry>
class Query
{
public:
        typedef std::function<bool(const Entry&)> KeepFn;
        typedef std::function<int(const Entry&, const Entry&)> OrderFn;
        typedef std::function<void(Entry&)> TransformFn;
private:
        Database<Entry>& db;
        std::list<const KeepFn&> filters;
        std::list<const OrderFn&> orders;
        bool keep(const Entry& entry);
        int order(const Entry& left, const Entry& right);
public:
        Query(Database<Entry>& db);

        bool includes(const Entry& entry);

        Query<Entry> filter(const KeepFn& keep);
        Query<Entry> sort(const OrderFn& order);

        std::list< Result<Entry> > fetch();
        void erase();
        void transform(const TransformFn& modify);
};

template<typename Entry>
bool Query<Entry>::keep(const Entry& entry)
{
        // TODO
}

template<typename Entry>
int Query<Entry>::order(const Entry& left, const Entry& right)
{
        // TODO
}

template<typename Entry>
Query<Entry>::Query(Database<Entry>& db)
        : db(db)
{ }

template<typename Entry>
bool Query<Entry>::includes(const Entry& entry)
{
        return keep(entry);
}

template<typename Entry>
Query<Entry> Query<Entry>::filter(const KeepFn& keep)
{
        filters.push_back(keep);
}

template<typename Entry>
Query<Entry> Query<Entry>::sort(const OrderFn& order)
{
        orders.push_front(order);
}

template<typename Entry>
std::list< Result<Entry> > Query<Entry>::fetch()
{
        std::list< Result<Entry> > results;

        for (const Result<Entry>& result : db)
                if (keep(result.data)) results.push_back(result);

        results.sort([](const Result<Entry>& left, const Result<Entry>& right) {
                return order(left.data, right.data);
        });

        return results;
}

template<typename Entry>
void Query<Entry>::erase()
{
        db.erase(*this);
}

}


#endif /* EDB_QUERY */
