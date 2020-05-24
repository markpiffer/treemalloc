#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define ALLOC_MAX 100


typedef struct tree_node tree_node_t;
struct tree_node
{
  tree_node_t *left, *right;
  // tree_node_t *prev, *next;
  int size;
};

ptrdiff_t heap_ptr(tree_node_t *node);

#define KILL_CHILD_MARKER ((tree_node_t*)-1ul)
#define RELINK_CHILD_MARKER ((tree_node_t*)-2ul)

tree_node_t*
calc_middle(tree_node_t *left, tree_node_t *right)
{
  return left + (((ptrdiff_t)right - (ptrdiff_t)left) / sizeof(tree_node_t) + 1) / 2;
}

/* tree_node_t* */
/* get_closest_prev(tree_node_t *root, */
/* 		 tree_node_t *new_node, */
/*                  tree_node_t *current_closest_prev) */
/* { */
/*   // Find the closest previous (left) node in the tree by using the tree structure. */
/*   // It is assumed that current_closest_prev is the best previous node which appeared */
/*   // during the descent in the tree until root. This function is called when the left */
/*   // subtree of root is empty */
/*   if (new_node > self) { */
/*     // ???    xxxx */
/*     return current_closest_prev; */
/*   } */
/*   if (/\*(current_closest_prev == NULL) ||*\/ (current_closest_prev < new_node)) { */
/*     return new_node; */
/*   } */
/*   return current_closest_prev; */
/* } */


/* void */
/* list_unlink_node(tree_node_t *kill_node) */
/* { */
/*   kill_node->prev->next = kill_node->next; */
/*   kill_node->next->prev = kill_node->prev; */
/* } */

/* tree_node_t* */
/* list_find_insert(tree_node_t* root, tree_node_t* new_node) */
/* { */
/*   // find place where new_node can be inserted in front */
/*   if (root < new_node) { */
/*     do{ */
/*       root = root->next; */
/*     } while (root->next < new_node); */
/*   } */
/*   else{ */
/*     while (root->prev > new_node) { */
/*       root = root->prev; */
/*     } */
/*   } */
/*   return root; */
/* } */
		 
