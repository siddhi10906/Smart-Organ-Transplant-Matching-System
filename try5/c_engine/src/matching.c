#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "matching.h"

static int str_eq(const char *a, const char *b) {
  return strcmp(a, b) == 0;
}

static void parse_blood_group(const char *bg, char *out_abo, int *out_rh) {
  if (!bg || !*bg) {
    *out_abo = '?';
    *out_rh = 0;
    return;
  }

  if (strncmp(bg, "AB", 2) == 0) {
    *out_abo = 'C';
    bg += 2;
  } else if (bg[0] == 'A') {
    *out_abo = 'A';
    bg += 1;
  } else if (bg[0] == 'B') {
    *out_abo = 'B';
    bg += 1;
  } else if (bg[0] == 'O') {
    *out_abo = 'O';
    bg += 1;
  } else {
    *out_abo = '?';
  }

  while (*bg == ' ' || *bg == '\t') bg++;
  if (*bg == '+') *out_rh = 1;
  else if (*bg == '-') *out_rh = 0;
  else *out_rh = 0;
}

static int compatible_blood(const char *recipient_bg, const char *donor_bg) {
  char r_abo = '?', d_abo = '?';
  int r_rh = 0, d_rh = 0;
  parse_blood_group(recipient_bg, &r_abo, &r_rh);
  parse_blood_group(donor_bg, &d_abo, &d_rh);

  if (r_abo == '?' || d_abo == '?') return 0;

  int abo_ok = 0;
  if (r_abo == 'C') {
    abo_ok = 1;
  } else if (r_abo == 'A') {
    abo_ok = (d_abo == 'A' || d_abo == 'O');
  } else if (r_abo == 'B') {
    abo_ok = (d_abo == 'B' || d_abo == 'O');
  } else if (r_abo == 'O') {
    abo_ok = (d_abo == 'O');
  }
  if (!abo_ok) return 0;

  if (r_rh == 0 && d_rh == 1) return 0;
  return 1;
}

void matchlist_init(MatchList *ml, int cap) {
  ml->cap = cap > 0 ? cap : 8;
  ml->size = 0;
  ml->arr = (Match *)malloc(sizeof(Match) * (size_t)ml->cap);
}

void matchlist_push(MatchList *ml, Match m) {
  if (ml->size >= ml->cap) {
    ml->cap *= 2;
    ml->arr = (Match *)realloc(ml->arr, sizeof(Match) * (size_t)ml->cap);
  }
  ml->arr[ml->size++] = m;
}

void matchlist_free(MatchList *ml) {
  free(ml->arr);
  ml->arr = NULL;
  ml->size = 0;
  ml->cap = 0;
}

int is_compatible(const Patient *p, const Donor *d) {
  if (!p->consent || !p->screening) return 0;
  if (!d->consent || !d->screening) return 0;

  // Organ must match
  if (!str_eq(p->organ, d->organ)) return 0;

  // Blood compatibility (ABO + Rh). Eye donation doesn't require blood matching.
  if (!str_eq(p->organ, "EYE")) {
    if (!compatible_blood(p->blood_group, d->blood_group)) return 0;
  }

  // ✅ Size constraint (critical for heart/lung/liver)
  if (str_eq(p->organ, "HEART") || str_eq(p->organ, "LUNG") || str_eq(p->organ, "LIVER")) {
    int diff = abs(p->weight - d->weight);
    if (diff > (p->weight * 25 / 100)) return 0;
  }

  return 1;
}

int compute_score(const Patient *p, const Donor *d) {
  int score = 0;

  // Base matching
  score += 80;

  // Urgency + severity
  score += p->urgency * 2;
  score += p->severity * 2;

  // Donor condition
  score += d->condition_score;

  // ✅ AGE LOGIC (improved)
  int age_diff = abs(p->age - d->age);

  if (age_diff <= 5) {
    score += 15;
  } else if (age_diff <= 15) {
    score += 5;
  } else if (age_diff <= 30) {
    score -= 10;
  } else {
    score -= 25;
  }

  // ✅ GENDER (soft preference ONLY)
  if (str_eq(p->gender, d->gender)) {
    score += 5;
  } else {
    score -= 2;
  }

  // ✅ Bonus for same hospital (logistics advantage)
  if (p->hospital_id_0 == d->hospital_id_0) {
    score += 10;
  }

  return score;
}

static int match_cmp_desc(const void *a, const void *b) {
  const Match *ma = (const Match *)a;
  const Match *mb = (const Match *)b;
  return (mb->score - ma->score);
}

static int match_cmp_desc_then_near(const void *a, const void *b) {
  const Match *ma = (const Match *)a;
  const Match *mb = (const Match *)b;
  if (mb->score != ma->score) return mb->score - ma->score;
  return ma->patient_id - mb->patient_id;
}

static void add_pair_matches(
  const Patient *patients,
  int patient_count,
  const Donor *donors,
  int donor_count,
  int restrict_hospital,
  MatchList *all
) {
  for (int di = 0; di < donor_count; di++) {
    for (int pi = 0; pi < patient_count; pi++) {
      const Patient *p = &patients[pi];
      const Donor *d = &donors[di];

      if (restrict_hospital >= 0) {
        if (p->hospital_id_0 != restrict_hospital || d->hospital_id_0 != restrict_hospital) continue;
      }

      if (!is_compatible(p, d)) continue;

      Match m;
      m.donor_id = d->id;
      m.patient_id = p->id;
      m.score = compute_score(p, d);
      matchlist_push(all, m);
    }
  }
}

