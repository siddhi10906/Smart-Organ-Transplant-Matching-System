#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

#define MAX_BLOOD 8
#define MAX_ORGAN 16

typedef struct {
  int id;
  char blood_group[MAX_BLOOD];
  int hospital_id_0;
  int urgency;
  int severity;
  int consent;
  int screening;
  char organ[MAX_ORGAN];
  int age;
  char gender[16];
  int weight;
} Patient;

typedef struct {
  int id;
  char blood_group[MAX_BLOOD];
  int condition_score;
  int hospital_id_0;
  int consent;
  int screening;
  char organ[MAX_ORGAN];
  int age;
  char gender[16];
  int weight;
} Donor;

typedef struct {
  int donor_id;
  int patient_id;
  int score;
} Match;

#endif