tree_node_t *
tree_unlink_leaf(tree_node_t *root)
{
  // Take a leaf out of the tree. There is just a recursive descent
  // (no search) involved and the leaf may come from anywhere in the subtree.
  if (root->left != NULL) {
    tree_node_t *unlinked_node = tree_unlink_leaf(root->left);
    if (root->left == unlinked_node) {
      root->left = NULL;
    }
    return unlinked_node; 
  } else if (root->right != NULL) {
    tree_node_t *unlinked_node = tree_unlink_leaf(root->right);
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
/*   tree_node_t* new_left = tree_unlink_leaf(root->left); // retrieve a leaf from left subtree */
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
/*   tree_node_t* new_right = tree_unlink_leaf(root->right); // retrieve a leaf from right subtree */
/*   if (root->right == new_right) { // kill_node was the only node in subtree */
/*     root->right = NULL; // done */
/*   } */
/*   else { // link the leaf as new intra-tree node */
/*     new_right->left = root->right->left; */
/*     new_right->right = root->right->right; */
/*     root->right = new_right; */
/*   } */
/* } */

void
tree_unlink_child(tree_node_t **kill_node_ptr)
{
  tree_node_t* intra_node = tree_unlink_leaf(*kill_node_ptr); // retrieve a leaf from subtree
  if (*kill_node_ptr == intra_node) { // kill_node was the only node in subtree
    *kill_node_ptr = NULL; // done
  }
  else { // link the leaf as new intra-tree node
    intra_node->left = (*kill_node_ptr)->left;
    intra_node->right = (*kill_node_ptr)->right;
    *kill_node_ptr = intra_node;
  }
}

int // returns kill_node->size if kill_node was removed
// or 0 if it was not found in the tree
tree_unlink_descendant(tree_node_t *root,
		       tree_node_t *kill_node)
{
  if (root->left == kill_node) {
    int size = kill_node->size;
    tree_unlink_child(&root->left);
    return size;
  }
  else if (root->right == kill_node) {
    int size = kill_node->size;
    tree_unlink_child(&root->right);
    return size;
  }
  else {
    return 0;
  }
}

int // returns size of kill_node if kill_node was removed
// or 0 if it was not found in the tree
tree_unlink_node(tree_node_t *root,
		 tree_node_t *from,
		 tree_node_t *to,
		 tree_node_t *kill_node)
{
  if (root == NULL)
    return 0;

  int size = tree_unlink_descendant(root, kill_node);
  if (size > 0) {
    return size;
  }
  else {
    // kill_node is not a direct descendant of root
    // select the subtree where kill_node resides:
    tree_node_t *middle = calc_middle(from, to);
    if (kill_node < middle) {
      return tree_unlink_node(root->left, from, middle, kill_node);
    }
    else {
      return tree_unlink_node(root->right, middle, to, kill_node);
    }
  }
}



int
kill_successor(tree_node_t* root,
	       tree_node_t* successor)
{
  if (root != NULL) {
    int size = tree_unlink_descendant(root, successor);
    if (size > 0) {
      return size;
    }
    else {
      // successor is not a direct descendant of root
      // go left if possible
      if (root->left != NULL) {
	return kill_successor(root->left, successor);
      }
      else /*if (calc_middle(from,to) <= successor) */{
	return kill_successor(root->right, successor);
      }
    }
  }
  return 0;
}

tree_node_t*
find_root_succ( // find the root of the successor node so that we can eliminate it
		     tree_node_t* root,
		     tree_node_t* from,
		     tree_node_t* to,
		     tree_node_t* find_this)
{
  /* 
     |                 x     |
     |        x  |         x |
     |    x|   x |x    |x    |
     |  |  |  |  |  |  |  |  |
  */
  tree_node_t* middle = calc_middle(from, to);
  printf("succ(%d)",heap_ptr(root));
  if (find_this < middle)
    {
	if (root->left == find_this)
	  {printf("SUCC(%d) ",heap_ptr(root->left));
	    return root;}
	else if(root->left != NULL)
	  return find_root_succ(root->left, from, middle, find_this);
	else
	  return NULL;
    }
  else if (root->right != NULL)
    {
      	if (root->right == find_this)
	  {printf("SUCC(%d) ",heap_ptr(root->right));
	    return root;}
	else
	  return find_root_succ(root->right, middle, to, find_this);
    }
  else
      return     printf("SUCC() "),NULL;
}


tree_node_t* // some_node: this is the predecessor
// NULL: no predecessor in this subtree
find_node_pred( // find the predecessor node so that we can integrate into it
	       tree_node_t* root,
	       tree_node_t* from,
	       tree_node_t* to,
	       tree_node_t* node,
	       tree_node_t** hurdle)
{
  /*
    |                   v     |
    |                     x   |
    |        x  |          x  |
    |    x|   x |x    |x      |
    | x|x |x |  |  |x |  x|x  |
  */
  if (root == NULL)
    return NULL;
  
  if (root + root->size == node)
    return root;

  if (root > *hurdle && root < node)
    *hurdle = root;
  
  tree_node_t* middle = calc_middle(from, to);
  
  if (middle < node) {
    // node is in right half, there could be a node in right subtree predecessing it
    tree_node_t* pred = find_node_pred(root->right, middle, to, node, hurdle);
    if (pred != NULL)
      return pred;
  }
  // it only makes sense to go left if there wasn't a better predecessor yet
  if (*hurdle < middle)
    return find_node_pred(root->left, from, middle, node, hurdle);

  return NULL;
}

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


void
tree_insert(tree_node_t* tree,
	    int tree_size,
	    tree_node_t* node,
	    int size)
{
  // This function assumes a heap with a special node at address 0
  // with size 0 and the usual right/left children pointers as tree
  // root.
  
  // first step: try to delete the node immediately following
  size += tree_unlink_node(tree, tree, tree+tree_size, node+size);
  // now look for a predecessor to which node is adjacent
  tree_node_t* hurdle = tree;
  tree_node_t* pred = find_node_pred(tree, tree, tree+tree_size, node, &hurdle);
  if (pred != NULL) {
    // easy solution: predecessor receives a few new bytes at its end
    pred->size += size;
  }
  else {
    // no predecessor, insert this node
    node->size = size;
    insert_node(tree, tree, tree+tree_size, node);
  } 
}



/* tree_node_t* */
/* tree_insert_down(tree_node_t *root, tree_node_t *from, tree_node_t *to, tree_node_t *new_node, int size) */
/* { */
/*   // Insert node as deep as possible */
/*   //--------------------------------------------------------------------------------- */
/*   // simplest case: inserted node appends to root */
/*   if (root + root->size == new_node) */
/*     { */
/*       root->size += size; // the inserted node is merged immediately into the existing root */
/*       if (root + root->size == root->next) // test if this fills a hole */
/* 	{  */
/* 	  root->size += root->next->size; // combine all three */
/* 	  tree_node_t *kill_node = root->next; */
/* 	  list_unlink_node(kill_node); */
/* 	  if ((from <= kill_node) && (kill_node <= to)) */
/* 	    { // if kill_node resides in this subtree, try to kill it immediately */
/* 	      kill_node = tree_unlink_node(root, from, to, kill_node); */
/* 	      // if kill_node != NULL here, the node wasn't found in the subtree, */
/* 	      // in this case the only place where kill_node is to be found is upwards */
/* 	    } */
/* 	  return kill_node; */
/* 	} */
/*       else */
/* 	{ */
/* 	  return NULL; */
/* 	} */
/*     } */
/*   //--------------------------------------------------------------------------------- */
/*   // second case: inserted node prepends root */
/*   else if (new_node + size == root) */
/*     {  */
/*       size += root->size; // the root is merged into the new node at new_node */
/*       if (root->prev + root->prev->size == new_node) */
/* 	{ // test if a hole left of the root is filled */
/* 	  root->prev->size += size; // notice that root->prev can be in a different tree */
/* 	  list_unlink_node(root); */
/* 	  return KILL_CHILD_MARKER; // root shall be erased  */
/* 	} */
/*       else */
/* 	{ // in this case, we can use the newly inserted node in place of the root; */
/* 	  // done by inserting the new node down into the subtree and then killing */
/* 	  new_node->prev = root->prev; */
/* 	  new_node->next = root->next; */
/* 	  new_node->left = root->left; */
/* 	  new_node->right = root->right; */
/* 	  root->prev = new_node; // remember where to point to now from the parent */
/* 	  return RELINK_CHILD_MARKER; // special case, signal "new child" to parent */
/* 	} */
/*     } */
/*   //--------------------------------------------------------------------------------- */
/*   else */
/*     { */
/*       tree_node_t *middle = calc_middle(from,to); */
/*       if (new_node < middle) */
/* 	{ */
/* 	  if (root->left != NULL) */
/* 	    { */
/* 	      tree_node_t *kill_node = tree_insert(root->left, from, middle-1, new_node, size); */
/* 	      if (kill_node == RELINK_CHILD_MARKER) */
/* 		{ // in this case new_node was prepended to root->left */
/* 		  root->left = root->left->prev; // relink to new child */
/* 		  return NULL; */
/* 		} */
/* 	      else if (kill_node == KILL_CHILD_MARKER) */
/* 		{ */
/* 		  if(tree_unlink_node(root, from, to, root->left) != NULL) */
/* 		    { */
/* 		      goto Error; */
/* 		    } */
/* 		  return NULL; */
/* 		} */
/* 	      else if ((kill_node != NULL) && (from <= kill_node) && (kill_node <= to)) */
/* 		{ */
/* 		  list_unlink_node(kill_node); */
/* 		  if (tree_unlink_node(root, from, to, kill_node) != NULL) */
/* 		    { */
/* 		      goto Error; */
/* 		    } */
/* 		  return NULL; */
/* 		} */
/* 	      else */
/* 		{ */
/* 		  return kill_node; // node is NULL or outside of current tree */
/* 		} */
/* 	    } */
/* 	  else */
/* 	    { */
/* 	      root->left = new_node; */
/* 	      closest_prev = get_closest_prev(new_node, closest_prev, root); */
/* 	      new_node->next = closest_prev->next; */
/* 	      closest_prev->next = new_node; */
/* 	      new_node->prev = closest_prev; */
/* 	      new_node->left = new_node->right = NULL; */
/* 	      new_node->size = size; */
/* 	      return NULL; */
/* 	    } */
/* 	} */
/*       else // new_node >= middle */
/* 	{ */
/* 	  if (root->right != NULL) */
/* 	    { */
/* 	      tree_node_t *kill_node = tree_insert(root->right, middle, to, new_node, size, get_closest_prev(new_node, closest_prev, root)); */
/* 	      if (kill_node == KILL_CHILD_MARKER) */
/* 		{ // in this case new_node was prepended to root->right */
/* 		  root->right = root->right->prev; // relink to new child */
/* 		  return NULL; */
/* 		} */
/* 	      else if ((kill_node != NULL) && (from <= kill_node) && (kill_node <= to)) */
/* 		{ */
/* 		  tree_unlink_node(root, from, to, kill_node); */
/* 		  list_unlink_node(kill_node); */
/* 		  return NULL; */
/* 		} */
/* 	      else */
/* 		{ */
/* 		  return kill_node; // node is NULL or outside of current tree */
/* 		} */
/* 	    } */
/* 	  else */
/* 	    { */
/* 	      root->right = new_node; */
/* 	      closest_prev = get_closest_prev(new_node, closest_prev, root); */
/* 	      new_node->next = closest_prev->next; */
/* 	      closest_prev->next = new_node; */
/* 	      new_node->prev = closest_prev; */
/* 	      new_node->left = new_node->right = NULL; */
/* 	      new_node->size = size; */
/* 	      return NULL; */
/* 	    } */
/* 	} */
/*     } */
/* } */

      
tree_node_t*
tree_retrieve(tree_node_t **root, int size)
{
  if (*root == NULL) {
    return NULL;
  }
  else if ((*root)->size >= size) {
    tree_node_t *alloc = (*root) + (*root)->size - size;
    (*root)->size -= size;
    if ((*root)->size == 0) { // whole node was taken
      tree_unlink_child(root);
    }
    return alloc;
  }
  else {
    tree_node_t *alloc = tree_retrieve(&((*root)->left), size); 
    if (alloc == NULL) {
      alloc = tree_retrieve(&((*root)->right), size);
    }
    return alloc;
  }
}



#define HEAP_SIZE 1000
//tree_node_t heap[HEAP_SIZE] = { { .left = NULL, .right = NULL, .prev = heap, .next = NULL, .size =  1000 } };
tree_node_t heap[HEAP_SIZE] = { { .left = heap+1, .right = NULL, .size = 0 }, { .left = NULL, .right=NULL, .size =  HEAP_SIZE-1 } };
tree_node_t* heap_root = &heap[0];
			 
ptrdiff_t
heap_ptr(tree_node_t *node)
{
  if (node == NULL) return -1;
  return node-heap;
}
//--------------------------------------------------------------------------------




void*
myalloc(int size)
{
  return tree_retrieve(&heap_root, size);
}

void
myfree(void* ptr, int size)
{
  printf("Freeing %d[%d]\n", heap_ptr(ptr),size);
  tree_insert(heap, HEAP_SIZE, ptr, size);
}
  
  //--------------------------------------------------------------------------------
void
dot_node(tree_node_t *parent, tree_node_t *root, tree_node_t *from, tree_node_t *to, int rank)
{
  if (root != NULL)
    {
      if (rank >= 0) {
	
	printf("N%d -- N%d [rank=%d]\nL%d -- N%d\nN%d -- R%d\n",heap_ptr(parent),heap_ptr(root),rank,heap_ptr(root),heap_ptr(from),heap_ptr(root),heap_ptr(to));
      }
      else {
	printf("N%d -- N%d [rank=%d]\nN%d -- L%d\nN%d -- R%d\n",heap_ptr(parent),heap_ptr(root),-rank,heap_ptr(root),heap_ptr(from),heap_ptr(root),heap_ptr(to));
      }	
      tree_node_t *middle = calc_middle(from,to);
      dot_node(root,root->left,from,middle,abs(rank)+1);
      dot_node(root,root->right,middle,to,-(abs(rank)+1));
    }
}

void
dot_tree(tree_node_t *parent, tree_node_t *root, tree_node_t *from, tree_node_t *to)
{
  printf("graph {\n");
  dot_node(parent, root, from, to,0);
  printf("}\n");
}

char check_arr[HEAP_SIZE+1];

void clr_check()
{
  for (int i = 0; i < HEAP_SIZE+1; i++)
    check_arr[i] = 0;
}

int check_tree(tree_node_t* root)
{
  if (root == NULL)
    return 0;
  if(check_arr[heap_ptr(root)] != 0){
    printf("Error at %d\n",heap_ptr(root));
    assert(check_arr[heap_ptr(root)] == 0);
  }
  assert(check_arr[heap_ptr(root + root->size)] == 0);
  check_arr[heap_ptr(root)] = 1;
  check_arr[heap_ptr(root + root->size)] = 1;
  return check_tree(root->left) + check_tree(root->right) + root->size;
}

struct tree_level
{
  char* next;
  char string[32];
};

struct tree_level tree_lvl[16];

void
print_tree(tree_node_t *parent, tree_node_t *root)
{
  if (root != NULL) {
    printf("[%d]->%d..%d[%d] l:%d r:%d\n",
	   heap_ptr(parent),
	   heap_ptr(root),
	   heap_ptr(root+root->size),
	   root->size,
	   heap_ptr(root->left),
	   heap_ptr(root->right)
	   );
    print_tree(root, root->left);
    print_tree(root, root->right);
  }
}

	     
#define N_ELEM 100000
typedef struct 
{
  void* plst[N_ELEM];
  int slst[N_ELEM];
  int len;
} alloced_list_t;

alloced_list_t al;

int check_alloc(alloced_list_t *al)
{
  int l = 0;
  for(int i=0; i < al->len; i++) {
    l += al->slst[i];
  }
  return l;
}

_Bool add_allc(alloced_list_t *al, int size)
{
  if (al->len == N_ELEM)
    return 0;
  void* ap = myalloc(size);
  if (ap == NULL)
    {
      printf("!!!!!!!!!!!!!!!!!!Can't allocate %d!\n", size);
      return 0;
    }
  printf("Alloced: (%d) @%d\n", size, heap_ptr(ap));
  al->plst[al->len] = ap;
  al->slst[al->len++] = size;
  return 1;
}

_Bool add_rand_allc(alloced_list_t *al)
{
  int size = 1 + (long)rand() * ALLOC_MAX / RAND_MAX;
  return add_allc(al, size);
}


_Bool remove_allc(alloced_list_t *al, int ix)
{
  if ((al->len == 0) || (al->len <= ix))
    return 0;
  if (ix == -1)
    ix = al->len-1;
  myfree(al->plst[ix],al->slst[ix]);
  printf("Freed: (%d) @%d\n", al->slst[ix], heap_ptr(al->plst[ix]));
  al->plst[ix] = al->plst[al->len-1];
  al->slst[ix] = al->slst[al->len-1];
  al->len--;
  return 1;
}

_Bool remove_rand_allc(alloced_list_t *al)
{
  int ix = (long)rand() * al->len / RAND_MAX;
  return remove_allc(al, ix);
}
    
      

void
print_heap(void)
{
  printf("--------------\n");
  print_tree(NULL, heap_root);
  clr_check();
  int l = check_tree(heap_root);
  l += check_alloc(&al);
  printf("Heapsize: %d\n",l);
  assert(l == HEAP_SIZE-1);
}
    
  
int main(void)
{
  print_tree(NULL, heap_root);
  for( int i = 0; i < 1000; i++) {
    if (rand() & 1 || al.len == 0) {
      add_rand_allc(&al);
    }
    else {
      remove_rand_allc(&al);
    }
    print_heap();
    //    getchar();
  }
  
  dot_tree(heap,heap,heap,heap+HEAP_SIZE); printf("\n");

}