static void heap_order_patients(const Patient *patients, int patient_count, Patient **out_ordered) {
  MaxHeap h;
  heap_init(&h, patient_count > 0 ? patient_count : 8);
  for (int i = 0; i < patient_count; i++) heap_push(&h, patients[i]);

  Patient *ordered = (Patient *)malloc(sizeof(Patient) * (size_t)patient_count);
  int idx = 0;
  while (!heap_empty(&h)) {
    ordered[idx++] = heap_pop(&h);
  }

  heap_free(&h);
  *out_ordered = ordered;
}

static void build_candidates_same_hospital_then_bfs(
  const Patient *patients_by_id,
  const Patient *patients_by_urgency,
  int patient_count,
  const Donor *donors,
  int donor_count,
  Graph *g,
  MatchList *all
) {
  for (int di = 0; di < donor_count; di++) {
    const Donor *d = &donors[di];

    MatchList local;
    matchlist_init(&local, 16);

    for (int pi = 0; pi < patient_count; pi++) {
      const Patient *p = &patients_by_urgency[pi];
      if (!is_compatible(p, d)) continue;
      Match m;
      m.donor_id = d->id;
      m.patient_id = p->id;
      m.score = compute_score(p, d);
      matchlist_push(&local, m);
    }

    if (local.size == 0) {
      matchlist_free(&local);
      continue;
    }

    int donor_h = d->hospital_id_0;

    int has_same = 0;
    for (int i = 0; i < local.size; i++) {
      int pid = local.arr[i].patient_id;
      if (pid >= 0 && pid < patient_count && patients_by_id[pid].hospital_id_0 == donor_h) {
        has_same = 1;
        break;
      }
    }

    if (has_same) {
      for (int i = 0; i < local.size; i++) {
        int pid = local.arr[i].patient_id;
        if (pid >= 0 && pid < patient_count && patients_by_id[pid].hospital_id_0 == donor_h) {
          matchlist_push(all, local.arr[i]);
        }
      }
    } else {
      int best_dist = 2147483647;
      for (int i = 0; i < local.size; i++) {
        int pid = local.arr[i].patient_id;
        if (pid < 0 || pid >= patient_count) continue;
        int ph = patients_by_id[pid].hospital_id_0;
        int dist = graph_bfs_distance(g, donor_h, ph);
        if (dist >= 0 && dist < best_dist) best_dist = dist;
      }

      for (int i = 0; i < local.size; i++) {
        int pid = local.arr[i].patient_id;
        if (pid < 0 || pid >= patient_count) continue;
        int ph = patients_by_id[pid].hospital_id_0;
        int dist = graph_bfs_distance(g, donor_h, ph);
        if (dist == best_dist) matchlist_push(all, local.arr[i]);
      }
    }

    matchlist_free(&local);
  }
}

static void greedy_select_unique(
  const MatchList *sorted,
  int donor_count,
  int patient_count,
  MatchList *selected
) {
  int *donor_used = (int *)calloc((size_t)donor_count, sizeof(int));
  int *patient_used = (int *)calloc((size_t)patient_count, sizeof(int));

  for (int i = 0; i < sorted->size; i++) {
    Match m = sorted->arr[i];
    if (m.donor_id < 0 || m.donor_id >= donor_count) continue;
    if (m.patient_id < 0 || m.patient_id >= patient_count) continue;

    if (donor_used[m.donor_id]) continue;
    if (patient_used[m.patient_id]) continue;

    donor_used[m.donor_id] = 1;
    patient_used[m.patient_id] = 1;
    matchlist_push(selected, m);
  }

  free(donor_used);
  free(patient_used);
}

void run_matching(
  const Patient *patients,
  int patient_count,
  const Donor *donors,
  int donor_count,
  Graph *g,
  MatchList *out_matches,
  int *out_max_score
) {
  MatchList all;
  matchlist_init(&all, patient_count * donor_count);

  Patient *patients_by_urgency = NULL;
  heap_order_patients(patients, patient_count, &patients_by_urgency);
  build_candidates_same_hospital_then_bfs(patients, patients_by_urgency, patient_count, donors, donor_count, g, &all);
  free(patients_by_urgency);

  if (all.size == 0) {
    *out_max_score = 0;
    matchlist_init(out_matches, 1);
    matchlist_free(&all);
    return;
  }

  qsort(all.arr, (size_t)all.size, sizeof(Match), match_cmp_desc_then_near);

  MatchList selected;
  matchlist_init(&selected, all.size);

  greedy_select_unique(&all, donor_count, patient_count, &selected);

  int *scores = (int *)malloc(sizeof(int) * (size_t)selected.size);
  for (int i = 0; i < selected.size; i++) scores[i] = selected.arr[i].score;

  SegmentTree st;
  seg_init(&st, selected.size);
  seg_build(&st, scores);
  *out_max_score = seg_query_max(&st, 0, selected.size - 1);

  seg_free(&st);
  free(scores);

  *out_matches = selected;
  matchlist_free(&all);
}
