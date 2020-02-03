#include <stdlib.h>

#include "../../utils/logger.h"
#include "../trie/trie.h"
#include "string_tree.h"


struct StringTree {
	Trie* subtrees;
	void* val;
};
#ifndef STRING_TREE_FWD
#define STRING_TREE_FWD
typedef struct StringTree StringTree;
#endif /* STRING_TREE_FWD */


StringTree* string_tree_create()
{
	LOGGER_DEBUG("Creating string tree");
	StringTree* tree = malloc(sizeof *tree);
	Trie* subtrees = trie_create(TRIE_OPS_NONE);

	if (!tree || !subtrees) {
		LOGGER_ERROR("Failed to allocate memory");
		free(tree);
		free(subtrees);
		return NULL;
	}

	tree->subtrees = subtrees;
	tree->val = NULL;
	return tree;
}


void string_tree_destroy(StringTree* tree)
{
	LOGGER_DEBUG("Destroying string tree");
	if (tree) {
		trie_destroy(tree->subtrees);
		free(tree);
	}
}


StringTree* string_tree_find_subtree(const StringTree* tree, const char* node)
{
	LOGGER_DEBUG("Finding the subtree for string %s", node);
	StringTree* found = trie_find(tree->subtrees, (char*)node);
	if (!found) {
		LOGGER_DEBUG("Subtree for string %s not found", node);
	}
	return found;
}


static StringTree* create_subtree(StringTree* tree, const char* node)
{
	LOGGER_DEBUG("Creating a new subtree for string %s", node);
	StringTree* new_subtree = string_tree_create();
	if (trie_insert(tree->subtrees, (char*)node, new_subtree) < 0) {
		LOGGER_ERROR("Subtree creation failed");
		string_tree_destroy(new_subtree);
		return NULL;
	}
	if (!new_subtree) {
		LOGGER_ERROR("Subtree creation failed");
	}
	return new_subtree;
}


StringTree* string_tree_get_subtree(StringTree* tree, const char* node)
{
	LOGGER_DEBUG("Getting the subtree for string %s", node);
	StringTree* subtree = string_tree_find_subtree(tree, node);
	if (subtree) {
		LOGGER_DEBUG("Subtree for %s already exists", node);
		return subtree;
	}
	LOGGER_DEBUG("Creating a new subtree");
	subtree = create_subtree(tree, node);
	if (!subtree) {
		LOGGER_ERROR("Subtree creation failed");
	}
	return subtree;
}


void string_tree_set_value(StringTree* tree, void* value)
{
	LOGGER_DEBUG("Setting the root value to %p", value);
	tree->val = value;
}


void* string_tree_get_value(StringTree* tree)
{
	LOGGER_DEBUG("Got the root value: %p", tree->val);
	return tree->val;
}
