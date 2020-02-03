/**
 * @file string_tree.h
 * @brief Data type for a tree of strings.
 */


#ifndef STRING_TREE
#define STRING_TREE


/** Trie data structure. */
struct StringTree;
#ifndef STRING_TREE_FWD
#define STRING_TREE_FWD
typedef struct StringTree StringTree;
#endif /* STRING_TREE_FWD */


/**
 * Instantiate a string tree.
 *
 * @returns Allocated string tree structure or NULL if out of memory
 */
StringTree* string_tree_create();

/**
 * Destroy a string tree.
 *
 * This does not destroy any tree values.
 *
 * @param tree String tree
 */
void string_tree_destroy(StringTree* tree);

/**
 * Return the subtree associated to the string <code>node</code>.
 *
 * A reference to the subtree is returned. Any modification to the returned
 * subtree will be reflected in the returned subtree of a subsequent call.
 *
 * @param tree String tree
 * @param node String the desired subtree is associated with
 * @returns Pointer to the desired subtree or NULL if not found
 */
StringTree* string_tree_find_subtree(const StringTree* tree, const char* node);

/**
 * Return or create a subtree associated to the string <code>node</code>.
 *
 * Similar to <code>string_tree_find_subtree</code>, except that a new subtree
 * is associated and returned when not found.
 *
 * A reference to the subtree is returned. Any modification to the returned
 * subtree will be reflected in the returned subtree of a subsequent call.
 *
 * @param tree String tree
 * @param node String the desired subtree is associated with
 * @returns Pointer to the desired subtree or NULL if out of memory
 */
StringTree* string_tree_get_subtree(StringTree* tree, const char* node);

/**
 * Associate the root node of a tree with a pointer value.
 *
 * Associated values will not be destroyed under any exposed method.
 *
 * @param tree String tree
 * @param value Value to insert
 */
void string_tree_set_value(StringTree* tree, void* value);

/**
 * Get the pointer value associated with the root node of a tree.
 *
 * @param tree String tree
 * @returns Associated pointer value or NULL if unassociated
 */
void* string_tree_get_value(StringTree* tree);


#endif /* STRING_TREE */
