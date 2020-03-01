#include "../src/adt/trie/stack.c"
#include "../src/adt/trie/trie.c"
#include "../src/adt/trie/trie.h"

#define WITHOUT_CTEST_NAMESPACE
#include "ctest.h"

#include <stdio.h>


static size_t get_rand_uint()
{
	size_t random;
	FILE* fd = fopen("/dev/urandom", "rbe");
	fread(&random, sizeof random, 1, fd);
	fclose(fd);
	return random;
}


static inline char rand_char(void)
{
	return (char)((get_rand_uint() % 255) + 1); // NOLINT
}


static inline size_t gen_len_bw(size_t min, size_t max)
{
	return (size_t)((get_rand_uint() % (max - min + 1)) + min);
}


static char* gen_rand_str(size_t len)
{
	char* arr = malloc(len + 1);
	for (size_t i = 0; i < len; ++i) {
		arr[i] = rand_char();
	}
	arr[len] = '\0';
	return arr;
}


#if 0
size_t n_freed;

static void increment(void* junk __attribute__((__unused__)))
{
	++n_freed;
}
#endif


static __attribute_used__ void __verbose_free(void* ptr)
{
	printf("About to free: %p\n", ptr), fflush(stdout);
	free(ptr);
}


static __attribute_used__ TrieNode* gen_singleton(test_result_t* res)
{
	char* seg = gen_rand_str(gen_len_bw(1, 10)); // NOLINT
	void* value = malloc(100);                   // NOLINT
	TrieNode* node = node_create(seg, value);
	if (!node) {
		test_check(res, "Node allocation failed", false);
	}
	free(seg);
	return node;
}


static __attribute_used__ void singleton_free(TrieNode* node)
{
	if (!node) {
		return;
	}
	free(node->segment);
	free(node->value);
	free(node->children);
	free(node);
}


static __attribute_used__ bool test_add(char* str1, char* str2, char* strsum)
{
	while (*str1 && *strsum) {
		if (*str1++ != *strsum++) {
			return false;
		}
	}
	while (*str2 && *strsum) {
		if (*str2++ != *strsum++) {
			return false;
		}
	}
	return *str2 == *strsum;
}


static __attribute_used__ bool tries_equal(TrieNode* node1, TrieNode* node2)
{
	if (!node1 || !node2) {
		return !node1 && !node2;
	}
	if (strcmp(node1->segment, node2->segment) != 0) {
		return false;
	}
	if (node1->n_children != node2->n_children) {
		return false;
	}
	size_t n_children = node1->n_children;
	for (size_t i = 0; i < n_children; ++i) {
		if (!tries_equal(&node1->children[i], &node2->children[i])) {
			return false;
		}
	}
	return true;
}


static bool test_compactness_invariant(TrieNode* node)
{
	if (!node->value && node->n_children == 1) {
		return false;
	}
	for (size_t i = 0; i < node->n_children; ++i) {
		if (!test_compactness_invariant(&node->children[i])) {
			return false;
		}
	}
	return true;
}


static __attribute_used__ bool test_compact(Trie* t)
{
	if (t->root->n_children < 1) {
		return true;
	}
	return test_compactness_invariant(&t->root->children[0]);
}


static bool is_prefix(const char* prefix, const char* str)
{
	for (; prefix[0] && str[0]; ++prefix, ++str) {
		if (prefix[0] != str[0]) {
			return false;
		}
	}
	return !prefix[0];
}


static bool lexical_lt(const char* str1, const char* str2)
{
	return str1 ? strcmp(str1, str2) < 0 : true;
}


/* TODO(phymod0): Place ASSERT_CHECKs at the end */


DEFINE_TEST(test_instantiation)
{
	DESCRIBE_TEST("Unit test for trie_create");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(root_proper, "Proper root tree structure");
	DEFINE_CHECK(ops_proper, "Proper trie operations");
	DEFINE_CHECK(keylen_proper, "Proper initial key max length");

	Trie* trie = trie_create(TRIE_OPS_FREE);
	CHECK_INCLUDE(mem_alloc, trie != NULL);
	CHECK_INCLUDE(root_proper, trie->root && trie->root->segment &&
				       trie->root->segment[0] == '\0' &&
				       trie->root->n_children == 0 &&
				       !trie->root->value);
	CHECK_INCLUDE(ops_proper, trie->ops && trie->ops->dtor == free);
	CHECK_INCLUDE(keylen_proper, trie->max_keylen_added == 0);

	trie_destroy(trie);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(root_proper);
	ASSERT_CHECK(ops_proper);
	ASSERT_CHECK(keylen_proper);
}


