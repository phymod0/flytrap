#ifndef UTILS_SPLIT_ITERATOR
#define UTILS_SPLIT_ITERATOR


#include <string>


template <char delim> class SplitIterator
{
      public:
	SplitIterator(std::string str);
	class iterator
	{
	      public:
		iterator(std::string str, bool ended);
		std::string operator*();
		iterator& operator++();
		iterator operator++(int);
		bool operator==(const iterator& iterator);
		bool operator!=(const iterator& iterator);

	      private:
		std::string str;
		size_t startIdx;
		size_t endIdx;
		size_t nextStartIdx();
		size_t nextEndIdx();
	};
	iterator begin();
	iterator end();

      private:
	std::string str;
};


#endif /* UTILS_SPLIT_ITERATOR */
