#ifndef EDB_QUERY
#define EDB_QUERY


#include "common.hpp"

#include <vector>
#include <functional>


namespace EDB
{

template<typename Entry>
class Query
{
        typedef std::function<bool(const Entry&)> KeepFn;
        typedef std::function<bool(const Entry&, const Entry&)> OrderFn;
        typedef std::function<void(Entry&)> ModifyFn;
private:
        Database<Entry>& db;
public:
        Query(Database<Entry>& db);
        Query(const Query<Entry>& query);
        Query<Entry>& operator=(const Query<Entry>& query);
        ~Query();
        Query<Entry> filter(const KeepFn& keep);
        Query<Entry> sort(const OrderFn& isGreater);
        std::vector<Result<Entry> > get();
        void erase();
        void transform(const ModifyFn& modify);
};

}


#endif /* EDB_QUERY */
