#include "avl_tree.h"

// root of the AVL tree
Node * avl_root = NULL;
// custom compare function
int (*compare_func)(const void *, const void *) = NULL;

void assign_compare_func(int (*my_comparator) (const void *, const void *)) {
    compare_func = my_comparator;
}

Node * get_avl_root(void) {
    return avl_root;
}
  
int max(int a, int b) {
    return (a > b) ? a : b;
}
  
int height(Node * node) {
    if(node == NULL) return 0;
    return node->height;
}
  
Node * create_node(void * key) {
    Node * node = (Node*) malloc(sizeof(Node));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}
  
Node * right_rotate(Node * y) {
    Node * x = y->left;
    Node * T2 = x->right;
    // perform rotation
    x->right = y;
    y->left = T2;
    // update heights 
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    // return new root
    return x;
}
  
Node * left_rotate(Node * x) {
    Node * y = x->right;
    Node * T2 = y->left;
    // perform rotation 
    y->left = x;
    x->right = T2;
    // update heights 
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    // return new root
    return y;
}
  
int get_balance(Node * node) {
    if(node == NULL) return 0; 
    return height(node->left) - height(node->right);
}

int insert(void * key) {
    if(find_node(key)) return 0;
    avl_root = insert_helper(avl_root, key);
    return 1;
}

Node * insert_helper(Node * node, void * key) {
    // perform the normal BST insertion
    if(node == NULL) return create_node(key); // empty leaf
    if((*compare_func)(key, node->key) == -1)
        node->left  = insert_helper(node->left, key);
    else if((*compare_func)(key, node->key) == 1)
        node->right = insert_helper(node->right, key);
    else {
        // already exists will return NULL
        return node;
    }
    // update height of this ancestor node
    node->height = max(height(node->left), height(node->right)) + 1; 

    // get balance
    int balance = get_balance(node); 
    // left left case 
    if(balance > 1 && (*compare_func)(key, node->left->key) == -1) return right_rotate(node);
    // right right case
    if (balance < -1 && (*compare_func)(key, node->right->key) == 1) return left_rotate(node);
    // left right case 
    if(balance > 1 && (*compare_func)(key, node->left->key) == 1) {
        node->left =  left_rotate(node->left);
        return right_rotate(node);
    }
    // right left case 
    if(balance < -1 && (*compare_func)(key, node->right->key) == -1) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }
    return node;
}

Node * min_value_node(Node * node) {
    Node * current = node;
    while(current->left != NULL)
        current = current->left;
    return current;
}

void delete(void * key) {
    // if there is no such element in the tree, don't even try to delete it
    if(!key || !find_node(key)) return;
    avl_root = delete_helper(avl_root, key);
}

Node * delete_helper(Node * root, void * key) {
    if(root == NULL) return root;
    if((*compare_func)(key, root->key) == -1) 
        root->left = delete_helper(root->left, key);
    else if((*compare_func)(key, root->key) == 1)
        root->right = delete_helper(root->right, key);
    else {
        if((root->left == NULL) || (root->right == NULL)) {
            Node * temp = root->left ? root->left : root->right;
            if(temp == NULL) {
                temp = root;
                root = NULL;
            }
            else *root = *temp;
            free(temp);
        }
        else {
            Node * temp = min_value_node(root->right);
            root->key = temp->key;
            root->right = delete_helper(root->right, temp->key);
        }
    }
    if(root == NULL) return root;
    root->height = max(height(root->left), height(root->right)) + 1;
    
    // start balancing
    int balance = get_balance(root);
    // left left case 
    if(balance > 1 && get_balance(root->left) >= 0)
        return right_rotate(root); 
    // left right case 
    if(balance > 1 && get_balance(root->left) < 0) {
        root->left =  left_rotate(root->left);
        return right_rotate(root);
    }
    // right right case 
    if (balance < -1 && get_balance(root->right) <= 0) 
        return left_rotate(root); 
    // right left case
    if(balance < -1 && get_balance(root->right) > 0) {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }
    return root;
} 

Node * find_node(void * key) {
    return find_node_helper(avl_root, key);
}

Node * find_node_helper(Node * node, void * key) {
	if(node == NULL) return NULL;
    if((*compare_func)(key, node->key) == -1)
        return find_node_helper(node->left, key);
    else if((*compare_func)(key, node->key) == 1)
        return find_node_helper(node->right, key);
	else return node;
}

void free_tree(Node * root) {
    if(!root) return;
    free_tree(root->left);
    free_tree(root->right);
    //free(root->key);
    free(root);
}

void in_order_get(void * (*my_get_callback)(void *)) {
    in_order_get_helper(avl_root, my_get_callback);
}

void in_order_get_helper(Node * node, void * (*my_get_callback)(void *)) {
    if(node->left)
        in_order_get_helper(node->left, my_get_callback);
    (*my_get_callback)(node->key);
    if(node->right)
        in_order_get_helper(node->right, my_get_callback);
}
