#ifndef CTEST_UTILS
#define CTEST_UTILS

#include <cstdlib>
#include <ctime>
#include <vector>

template <typename Integer>
std::vector<Integer> getRandomIntegers(unsigned int maxSize)
{
	srand(time(NULL));

	int size = (rand() % maxSize) + 1;

	std::vector<Integer> rands(size);
	for (int i = 0; i < size; ++i) {
		rands[i] = static_cast<Integer>(rand());
	}
	return rands;
}

template <typename Integer>
std::vector<Integer> getRandomNIntegers(unsigned int size)
{
	srand(time(NULL));

	std::vector<Integer> rands(size);
	for (unsigned int i = 0; i < size; ++i) {
		rands[i] = static_cast<Integer>(rand());
	}
	return rands;
}

template <typename Integer> Integer getRandom(Integer _min, Integer _max)
{
	Integer diff = _max - _min + 1;
	Integer R = static_cast<Integer>(rand());

	return (R % diff) + _min;
}

#endif /* CTEST_UTILS */
