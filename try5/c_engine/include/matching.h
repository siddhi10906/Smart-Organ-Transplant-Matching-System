#ifndef MATCHING_H
#define MATCHING_H

#include "types.h"
#include "avl.h"
#include "heap.h"
#include "graph.h"
#include "segment_tree.h"

typedef struct {
  Match *arr;
  int size;
  int cap;
} MatchList;

void matchlist_init(MatchList *ml, int cap);
void matchlist_push(MatchList *ml, Match m);
void matchlist_free(MatchList *ml);

int compute_score(const Patient *p, const Donor *d);
int is_compatible(const Patient *p, const Donor *d);

void run_matching(
  const Patient *patients,
  int patient_count,
  const Donor *donors,
  int donor_count,
  Graph *g,
  MatchList *out_matches,
  int *out_max_score
);

#endif
