#ifndef TREEMALLOC_H
#define TREEMALLOC_H

#include <stddef.h>

typedef struct tree_node tree_node_t;

struct tree_node
{
  tree_node_t *left, *right;
  int size;
};

void
tree_merge_back(tree_node_t* heap,
		int heap_size,
		tree_node_t* node,
		int size);
tree_node_t*
tree_retrieve(tree_node_t* heap,
	      int size);
#endif