DEFINE_TEST(asan_test_destroy)
{
	DESCRIBE_TEST("Unit test for trie_destroy");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");

#if 0
	Trie* trie = trie_create(
			&(struct TrieOps){
				.dtor = __verbose_free,
			}
		     );
#endif
	Trie* trie = trie_create(TRIE_OPS_FREE);
	Trie* trie2 = trie_create(TRIE_OPS_NONE);
#if 0
	Trie* trie2 = trie_create(&(struct TrieOps){
		.dtor = increment,
	});
#endif
	CHECK_INCLUDE(mem_alloc, trie != NULL && trie2 != NULL);

	size_t t = gen_len_bw(100, 200); // NOLINT
	while (t-- > 0) {
		char* seg = gen_rand_str(t);
		void* junk = malloc(10); // NOLINT
		CHECK_INCLUDE(mem_alloc, seg != NULL && junk != NULL);
		trie_insert(trie, seg, junk);
		trie_insert(trie2, seg, junk);
		free(seg);
	}

	trie_destroy(trie);
	trie_destroy(trie2);
#if 0
	n_freed = 0;
	test_check(res, "Insert count matches free count", n_freed == T);
#endif

	ASSERT_CHECK(mem_alloc);
}


DEFINE_TEST(test_max_keylen)
{
	DESCRIBE_TEST("Unit test for trie_maxkeylen_added");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(initial_keylen, "Key length initially 0");
	DEFINE_CHECK(consistent_keylen, "Key length is consistently correct");

	Trie* trie = trie_create(TRIE_OPS_FREE);
	CHECK_INCLUDE(mem_alloc, trie != NULL);
	CHECK_INCLUDE(initial_keylen, trie_maxkeylen_added(trie) == 0);

	size_t t = gen_len_bw(100, 200); // NOLINT
	size_t M = 0;
	while (t-- > 0) {
		size_t len = gen_len_bw(100, 200); // NOLINT
		char* seg = gen_rand_str(len);
		CHECK_INCLUDE(mem_alloc, seg != NULL);
		M = len > M ? len : M;
		trie_insert(trie, seg, malloc(1));
		free(seg);
		if (trie_maxkeylen_added(trie) != M) {
			CHECK_INCLUDE(consistent_keylen, false);
			break;
		}
	}

	trie_destroy(trie);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(initial_keylen);
	ASSERT_CHECK(consistent_keylen);
}


DEFINE_TEST(test_node_create)
{
	DESCRIBE_TEST("Test for node creation");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(initial_children, "No initial children");
	DEFINE_CHECK(segment_value_match, "Segment and value match");

	TrieNode* node = gen_singleton(__CTest_result);
	CHECK_INCLUDE(mem_alloc, node != NULL);

	char* seg = str_dup(node->segment);
	void* value = node->value;
	CHECK_INCLUDE(initial_children, node->n_children == 0);

	bool seg_match = node->segment && strcmp(node->segment, seg) == 0;
	bool val_match =
	    node->value && memcmp(node->value, value, 100) == 0; // NOLINT
	CHECK_INCLUDE(segment_value_match, seg_match && val_match);

	singleton_free(node);
	free(seg);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(initial_children);
	ASSERT_CHECK(segment_value_match);
}


