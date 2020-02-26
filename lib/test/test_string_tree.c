#define NDEBUG

#include "../src/adt/string_tree/string_tree.c"
#include "../src/adt/trie/stack.c"
#include "../src/adt/trie/trie.c"
#include "../src/adt/trie/trie.h"

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


TEST_DEFINE(test_instantiation, res)
{
	test_name(res, "string tree creation");

	StringTree* string_tree = string_tree_create();
	test_check(res, "Memory allocated", string_tree != NULL);
	test_check(res, "Value is initialized to NULL",
		   string_tree->val == NULL);

	int n_subtrees = string_tree->subtrees->root->n_children;
	test_check(res, "No initial subtrees", n_subtrees == 0);

	string_tree_destroy(string_tree);
}


TEST_DEFINE(test_create_subtree, res)
{
	test_name(res, "subtree creation");

	StringTree* parent = string_tree_create();
	test_check(res, "Parent tree created", parent != NULL);

	StringTree* subtree1 = create_subtree(parent, "hell");
	StringTree* subtree2 = create_subtree(parent, "hello");
	StringTree* subtree3 = create_subtree(parent, "hellboy");
	StringTree* subtree4 = create_subtree(parent, "helpme");
	StringTree* subtree5 = create_subtree(parent, "different");

	test_check(res, "Subtrees created",
		   subtree1 != NULL && subtree2 != NULL && subtree3 != NULL &&
		       subtree4 != NULL && subtree5 != NULL);

	test_check(res, "First subtree exists in parent",
		   trie_find(parent->subtrees, "hell") == subtree1);
	test_check(res, "Second subtree exists in parent",
		   trie_find(parent->subtrees, "hello") == subtree2);
	test_check(res, "Third subtree exists in parent",
		   trie_find(parent->subtrees, "hellboy") == subtree3);
	test_check(res, "Fourth subtree exists in parent",
		   trie_find(parent->subtrees, "helpme") == subtree4);
	test_check(res, "Fifth subtree exists in parent",
		   trie_find(parent->subtrees, "different") == subtree5);

	string_tree_destroy(parent);
}


TEST_DEFINE(asan_test_destroy, res)
{
	test_name(res, "subtree destruction");

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


TEST_DEFINE(test_find_subtree, res)
{
	test_name(res, "subtree retrieval");

#define n_words 16
#define min_word_len 100
#define max_word_len 200
	StringTree* string_tree;
	char* words[n_words];
	StringTree* subtrees[n_words];
	bool found_matches_created = true;

	string_tree = string_tree_create();
	test_check(res, "String tree created", string_tree != NULL);

	for (int i = 0; i < n_words; ++i) {
		int j;
		words[i] = gen_rand_str(gen_len_bw(min_word_len, max_word_len));
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
		for (int j = 0; j <= i; ++j) {
			StringTree* found;
			found = string_tree_find_subtree(string_tree, words[j]);
			found_matches_created &= found == subtrees[j];
		}
	}
	test_check(res, "Found tree matches created", found_matches_created);

	for (int i = 0; i < n_words; ++i) {
		free(words[i]);
	}

	string_tree_destroy(string_tree);
#undef max_word_len
#undef min_word_len
#undef n_words
}


TEST_DEFINE(test_get_subtree, res)
{
	test_name(res, "subtree retrieval with creation");

#define n_words 16
#define min_word_len 100
#define max_word_len 200
	StringTree* string_tree;
	char* words[n_words];
	StringTree* subtrees[n_words];
	bool not_found_before_getting = true;
	bool created_through_get = true;
	bool found_matches_created = true;

	string_tree = string_tree_create();
	test_check(res, "String tree created", string_tree != NULL);

	for (int i = 0; i < n_words; ++i) {
		int j;
		words[i] = gen_rand_str(gen_len_bw(min_word_len, max_word_len));
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
		not_found_before_getting &= subtree == NULL;
		subtree = string_tree_get_subtree(string_tree, words[i]);
		created_through_get &= subtree != NULL;
		subtrees[i] = subtree;
		for (int j = 0; j <= i; ++j) {
			StringTree* found;
			found = string_tree_get_subtree(string_tree, words[j]);
			found_matches_created &= found == subtrees[j];
		}
	}
	test_check(res, "Not found before getting", not_found_before_getting);
	test_check(res, "Subtree created through get", created_through_get);
	test_check(res, "Found tree matches created", found_matches_created);

	for (int i = 0; i < n_words; ++i) {
		free(words[i]);
	}

	string_tree_destroy(string_tree);
#undef max_word_len
#undef min_word_len
#undef n_words
}


TEST_DEFINE(test_get_value, res)
{
	test_name(res, "value getting");

	StringTree* string_tree;
	const char* value;
	const char* retrieved;

	string_tree = string_tree_create();
	test_check(res, "String tree created", string_tree != NULL);

	value = "Arbitrary string";
	string_tree->val = (void*)value;
	retrieved = string_tree_get_value(string_tree);
	test_check(res, "Value retrieved correctly", retrieved == value);

	string_tree_destroy(string_tree);
}


TEST_DEFINE(test_set_value, res)
{
	test_name(res, "value setting");

	StringTree* string_tree;
	const char* value;

	string_tree = string_tree_create();
	test_check(res, "String tree created", string_tree != NULL);

	value = "Arbitrary string";
	string_tree_set_value(string_tree, (void*)value);
	test_check(res, "Value set correctly", string_tree->val == value);

	string_tree_destroy(string_tree);
}


TEST_START(test_instantiation, test_create_subtree, asan_test_destroy,
	   test_find_subtree, test_get_subtree, test_get_value, test_set_value)
