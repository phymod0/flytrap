#define NDEBUG

#include "../src/adt/string_tree/string_tree.c"
#include "../src/adt/trie/stack.c"
#include "../src/adt/trie/trie.c"
#include "../src/adt/trie/trie.h"

#define WITHOUT_CTEST_NAMESPACE
#include "ctest.h"

#include <stdio.h>
#include <stdlib.h>


static size_t get_rand_uint()
{
	size_t random;
	FILE* fd = fopen("/dev/urandom", "rbe");
	fread(&random, sizeof random, 1, fd);
	fclose(fd);
	return random;
}


static inline size_t gen_len_bw(size_t min, size_t max)
{
	return (size_t)((get_rand_uint() % (max - min + 1)) + min);
}

static char* gen_rand_str(size_t len)
{
	char* arr = malloc(len + 1);
	FILE* fd = fopen("/dev/urandom", "rbe");
	fread(arr, 1, len, fd);
	arr[len] = '\0';
	fclose(fd);
	return arr;
}


DEFINE_TEST(test_instantiation)
{
	DESCRIBE_TEST("Unit test for string_tree_create");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(val_init, "Value is initialized to NULL");
	DEFINE_CHECK(no_subtrees, "No initial subtrees");

	StringTree* string_tree = string_tree_create();
	CHECK_INCLUDE(mem_alloc, string_tree != NULL);
	CHECK_INCLUDE(val_init, string_tree->val == NULL);

	int n_subtrees = string_tree->subtrees->root->n_children;
	CHECK_INCLUDE(no_subtrees, n_subtrees == 0);

	string_tree_destroy(string_tree);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(val_init);
	ASSERT_CHECK(no_subtrees);
}


DEFINE_TEST(test_create_subtree)
{
	DESCRIBE_TEST("Unit test for create_subtree");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(subtrees_exist_in_parent,
		     "Inserted subtrees exist in parent tree");

	StringTree* parent = string_tree_create();
	CHECK_INCLUDE(mem_alloc, parent != NULL);

	StringTree* subtree1 = create_subtree(parent, "hell");
	CHECK_INCLUDE(mem_alloc, subtree1 != NULL);
	CHECK_INCLUDE(subtrees_exist_in_parent,
		      trie_find(parent->subtrees, "hell") == subtree1);

	StringTree* subtree2 = create_subtree(parent, "hello");
	CHECK_INCLUDE(mem_alloc, subtree2 != NULL);
	CHECK_INCLUDE(subtrees_exist_in_parent,
		      trie_find(parent->subtrees, "hello") == subtree2);

	StringTree* subtree3 = create_subtree(parent, "hellboy");
	CHECK_INCLUDE(mem_alloc, subtree3 != NULL);
	CHECK_INCLUDE(subtrees_exist_in_parent,
		      trie_find(parent->subtrees, "hellboy") == subtree3);

	StringTree* subtree4 = create_subtree(parent, "helpme");
	CHECK_INCLUDE(mem_alloc, subtree4 != NULL);
	CHECK_INCLUDE(subtrees_exist_in_parent,
		      trie_find(parent->subtrees, "helpme") == subtree4);

	StringTree* subtree5 = create_subtree(parent, "different");
	CHECK_INCLUDE(mem_alloc, subtree5 != NULL);
	CHECK_INCLUDE(subtrees_exist_in_parent,
		      trie_find(parent->subtrees, "different") == subtree5);

	string_tree_destroy(parent);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(subtrees_exist_in_parent);
}


DEFINE_TEST(asan_test_destroy)
{
	DESCRIBE_TEST("Unit test for string_tree_destroy");

	const char* words[] = {"hell", "hello", "hellboy", "helpme",
			       "different"};
	const size_t n_words = sizeof words / sizeof words[0];

	for (size_t i = 0; i <= n_words; ++i) {
		StringTree* string_tree = string_tree_create();
		for (size_t j = 0; j < i; ++j) {
			StringTree* subtree;
			subtree = create_subtree(string_tree, words[j]);
			create_subtree(subtree, words[i - j - 1]);
		}
		string_tree_destroy(string_tree);
	}
}


