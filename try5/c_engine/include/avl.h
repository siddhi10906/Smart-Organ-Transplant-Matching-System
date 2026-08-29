#ifndef AVL_H
#define AVL_H

#include "types.h"

typedef struct AVLNode {
  Patient p;
  int height;
  struct AVLNode *left;
  struct AVLNode *right;
} AVLNode;

AVLNode *avl_insert(AVLNode *root, Patient p);
Patient *avl_search_by_id(AVLNode *root, int id);
void avl_free(AVLNode *root);

#endif
