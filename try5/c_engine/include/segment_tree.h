#ifndef SEGMENT_TREE_H
#define SEGMENT_TREE_H

typedef struct {
  int n;
  int *tree;
} SegmentTree;

void seg_init(SegmentTree *st, int n);
void seg_build(SegmentTree *st, const int *arr);
int seg_query_max(SegmentTree *st, int ql, int qr);
void seg_free(SegmentTree *st);

#endif
