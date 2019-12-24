#include "splitIterator.hpp"

#include <stdexcept>


template <char delim>
SplitIterator<delim>::SplitIterator(std::string str) : str(std::move(str))
{
}


template <char delim>
SplitIterator<delim>::iterator::iterator(std::string str, bool ended)
    : str(str),
      startIdx(ended ? std::string::npos : str.find_first_not_of(delim)),
      endIdx(ended ? str.length() : nextEndIdx())
{
}


template <char delim> std::string SplitIterator<delim>::iterator::operator*()
{
	if (startIdx == std::string::npos) {
		throw std::runtime_error("Dereferenced invalid iterator");
	}
	return str.substr(startIdx, endIdx - startIdx);
}


template <char delim>
typename SplitIterator<delim>::iterator&
SplitIterator<delim>::iterator::operator++()
{
	if (startIdx != std::string::npos) {
		startIdx = nextStartIdx();
		endIdx = nextEndIdx();
	}
	return *this;
}


template <char delim>
typename SplitIterator<delim>::iterator
SplitIterator<delim>::iterator::operator++(int)
{
	iterator prev = *this;
	if (startIdx != std::string::npos) {
		startIdx = nextStartIdx();
		endIdx = nextEndIdx();
	}
	return prev;
}


template <char delim>
bool SplitIterator<delim>::iterator::operator==(const iterator& iterator)
{
	return startIdx == iterator.startIdx and endIdx == iterator.endIdx;
}


template <char delim>
bool SplitIterator<delim>::iterator::operator!=(const iterator& iterator)
{
	return startIdx != iterator.startIdx or endIdx != iterator.endIdx;
}


template <char delim>
size_t SplitIterator<delim>::SplitIterator::iterator::nextStartIdx()
{
	return str.find_first_not_of(delim, endIdx);
}


template <char delim>
size_t SplitIterator<delim>::SplitIterator::iterator::nextEndIdx()
{
	return std::min(str.find(delim, startIdx), str.length());
}


template <char delim>
typename SplitIterator<delim>::iterator SplitIterator<delim>::begin()
{
	return iterator(str, false);
}


template <char delim>
typename SplitIterator<delim>::iterator SplitIterator<delim>::end()
{
	return iterator(str, true);
}
