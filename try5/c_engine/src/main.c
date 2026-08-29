#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "avl.h"
#include "heap.h"
#include "graph.h"
#include "matching.h"
#include "io.h"

static int max_int(int a, int b) { return a > b ? a : b; }

static int compute_hospital_count(const Patient *p, int pc, const Donor *d, int dc) {
  int max_h = 0;
  for (int i = 0; i < pc; i++) max_h = max_int(max_h, p[i].hospital_id_0);
  for (int i = 0; i < dc; i++) max_h = max_int(max_h, d[i].hospital_id_0);
  return max_h + 1;
}

int main(void) {
  Patient *patients = NULL;
  Donor *donors = NULL;
  int pc = 0, dc = 0;

  if (!read_patients("patients.txt", &patients, &pc)) {
    printf("Error: cannot read patients.txt\n");
    return 1;
  }
  if (!read_donors("donors.txt", &donors, &dc)) {
    printf("Error: cannot read donors.txt\n");
    free(patients);
    return 1;
  }

  AVLNode *root = NULL;
  for (int i = 0; i < pc; i++) root = avl_insert(root, patients[i]);

  MaxHeap heap;
  heap_init(&heap, pc > 0 ? pc : 8);
  for (int i = 0; i < pc; i++) heap_push(&heap, patients[i]);

  int hospital_count = compute_hospital_count(patients, pc, donors, dc);
  if (hospital_count < 1) hospital_count = 1;

  Graph g;
  graph_init_default(&g, hospital_count);

  MatchList matches;
  int max_score = 0;
  run_matching(patients, pc, donors, dc, &g, &matches, &max_score);

  printf("Matching Result\n");
  printf("[Segment Tree] Max Score: %d\n\n", max_score);

  if (matches.size == 0) {
    printf("No compatible matches found.\n");
  } else {
    for (int i = 0; i < matches.size; i++) {
      Match m = matches.arr[i];
      Patient *p = avl_search_by_id(root, m.patient_id);
      Donor *d = &donors[m.donor_id];
      if (!p) continue;

      printf(
        "Donor#%d(%s,%s,H%d)  ->  Patient#%d(%s,%s,H%d,U%d,S%d)  (Score: %d)\n",
        d->id,
        d->organ,
        d->blood_group,
        d->hospital_id_0 + 1,
        p->id,
        p->organ,
        p->blood_group,
        p->hospital_id_0 + 1,
        p->urgency,
        p->severity,
        m.score
      );
    }
  }

  matchlist_free(&matches);
  graph_free(&g);
  heap_free(&heap);
  avl_free(root);
  free(patients);
  free(donors);

  return 0;
}
