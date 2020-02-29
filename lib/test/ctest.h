#ifndef TEST
#define TEST

#include <stdbool.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#define MAX_CHECKNAMES_PER_UNIT 256
#define N_RUNS_PER_TEST 3
#define PRINT_WIDTH 64

struct test_result;
typedef struct test_result test_result_t;

typedef struct {
	const char* description;
	bool satisfied;
} test_check_t;

typedef void (*test_t)(test_result_t* result);

void test_name(test_result_t* result, const char* name);
void test_acheck(test_result_t* result, bool check);
void test_check(test_result_t* result, const char* name, bool check);
int test_run(const test_t* tests, size_t n_tests, const char* module_name);

#ifdef WITHOUT_CTEST_NAMESPACE
#define DEFINE_CHECK(name, desc)                                               \
	test_check_t name = {.description = (desc), .satisfied = true}
#define CHECK_INCLUDE(name, cond)                                              \
	((name).satisfied = ((name).satisfied && (cond)))
#define ASSERT_CHECK(name)                                                     \
	test_check(__CTest_result, (name).description, (name).satisfied);

#define DEFINE_TEST(name) void name(test_result_t* __CTest_result)
#define DESCRIBE_TEST(desc) test_name(__CTest_result, desc)

#define START(...)                                                             \
	int main(void)                                                         \
	{                                                                      \
		void (*test_fns[])(test_result_t*) = {__VA_ARGS__};            \
		const size_t n_tests = sizeof test_fns / sizeof test_fns[0];   \
		return test_run(test_fns, n_tests, __FILE__);                  \
	}
#else /* WITHOUT_CTEST_NAMESPACE */
#define CTEST_DEFINE_CHECK(name, desc)                                         \
	test_check_t name = {.description = (desc), .satisfied = true}
#define CTEST_CHECK_INCLUDE(name, cond) (name).satisfied &= (cond)
#define CTEST_ASSERT_CHECK(name)                                               \
	test_check(__CTest_result, (name).description, (name).satisfied);

#define CTEST_DEFINE_TEST(name) void name(test_result_t* __CTest_result)
#define CTEST_DESCRIBE_TEST(desc) test_name(__CTest_result, desc)

#define CTEST_START(...)                                                       \
	int main(void)                                                         \
	{                                                                      \
		void (*test_fns[])(test_result_t*) = {__VA_ARGS__};            \
		const size_t n_tests = sizeof test_fns / sizeof test_fns[0];   \
		return test_run(test_fns, n_tests, __FILE__);                  \
	}
#endif /* WITHOUT_CTEST_NAMESPACE */

#endif /* TEST */