DEFINE_TEST(test_insert)
{
	DESCRIBE_TEST("Unit test for trie_insert");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(null_insertion, "Inserting NULL fails with -1");
	DEFINE_CHECK(empty_key_insert, "Empty key insertion succeeds");
	DEFINE_CHECK(trie_structure, "Trie structure is as expected");
	DEFINE_CHECK(trie_segments, "Trie segments are as expected");

#define KEY_INSERT(str1, str2)                                                 \
	(trie_insert(trie, _seg = add_strs((str1), (str2)), malloc(10)),       \
	 free(_seg))

	Trie* trie = trie_create(TRIE_OPS_FREE);
	CHECK_INCLUDE(mem_alloc, trie != NULL);
	CHECK_INCLUDE(null_insertion, trie_insert(trie, "", NULL) == -1);

	void* __v = malloc(1);
	trie_insert(trie, "", __v);
	CHECK_INCLUDE(empty_key_insert, trie->root->value == __v);

	// node1->node2
	// node2->node4
	// node2->node5
	// node1->node3
	// node3->node6
	char* seg1 = gen_rand_str(0);
	char* seg2_1 = gen_rand_str(gen_len_bw(1, 10)); // NOLINT
	seg2_1[0] = 'a';
	char* seg2_2 = gen_rand_str(gen_len_bw(1, 10)); // NOLINT
	char* seg2 = add_strs(seg2_1, seg2_2);
	char* seg3 = gen_rand_str(gen_len_bw(1, 10)); // NOLINT
	seg3[0] = 'z';
	char* seg4 = gen_rand_str(gen_len_bw(1, 10)); // NOLINT
	seg4[0] = 'p';
	char* seg5 = gen_rand_str(gen_len_bw(1, 10)); // NOLINT
	seg5[0] = 'q';
	char* seg6 = gen_rand_str(gen_len_bw(1, 10)); // NOLINT

	char* _seg = NULL;
	KEY_INSERT(seg2, seg4);
	KEY_INSERT(seg2, seg5);
	KEY_INSERT(seg3, "");
	KEY_INSERT(seg3, seg6);

	TrieNode* node1 = trie ? trie->root : NULL;
	TrieNode* node2 =
	    node1 && node1->n_children > 0 ? &node1->children[0] : NULL;
	TrieNode* node4 =
	    node2 && node2->n_children > 0 ? &node2->children[0] : NULL;
	TrieNode* node5 =
	    node2 && node2->n_children > 1 ? &node2->children[1] : NULL;
	TrieNode* node3 =
	    node1 && node1->n_children > 1 ? &node1->children[1] : NULL;
	TrieNode* node6 =
	    node3 && node3->n_children > 0 ? &node3->children[0] : NULL;
	CHECK_INCLUDE(trie_structure,
		      node1 && node2 && node3 && node4 && node5 && node6);
	CHECK_INCLUDE(trie_segments,
		      node1 && strcmp(node1->segment, seg1) == 0 && node2 &&
			  strcmp(node2->segment, seg2) == 0 && node3 &&
			  strcmp(node3->segment, seg3) == 0 && node4 &&
			  strcmp(node4->segment, seg4) == 0 && node5 &&
			  strcmp(node5->segment, seg5) == 0 && node6 &&
			  strcmp(node6->segment, seg6) == 0);

	free(seg1);
	free(seg2_1);
	free(seg2_2);
	free(seg2);
	free(seg3);
	free(seg4);
	free(seg5);
	free(seg6);
	trie_destroy(trie);
#undef KEY_INSERT

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(null_insertion);
	ASSERT_CHECK(empty_key_insert);
	ASSERT_CHECK(trie_structure);
	ASSERT_CHECK(trie_segments);
}


