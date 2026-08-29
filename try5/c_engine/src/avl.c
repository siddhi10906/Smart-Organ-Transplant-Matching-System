#include <stdlib.h>
#include "avl.h"

static int height(AVLNode *n) { return n ? n->height : 0; }
static int max2(int a, int b) { return a > b ? a : b; }

static AVLNode *new_node(Patient p) {
  AVLNode *n = (AVLNode *)malloc(sizeof(AVLNode));
  n->p = p;
  n->height = 1;
  n->left = NULL;
  n->right = NULL;
  return n;
}

static AVLNode *right_rotate(AVLNode *y) {
  AVLNode *x = y->left;
  AVLNode *t2 = x->right;

  x->right = y;
  y->left = t2;

  y->height = max2(height(y->left), height(y->right)) + 1;
  x->height = max2(height(x->left), height(x->right)) + 1;

  return x;
}

static AVLNode *left_rotate(AVLNode *x) {
  AVLNode *y = x->right;
  AVLNode *t2 = y->left;

  y->left = x;
  x->right = t2;

  x->height = max2(height(x->left), height(x->right)) + 1;
  y->height = max2(height(y->left), height(y->right)) + 1;

  return y;
}

static int get_balance(AVLNode *n) {
  if (!n) return 0;
  return height(n->left) - height(n->right);
}

AVLNode *avl_insert(AVLNode *root, Patient p) {
  if (!root) return new_node(p);

  if (p.id < root->p.id)
    root->left = avl_insert(root->left, p);
  else if (p.id > root->p.id)
    root->right = avl_insert(root->right, p);
  else
    return root;

  root->height = 1 + max2(height(root->left), height(root->right));

  int balance = get_balance(root);

  if (balance > 1 && p.id < root->left->p.id) return right_rotate(root);
  if (balance < -1 && p.id > root->right->p.id) return left_rotate(root);

  if (balance > 1 && p.id > root->left->p.id) {
    root->left = left_rotate(root->left);
    return right_rotate(root);
  }

  if (balance < -1 && p.id < root->right->p.id) {
    root->right = right_rotate(root->right);
    return left_rotate(root);
  }

  return root;
}

Patient *avl_search_by_id(AVLNode *root, int id) {
  AVLNode *cur = root;
  while (cur) {
    if (id == cur->p.id) return &cur->p;
    cur = id < cur->p.id ? cur->left : cur->right;
  }
  return NULL;
}

void avl_free(AVLNode *root) {
  if (!root) return;
  avl_free(root->left);
  avl_free(root->right);
  free(root);
}
