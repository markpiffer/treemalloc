
/*
  |                   v     |
  |                     x   |
  |        x  |          x  |
  |    x|   x |x    |x      |
  | x|x |x |  |  |x |  x|x  |
*/
#include "treemalloc.h"



tree_node_t*
calc_middle(tree_node_t *left, tree_node_t *right)
{
  return left + (((ptrdiff_t)right - (ptrdiff_t)left) / sizeof(tree_node_t) + 1) / 2;
}


tree_node_t *
unlink_leaf(tree_node_t *root)
{
  // Take a leaf out of the tree. There is just a recursive descent
  // (no search) involved and the leaf may come from anywhere in the subtree.
  if (root->left != NULL) {
    tree_node_t *unlinked_node = unlink_leaf(root->left);
    if (root->left == unlinked_node) {
      root->left = NULL;
    }
    return unlinked_node; 
  } else if (root->right != NULL) {
    tree_node_t *unlinked_node = unlink_leaf(root->right);
    if (root->right == unlinked_node) {
      root->right = NULL;
    }
    return unlinked_node;
  }
  return root;
}

/* void */
/* kill_left(tree_node_t* root) */
/* { */
/*   tree_node_t* new_left = unlink_leaf(root->left); // retrieve a leaf from left subtree */
/*   if (root->left == new_left) { // kill_node was the only node in subtree */
/*     root->left = NULL; // done */
/*   } */
/*   else { // link the leaf as new intra-tree node */
/*     new_left->left = root->left->left; */
/*     new_left->right = root->left->right; */
/*     root->left = new_left; */
/*   } */
/* } */

/* void */
/* kill_right(tree_node_t* root) */
/* { */
/*   tree_node_t* new_right = unlink_leaf(root->right); // retrieve a leaf from right subtree */
/*   if (root->right == new_right) { // kill_node was the only node in subtree */
/*     root->right = NULL; // done */
/*   } */
/*   else { // link the leaf as new intra-tree node */
/*     new_right->left = root->right->left; */
/*     new_right->right = root->right->right; */
/*     root->right = new_right; */
/*   } */
/* } */

static
void
delete_self(tree_node_t **parent_ptr)
{
  tree_node_t* intra_node = unlink_leaf(*parent_ptr); // retrieve a leaf from subtree
  if (*parent_ptr == intra_node) { // node was the only node in subtree
    *parent_ptr = NULL; // done
  }
  else { // link the leaf as new intra-tree node
    intra_node->left = (*parent_ptr)->left;
    intra_node->right = (*parent_ptr)->right;
    *parent_ptr = intra_node;
  }
}

static
int // returns node_addr->size if node_addr was removed
// or 0 if it was not found in the tree
delete_descendant(tree_node_t *root,
		       tree_node_t *node_addr)
{
  if (root->left == node_addr) {
    int size = node_addr->size;
    delete_self(&root->left);
    return size;
  }
  else if (root->right == node_addr) {
    int size = node_addr->size;
    delete_self(&root->right);
    return size;
  }
  else {
    return 0;
  }
}

static
int // returns size of node_addr if node_addr was removed
// or 0 if it was not found in the tree
delete_node(tree_node_t *root,
		 tree_node_t *from,
		 tree_node_t *to,
		 tree_node_t *node_addr)
{
  if (root == NULL)
    return 0;

  int size = delete_descendant(root, node_addr);
  if (size > 0) {
    return size;
  }
  else {
    // node_addr is not a direct descendant of root
    // select the subtree where node_addr resides:
    tree_node_t *middle = calc_middle(from, to);
    if (node_addr < middle) {
      return delete_node(root->left, from, middle, node_addr);
    }
    else {
      return delete_node(root->right, middle, to, node_addr);
    }
  }
}


static
tree_node_t* // some_node: this is the predecessor
// NULL: no predecessor in this subtree
find_predecessor( // find the predecessor node so that we can integrate into it
	       tree_node_t* root,
	       tree_node_t* from,
	       tree_node_t* to,
	       tree_node_t* node,
	       tree_node_t** hurdle)
{
  if (root == NULL)
    return NULL;
  
  if (root + root->size == node)
    return root;

  if (root > *hurdle && root < node)
    *hurdle = root;
  
  tree_node_t* middle = calc_middle(from, to);
  
  if (middle < node) {
    // node is in right half, there could be a node in right subtree predecessing it
    tree_node_t* pred = find_predecessor(root->right, middle, to, node, hurdle);
    if (pred != NULL)
      return pred;
  }
  // it only makes sense to go left if there wasn't a better predecessor yet
  if (*hurdle < middle)
    return find_predecessor(root->left, from, middle, node, hurdle);

  return NULL;
}

static
void
insert_node(tree_node_t* root,
	    tree_node_t* from,
	    tree_node_t* to,
	    tree_node_t* node)
{
  tree_node_t* middle = calc_middle(from, to);
  if (node < middle) 
    if (root->left == NULL)
      root->left = node;
    else
      insert_node(root->left, from, middle, node);
  else
    if (root->right == NULL)
      root->right = node;
    else
      insert_node(root->right, middle, to, node);
}


static
tree_node_t*
retrieve_node(tree_node_t **root,
	      int size)
{
  if (*root == NULL)
    return NULL;
  else if ((*root)->size >= size) {
    tree_node_t *alloc = (*root) + (*root)->size - size;
    (*root)->size -= size;
    if ((*root)->size == 0) // whole node was taken
      delete_self(root);
    return alloc;
  }
  else {
    tree_node_t *alloc = retrieve_node(&((*root)->left), size); 
    if (alloc == NULL)
      alloc = retrieve_node(&((*root)->right), size);
    return alloc;
  }
}

void
tree_merge_back(tree_node_t* heap,
		int heap_size,
		tree_node_t* node,
		int size)
{
  // This function expects a heap with a special node at address 0
  // with size 0 and the usual right/left children pointers as tree
  // root.
  
  // first step: try to delete the node immediately following this
  // node and merge its size into this node:
  size += delete_node(heap, heap, heap+heap_size, node+size);
  // now look for a predecessor to which this node is immediately
  // adjacent:
  tree_node_t* hurdle = heap; // helper marker for pruning of nonmatching trees
  tree_node_t* pred = find_predecessor(heap, heap, heap+heap_size, node, &hurdle);
  if (pred != NULL)
    // easy solution: predecessor receives the returned bytes at its end
    pred->size += size;
  else {
    // no predecessor, insert this node
    node->size = size;
    node->right = node->left = NULL;
    insert_node(heap, heap, heap+heap_size, node);
  } 
}

tree_node_t*
tree_retrieve(tree_node_t* heap,
	      int size)
{
  // This function expects a heap with a special node at address 0
  // with size 0 and the usual right/left children pointers as tree
  // root.
  tree_node_t* r = retrieve_node(&heap->left, size);
  if (r == NULL)
    r = retrieve_node(&heap->right, size);
  return r;
}