DEFINE_TEST(test_delete)
{
	DESCRIBE_TEST("Unit test for trie_delete");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(add_delete,
		     "Trie stays the same under corresponding add/deletes");
	DEFINE_CHECK(empty_noaffect,
		     "Empty value deletion does not affect structure");
	DEFINE_CHECK(compact,
		     "Trie stays as compact as possible after an operation");

	typedef enum {
		PASS,
		INSERT,
		DELETE,
		BOTH,
	} instruction;

	typedef struct record {
		char* key;
		instruction ins;
	} record;

	size_t n_rec, N = get_rand_uint() & 1 ? 5 : 50; // NOLINT
	record* records = malloc((n_rec = gen_len_bw(3, N)) * sizeof *records);
	CHECK_INCLUDE(mem_alloc, records != NULL);
	if (records == NULL) {
		return;
	}

	for (size_t i = 0; i < n_rec; ++i) {
		records[i].key = gen_rand_str(gen_len_bw(0, N));
		CHECK_INCLUDE(mem_alloc, records[i].key != NULL);
		if (records[i].key == NULL) {
			break;
		}
		for (size_t j = 0; j < i; ++j) {
			if (!strcmp(records[i].key, records[j].key)) {
				free(records[i--].key);
				continue;
			}
		}
	}

	for (size_t iters = 0; iters < N; ++iters) {
		Trie* trie_a = trie_create(TRIE_OPS_FREE);
		Trie* trie_b = trie_create(TRIE_OPS_FREE);
		CHECK_INCLUDE(mem_alloc, trie_a && trie_b);
		for (size_t i = 0; i < n_rec; ++i) {
			records[i].ins = get_rand_uint() % 4;
		}

		for (size_t i = 0; i < n_rec; ++i) {
			char* key = records[i].key; // NOLINT
			switch (records[i].ins) {
			case INSERT:
				trie_insert(trie_a, key, malloc(10)); // NOLINT
				CHECK_INCLUDE(compact, test_compact(trie_a));
				// fallthrough
			case BOTH:
				trie_insert(trie_b, key, malloc(10)); // NOLINT
				CHECK_INCLUDE(compact, test_compact(trie_b));
				// fallthrough
			default:
				break;
			}
		}

		for (size_t i = 0; i < n_rec; ++i) {
			char* key = records[i].key;
			switch (records[i].ins) {
			case DELETE:
				trie_delete(trie_a, key);
				CHECK_INCLUDE(compact, test_compact(trie_a));
				// fallthrough
			case BOTH:
				trie_delete(trie_b, key);
				CHECK_INCLUDE(compact, test_compact(trie_b));
				// fallthrough
			default:
				break;
			}
		}

		trie_delete(trie_a, "");
		CHECK_INCLUDE(compact, test_compact(trie_a));
		trie_delete(trie_b, "");
		CHECK_INCLUDE(compact, test_compact(trie_b));
		CHECK_INCLUDE(
		    empty_noaffect,
		    trie_a->root && trie_a->root->segment[0] == '\0' &&
			trie_b->root && trie_b->root->segment[0] == '\0');

		CHECK_INCLUDE(add_delete,
			      tries_equal(trie_a->root, trie_b->root));
		trie_destroy(trie_a);
		trie_destroy(trie_b);
	}

	for (size_t i = 0; i < n_rec; ++i) {
		free(records[i].key);
	}
	free(records);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(add_delete);
	ASSERT_CHECK(empty_noaffect);
	ASSERT_CHECK(compact);
}


DEFINE_TEST(test_find)
{
	DESCRIBE_TEST("Unit test for trie_find");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(sound, "No false positives when finding");
	DEFINE_CHECK(complete, "No false negatives when finding");

	typedef struct key_val {
		char* key;
		void* val;
		bool ins;
	} key_val_t;

	size_t n_kv = get_rand_uint() & 1 ? gen_len_bw(1, 4)
					  : gen_len_bw(1, 50); // NOLINT
	key_val_t* kv = malloc(n_kv * sizeof kv[0]);
	for (size_t i = 0; i < n_kv; ++i) {
		kv[i].key = gen_rand_str(gen_len_bw(1, 100)); // NOLINT
		CHECK_INCLUDE(mem_alloc, kv[i].key != NULL);
		if (kv[i].key == NULL) {
			break;
		}
		for (size_t j = 0; j < i; ++j) {
			if (strcmp(kv[j].key, kv[i].key) == 0) {
				free(kv[i--].key);
				break;
			}
		}
	}
	for (size_t i = 0; i < n_kv; ++i) {
		kv[i].val = malloc(1);
		CHECK_INCLUDE(mem_alloc, kv[i].val != NULL);
	}

	int n_iters = (int)gen_len_bw(100, 200); // NOLINT
	for (int iter = 0; iter < n_iters; ++iter) {
		Trie* trie = trie_create(TRIE_OPS_NONE);
		for (size_t i = 0; i < n_kv; ++i) {
			if (get_rand_uint() & 1) {
				kv[i].ins = false;
				continue;
			}
			kv[i].ins = true;
			trie_insert(trie, kv[i].key, kv[i].val); // NOLINT
		}
		for (size_t i = 0; i < n_kv; ++i) {
			void* val = trie_find(trie, kv[i].key);
			CHECK_INCLUDE(complete, !kv[i].ins || val == kv[i].val);
			CHECK_INCLUDE(sound, kv[i].ins || !val);
		}
		trie_destroy(trie);
	}

	for (size_t i = 0; i < n_kv; ++i) {
		free(kv[i].key); // NOLINT
		free(kv[i].val);
	}
	free(kv);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(sound);
	ASSERT_CHECK(complete);
}


