#include "trie.h"
#include "trie.hpp"
#include <utility>


using std::pair;


static char* _Strdup(const char* str)
{
	size_t len = strlen(str);
	char* result = new char[len + 1];
	memcpy(result, str, len + 1);
	return result;
}


template <typename T> static void _Generic_destroy(void* objPtr)
{
	delete static_cast<T*>(objPtr);
}


template <typename T> trie<T>::iterator::~iterator()
{
	trie_iter_destroy(iter);
}


template <typename T>
typename trie<T>::iterator& trie<T>::iterator::operator++()
{
	trie_iter_next(&iter);
	return *this;
}


template <typename T> pair<const char*, T&> trie<T>::iterator::operator*() const
{
	const char* key = trie_iter_getkey(iter);
	void* val = trie_iter_getval(iter);
	return pair<const char*, T&>(key, *static_cast<T*>(val));
}


template <typename T>
bool trie<T>::iterator::operator==(const iterator& it) const
{
	return iter == it.iter;
}


template <typename T>
bool trie<T>::iterator::operator!=(const iterator& it) const
{
	return iter != it.iter;
}


template <typename T>
trie<T>::trie() : t(trie_create(trie_makeops(_Generic_destroy<T>, NULL)))
{
}


template <typename T> trie<T>::~trie() { trie_destroy(t); }


template <typename T> size_t trie<T>::maxKeylenAdded()
{
	return trie_maxkeylen_added(t);
}


template <typename T> void trie<T>::insert(const char* key, const T& value)
{
	T* val = new T(value);
	trie_insert(t, (char*)key, (void*)val);
}


template <typename T> void trie<T>::remove(const char* key)
{
	trie_delete(t, (char*)key);
}


template <typename T> typename trie<T>::iterator trie<T>::find(const char* key)
{
	iterator it;
	it.iter = trie_findall(t, (char*)key, strlen(key));
	return it;
}


template <typename T> T& trie<T>::operator[](const char* key)
{
	T* found = static_cast<T*>(trie_find(t, (char*)key));
	if (not found) {
		T* newVal = new T();
		trie_insert(t, (char*)key, (void*)newVal);
		return *newVal;
	}
	return *found;
}


template <typename T> typename trie<T>::iterator trie<T>::begin()
{
	iterator it;
	it.iter = trie_findall(t, "", maxKeylenAdded());
	return it;
}


template <typename T> typename trie<T>::iterator trie<T>::end()
{
	iterator it;
	it.iter = nullptr;
	return it;
}


template <typename T>
subtrie<T> trie<T>::getSubtrie(const char* prefix, size_t maxKeylen)
{
	return subtrie<T>(t, prefix, maxKeylen);
}


template <typename T> subtrie<T>::iterator::~iterator()
{
	trie_iter_destroy(iter);
}


template <typename T>
typename subtrie<T>::iterator& subtrie<T>::iterator::operator++()
{
	trie_iter_next(&iter);
	return *this;
}


template <typename T>
pair<const char*, T&> subtrie<T>::iterator::operator*() const
{
	const char* key = trie_iter_getkey(iter);
	void* val = trie_iter_getval(iter);
	return pair<const char*, T&>(key, *static_cast<T*>(val));
}


template <typename T>
bool subtrie<T>::iterator::operator==(const iterator& it) const
{
	return iter == it.iter;
}


template <typename T>
bool subtrie<T>::iterator::operator!=(const iterator& it) const
{
	return iter != it.iter;
}


template <typename T>
subtrie<T>::subtrie(Trie* t, const char* prefix, size_t maxKeylen)
    : t(t), prefix(_Strdup(prefix)), maxKeylen(maxKeylen)
{
}


template <typename T> subtrie<T>::~subtrie() { delete[] prefix; }


template <typename T> typename subtrie<T>::iterator subtrie<T>::begin()
{
	iterator it;
	it.iter = trie_findall(t, prefix, maxKeylen);
	return it;
}


template <typename T> typename subtrie<T>::iterator subtrie<T>::end()
{
	iterator it;
	it.iter = nullptr;
	return it;
}