DEFINE_TEST(test_find_subtree)
{
	DESCRIBE_TEST("Unit test for string_tree_find_subtree");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(found_matches_created, "Expected subtrees are found");

#define n_words 16
#define min_word_len 100
#define max_word_len 200
	StringTree* string_tree;
	char* words[n_words];
	StringTree* subtrees[n_words];

	string_tree = string_tree_create();
	CHECK_INCLUDE(mem_alloc, string_tree != NULL);

	for (int i = 0; i < n_words; ++i) {
		int j;
		words[i] = gen_rand_str(gen_len_bw(min_word_len, max_word_len));
		CHECK_INCLUDE(mem_alloc, words[i] != NULL);
		for (j = 0; j < i; ++j) {
			if (strcmp(words[j], words[i]) == 0) {
				break;
			}
		}
		if (j < i) {
			--i;
		}
	}

	for (int i = 0; i < n_words; ++i) {
		subtrees[i] = create_subtree(string_tree, words[i]);
		CHECK_INCLUDE(mem_alloc, subtrees[i] != NULL);
		for (int j = 0; j <= i; ++j) {
			StringTree* found;
			found = string_tree_find_subtree(string_tree, words[j]);
			CHECK_INCLUDE(found_matches_created,
				      found == subtrees[j]);
		}
	}

	for (int i = 0; i < n_words; ++i) {
		free(words[i]);
	}

	string_tree_destroy(string_tree);
#undef max_word_len
#undef min_word_len
#undef n_words

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(found_matches_created);
}


DEFINE_TEST(test_get_subtree)
{
	DESCRIBE_TEST("Unit test for string_tree_get_subtree");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(not_found_before_getting, "Not found before getting");
	DEFINE_CHECK(created_through_get, "Subtree created through get");
	DEFINE_CHECK(found_matches_created, "Found subtree matches created");

#define n_words 16
#define min_word_len 100
#define max_word_len 200
	StringTree* string_tree;
	char* words[n_words];
	StringTree* subtrees[n_words];

	string_tree = string_tree_create();
	CHECK_INCLUDE(mem_alloc, string_tree != NULL);

	for (int i = 0; i < n_words; ++i) {
		int j;
		words[i] = gen_rand_str(gen_len_bw(min_word_len, max_word_len));
		CHECK_INCLUDE(mem_alloc, words[i] != NULL);
		for (j = 0; j < i; ++j) {
			if (strcmp(words[j], words[i]) == 0) {
				break;
			}
		}
		if (j < i) {
			--i;
		}
	}

	for (int i = 0; i < n_words; ++i) {
		StringTree* subtree;
		subtree = string_tree_find_subtree(string_tree, words[i]);
		CHECK_INCLUDE(not_found_before_getting, subtree == NULL);
		subtree = string_tree_get_subtree(string_tree, words[i]);
		CHECK_INCLUDE(mem_alloc, subtree != NULL);
		CHECK_INCLUDE(created_through_get, subtree != NULL);
		subtrees[i] = subtree;
		for (int j = 0; j <= i; ++j) {
			StringTree* found;
			found = string_tree_get_subtree(string_tree, words[j]);
			CHECK_INCLUDE(found_matches_created,
				      found == subtrees[j]);
		}
	}

	for (int i = 0; i < n_words; ++i) {
		free(words[i]);
	}

	string_tree_destroy(string_tree);
#undef max_word_len
#undef min_word_len
#undef n_words

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(not_found_before_getting);
	ASSERT_CHECK(created_through_get);
	ASSERT_CHECK(found_matches_created);
}


DEFINE_TEST(test_get_value)
{
	DESCRIBE_TEST("Unit test for string_tree_get_value");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(correct_retrieved, "The correct value was retrieved");

	StringTree* string_tree;
	const char* value;
	const char* retrieved;

	string_tree = string_tree_create();
	CHECK_INCLUDE(mem_alloc, string_tree != NULL);

	value = "Arbitrary string";
	string_tree->val = (void*)value;
	retrieved = string_tree_get_value(string_tree);
	CHECK_INCLUDE(correct_retrieved, retrieved == value);

	string_tree_destroy(string_tree);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(correct_retrieved);
}


DEFINE_TEST(test_set_value)
{
	DESCRIBE_TEST("Unit test for string_tree_set_value");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(correct_set, "The correct value was set");

	StringTree* string_tree;
	const char* value;

	string_tree = string_tree_create();
	CHECK_INCLUDE(mem_alloc, string_tree != NULL);

	value = "Arbitrary string";
	string_tree_set_value(string_tree, (void*)value);
	CHECK_INCLUDE(correct_set, string_tree->val == value);

	string_tree_destroy(string_tree);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(correct_set);
}


START(test_instantiation, test_create_subtree, asan_test_destroy,
      test_find_subtree, test_get_subtree, test_get_value, test_set_value)
