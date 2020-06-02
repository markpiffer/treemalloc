#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "treemalloc.h"

#define ALLOC_MAX 100

ptrdiff_t heap_ptr(tree_node_t *node);



#define HEAP_SIZE 1000
//tree_node_t heap[HEAP_SIZE] = { { .left = NULL, .right = NULL, .prev = heap, .next = NULL, .size =  1000 } };
tree_node_t heap[HEAP_SIZE] =
  {
   { .left = heap+1, .right = NULL, .size = 0 },
   { .left = NULL, .right=NULL, .size =  HEAP_SIZE-1 }
  };

			 
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
  void* r = tree_retrieve(heap, size);
  printf("Allocing %d at %d\n",size, heap_ptr(r));
  return r;
}

void
myfree(void* ptr, int size)
{
  printf("Freeing %d[%d]\n", heap_ptr(ptr),size);
  tree_merge_back(heap, HEAP_SIZE, ptr, size);
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
  print_tree(NULL, heap);
  clr_check();
  int l = check_tree(heap);
  l += check_alloc(&al);
  printf("Heapsize: %d\n",l);
  assert(l == HEAP_SIZE-1);
}
    
  
int main(void)
{
  print_tree(NULL, heap);
  for( int i = 0; i < 1000000; i++) {
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
