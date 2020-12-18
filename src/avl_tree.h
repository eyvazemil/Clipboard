#include <stdio.h>
#include <stdlib.h>

// AVL tree node 
typedef struct _node {
    void * key;
    struct _node * left;
    struct _node * right;
    int height;
} Node;

// get the root of the AVL tree
Node * get_avl_root(void);
// assign custom compare function
void assign_compare_func(int (*my_comparator)(const void *, const void *));
// get maximum of two integers 
static int max(int, int);
// get the height of the tree
static int height(Node *);
static Node * create_node(void *);
// right rotate 
static Node * right_rotate(Node *);
// left rotate
static Node * left_rotate(Node *);
// get Balance factor of node 
static int get_balance(Node *);
// insert and balance the tree(if needed)
/* takes already allocated key and assigns it to the new created node */
/* frees that key, if it is already present in the tree */
/* return 1 if insertion was successful and 0, if such a key was already in the tree */
int insert(void *);
static Node * insert_helper(Node *, void *);
// find the node with the minimum value
static Node * min_value_node(Node *);
// delete a node from the tree
/* takes already allocated key, but, does not delete it */
/* the key should be deleted manually by calling find_node before deletion and freeing it after */
void delete(void *);
static Node * delete_helper(Node *, void *);
// find a node(if (exists) return 1; else return 0)
Node * find_node(void *);
static Node * find_node_helper(Node *, void *);
// free the tree
void free_tree(Node *);
// get AVL tree root
Node * get_avl_root(void);
// get keys of the tree in-order
void in_order_get(void * (*my_get_callback)(void *));
static void in_order_get_helper(Node *, void * (*my_get_callback)(void *));
