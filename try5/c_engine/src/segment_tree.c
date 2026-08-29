#include <stdlib.h>
#include "segment_tree.h"

static int max2(int a, int b) { return a > b ? a : b; }

void seg_init(SegmentTree *st, int n) {
  st->n = n;
  st->tree = (int *)malloc(sizeof(int) * (size_t)(4 * (n > 0 ? n : 1)));
}

static void build_rec(SegmentTree *st, const int *arr, int node, int l, int r) {
  if (l == r) {
    st->tree[node] = arr[l];
    return;
  }
  int mid = (l + r) / 2;
  build_rec(st, arr, node * 2, l, mid);
  build_rec(st, arr, node * 2 + 1, mid + 1, r);
  st->tree[node] = max2(st->tree[node * 2], st->tree[node * 2 + 1]);
}

void seg_build(SegmentTree *st, const int *arr) {
  if (st->n <= 0) return;
  build_rec(st, arr, 1, 0, st->n - 1);
}

static int query_rec(SegmentTree *st, int node, int l, int r, int ql, int qr) {
  if (qr < l || r < ql) return -2147483647;
  if (ql <= l && r <= qr) return st->tree[node];
  int mid = (l + r) / 2;
  int a = query_rec(st, node * 2, l, mid, ql, qr);
  int b = query_rec(st, node * 2 + 1, mid + 1, r, ql, qr);
  return max2(a, b);
}

int seg_query_max(SegmentTree *st, int ql, int qr) {
  if (st->n <= 0) return 0;
  if (ql < 0) ql = 0;
  if (qr >= st->n) qr = st->n - 1;
  if (ql > qr) return 0;
  return query_rec(st, 1, 0, st->n - 1, ql, qr);
}

void seg_free(SegmentTree *st) {
  free(st->tree);
  st->tree = NULL;
  st->n = 0;
}
