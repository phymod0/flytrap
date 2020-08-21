/**
 * @file condition_tree.h
 * @brief Data type for condition trees.
 */

#ifndef CONDITION_TREE
#define CONDITION_TREE


/** Condition type */
typedef void* Condition;

/** Condition type */
typedef void* ConstCondition;

/** Tree node representing a condition, conjunction or disjunction */
struct ConditionTreeNode;
typedef struct ConditionTreeNode ConditionTreeNode;

/** Condition tree */
struct ConditionTree;
typedef struct ConditionTree ConditionTree;

/** Tree node junction types */
typedef enum {
	CONDTREE_CONJUNCTION,
	CONDTREE_DISJUNCTION,
	CONDTREE_TERMINAL,
} ConditionTreeJunctionType;

/** Operations on vector elements */
typedef struct {

	/** Destructor for the object pointed to by `C` */
	void (*dtor)(Condition C);

	/** Display the object pointed to by `C` */
	void (*print)(ConstCondition C);

	/**
	 * Return a pointer to a copy of the object pointed to by `C`.
	 *
	 * `NULL` must only be returned to indicate a memory allocation failure.
	 */
	Condition (*copy)(ConstCondition C);

} ConditionOps;


/**
 * Instantiate a condition tree.
 *
 * `ops` will be copied rather than borrowed. Operations can safely be passed as
 * `NULL` pointers as an alternative to their respective default implementations
 * (harmless no-ops for `dtor`/`print` and the identity function for `copy`).
 * `ops` itself can be `NULL` as an alternative to passing all operations as
 * `NULL` pointers.
 *
 * @param ops Pointer to operations
 * @return Pointer to an empty condition tree or `NULL` if out of memory
 */
ConditionTree* condition_tree_create(const ConditionOps* ops);

/**
 * Destroy a condition tree.
 *
 * `dtor`, if provided to `condition_tree_create`, will be called exactly once
 * on each condition used in the condition tree. Calling this function on `NULL`
 * is a harmless no-op.
 * @see condition_tree_create
 *
 * @param condition_tree Pointer returned by `condition_tree_create` or `NULL`
 */
void condition_tree_destroy(ConditionTree* conditionTree);

/*
 * TODO(phymod0):
 *    - Create terminal
 *    - Construct conjunction
 *    - Construct disjunction
 *    - Invert
 *    - Normalize
 *    - Fold/reduce
 *    - Print
 */


#endif /* CONDITION_TREE */