DEFINE_TEST(test_segncpy)
{
	DESCRIBE_TEST("Unit test for segncpy");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(normal_concat, "Normal concat correct");
	DEFINE_CHECK(partial_concat, "Partial concat correct");

	char* buf = key_buffer_create(15); // NOLINT
	char* oldbuf = buf;
	CHECK_INCLUDE(mem_alloc, buf);

	buf = segncpy(buf, "Hello ", 100); // NOLINT
	CHECK_INCLUDE(normal_concat, strcmp(oldbuf, "Hello ") == 0);
	buf = segncpy(buf, "world!", 100); // NOLINT
	CHECK_INCLUDE(normal_concat, strcmp(oldbuf, "Hello world!") == 0);
	oldbuf[15] = '\0'; // NOLINT
	segncpy(buf, "Additional text", 3);
	CHECK_INCLUDE(partial_concat, strcmp(oldbuf, "Hello world!Add") == 0);

	free(oldbuf);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(normal_concat);
	ASSERT_CHECK(partial_concat);
}


DEFINE_TEST(test_key_add_segment)
{
	DESCRIBE_TEST("Unit test for key_add_segment");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(concat, "Middle concat correct");
	DEFINE_CHECK(oob_concat, "NULL returned on out-of-bounds concat");
	DEFINE_CHECK(last_concat, "Last concat correct");

	char* buf = key_buffer_create(15); // NOLINT
	char* oldbuf = buf;
	CHECK_INCLUDE(mem_alloc, buf != NULL);

	buf = key_add_segment(buf, "Hello ", oldbuf, 15); // NOLINT
	CHECK_INCLUDE(concat, strcmp(oldbuf, "Hello ") == 0);
	buf = key_add_segment(buf, "world!", oldbuf, 15); // NOLINT
	CHECK_INCLUDE(concat, strcmp(oldbuf, "Hello world!") == 0);
	char* invalid_buf =
	    key_add_segment(buf, "Additional", oldbuf, 15); // NOLINT
	CHECK_INCLUDE(oob_concat, !invalid_buf);
	char* valid_buf = key_add_segment(buf, "Add", oldbuf, 15); // NOLINT
	CHECK_INCLUDE(last_concat, strcmp(oldbuf, "Hello world!Add") == 0 &&
				       valid_buf && oldbuf[15] == '\0' &&
				       valid_buf == &oldbuf[15]);

	free(oldbuf);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(concat);
	ASSERT_CHECK(oob_concat);
	ASSERT_CHECK(last_concat);
}


DEFINE_TEST(asan_test_iter_destroy)
{
	DESCRIBE_TEST("Unit test for trie_iter_destroy");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");

	Trie* trie = trie_create(TRIE_OPS_FREE);
	CHECK_INCLUDE(mem_alloc, trie != NULL);
	size_t n_iters = gen_len_bw(100, 200); // NOLINT
	char* pref = NULL;
	for (size_t i = 0; i < n_iters; ++i) {
		char* key = gen_rand_str(gen_len_bw(10, 100)); // NOLINT
		CHECK_INCLUDE(mem_alloc, key != NULL);
		trie_insert(trie, key, malloc(1));
		if (!pref) {
			pref = key, pref[2] = '\0';
		} else {
			free(key);
		}
	}

	TrieIterator* iter = trie_findall(trie, pref, 150); // NOLINT
	CHECK_INCLUDE(mem_alloc, iter != NULL);
	trie_iter_destroy(iter);

	trie_destroy(trie);
	free(pref);

	ASSERT_CHECK(mem_alloc);
}


DEFINE_TEST(test_iterator)
{
	DESCRIBE_TEST("Test for trie iteration");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(sorted, "Iterated over keys in ascending order");
	DEFINE_CHECK(bounded, "Did not iterate over out-of-bound keys");
	DEFINE_CHECK(prefixed, "Only covered keys with the given prefix");
	DEFINE_CHECK(val_correct, "Value seen matched the value inserted");
	DEFINE_CHECK(complete, "Iteration was complete");
	DEFINE_CHECK(sound, "Iteration was sound");

	typedef struct key_val {
		char* key;
		void* val;
		bool ins;
	} key_val_t;

	Trie* trie = trie_create(TRIE_OPS_FREE);
	CHECK_INCLUDE(mem_alloc, trie != NULL);

	size_t N = get_rand_uint() & 1 ? 10 : 100; // NOLINT
	size_t n_kv = gen_len_bw(100, 200);        // NOLINT
	size_t pf_len = get_rand_uint() & 1 ? 0 : 3;
	size_t max_keylen = gen_len_bw(N / 4, 3 * N / 4);
	size_t n_findable = 0;
	size_t n_found = 0;
	key_val_t* kv = malloc(n_kv * sizeof kv[0]);
	CHECK_INCLUDE(mem_alloc, kv != NULL);
	char* prf = NULL;
	for (size_t i = 0; i < n_kv; ++i) {
		kv[i].key = gen_rand_str(gen_len_bw(10, N)); // NOLINT
		kv[i].val = malloc(1);
		CHECK_INCLUDE(mem_alloc,
			      kv[i].key != NULL && kv[i].val != NULL);
		if (!prf) {
			prf = str_dup(kv[i].key), prf[pf_len] = 0;
			continue;
		}
		for (size_t j = 0; j < i; ++j) {
			if (strcmp(kv[j].key, kv[i].key) == 0) {
				free(kv[i].key);
				free(kv[i].val);
				--i;
				break;
			}
		}
	}
	for (size_t i = 0; i < n_kv; ++i) {
		if ((kv[i].ins = get_rand_uint() & 1)) {
			trie_insert(trie, kv[i].key, kv[i].val);
			if (is_prefix(prf, kv[i].key) &&
			    strlen(kv[i].key) <= max_keylen) {
				++n_findable;
			}
		} else {
			free(kv[i].val);
		}
	}

	TrieIterator* iter = trie_findall(trie, prf, max_keylen);
	const char* key = NULL;
	while (iter) {
		char* key_prev = key ? str_dup(key) : NULL;
		const char* key = trie_iter_getkey(iter);
		void* val = trie_iter_getval(iter);
		size_t i;
		for (i = 0; i < n_kv; ++i) {
			if (strcmp(kv[i].key, key) == 0) {
				break;
			}
		}
		if (i == n_kv) {
			CHECK_INCLUDE(sound, false);
			trie_iter_next(&iter);
			free(key_prev);
			continue;
		}
		n_found += is_prefix(prf, key) && strlen(key) <= max_keylen;
		CHECK_INCLUDE(sorted, lexical_lt(key_prev, key));
		CHECK_INCLUDE(bounded, strlen(key) <= max_keylen);
		CHECK_INCLUDE(prefixed, is_prefix(prf, key));
		CHECK_INCLUDE(val_correct, val == kv[i].val);

		trie_iter_next(&iter);
		free(key_prev);
	}
	CHECK_INCLUDE(complete, n_findable == n_found);
	CHECK_INCLUDE(sound, bounded.satisfied && prefixed.satisfied);

	trie_iter_destroy(iter);
	trie_destroy(trie);
	for (size_t i = 0; i < n_kv; ++i) {
		free(kv[i].key);
	}
	free(kv);
	free(prf);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(sorted);
	ASSERT_CHECK(bounded);
	ASSERT_CHECK(prefixed);
	ASSERT_CHECK(val_correct);
	ASSERT_CHECK(complete);
	ASSERT_CHECK(sound);
}


START(test_instantiation, asan_test_destroy, test_max_keylen, test_node_create,
      test_insert, test_delete, test_find, test_segncpy, test_key_add_segment,
      asan_test_iter_destroy, test_iterator)
