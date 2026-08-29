#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

static void safe_copy(char *dst, size_t cap, const char *src) {
  if (!src) {
    if (cap) dst[0] = '\0';
    return;
  }
  strncpy(dst, src, cap - 1);
  dst[cap - 1] = '\0';
}

static int parse_int_or_default(const char *s, int def) {
  if (!s || !*s) return def;
  return atoi(s);
}

static void trim_newline(char *s) {
  if (!s) return;
  size_t n = strlen(s);
  while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
    s[n - 1] = '\0';
    n--;
  }
}

int read_patients(const char *path, Patient **out, int *out_count) {
  FILE *f = fopen(path, "r");
  if (!f) return 0;

  int cap = 32;
  int count = 0;
  Patient *arr = (Patient *)malloc(sizeof(Patient) * (size_t)cap);

  char line[512];
  while (fgets(line, sizeof(line), f)) {
    trim_newline(line);
    if (line[0] == '\0') continue;

    char blood[32] = {0};
    char hstr[32] = {0};
    char urg[32] = {0};
    char sev[32] = {0};
    char cons[32] = {0};
    char scr[32] = {0};
    char organ[32] = {0};
    char age_str[32] = {0};
    char gender[32] = {0};
    char weight_str[32] = {0};

    int n = sscanf(line, "%31s %31s %31s %31s %31s %31s %31s %31s %31s %31s", 
                   blood, hstr, urg, sev, cons, scr, organ, age_str, gender, weight_str);
    if (n < 2) continue;

    if (count >= cap) {
      cap *= 2;
      arr = (Patient *)realloc(arr, sizeof(Patient) * (size_t)cap);
    }

    Patient p;
    p.id = count;
    safe_copy(p.blood_group, MAX_BLOOD, blood);
    int h1 = atoi(hstr);
    p.hospital_id_0 = h1 > 0 ? (h1 - 1) : 0;
    p.urgency = parse_int_or_default(n >= 3 ? urg : NULL, 50);
    p.severity = parse_int_or_default(n >= 4 ? sev : NULL, 20);
    p.consent = parse_int_or_default(n >= 5 ? cons : NULL, 1);
    p.screening = parse_int_or_default(n >= 6 ? scr : NULL, 1);
    safe_copy(p.organ, MAX_ORGAN, (n >= 7 ? organ : "KIDNEY"));
    p.age = parse_int_or_default(n >= 8 ? age_str : NULL, 30);
    safe_copy(p.gender, 16, (n >= 9 ? gender : "Other"));
    p.weight = parse_int_or_default(n >= 10 ? weight_str : NULL, 70);

    arr[count++] = p;
  }

  fclose(f);
  *out = arr;
  *out_count = count;
  return 1;
}

int read_donors(const char *path, Donor **out, int *out_count) {
  FILE *f = fopen(path, "r");
  if (!f) return 0;

  int cap = 32;
  int count = 0;
  Donor *arr = (Donor *)malloc(sizeof(Donor) * (size_t)cap);

  char line[512];
  while (fgets(line, sizeof(line), f)) {
    trim_newline(line);
    if (line[0] == '\0') continue;

    char blood[32] = {0};
    char cstr[32] = {0};
    char hstr[32] = {0};
    char cons[32] = {0};
    char scr[32] = {0};
    char organ[32] = {0};
    char age_str[32] = {0};
    char gender[32] = {0};
    char weight_str[32] = {0};

    int n = sscanf(line, "%31s %31s %31s %31s %31s %31s %31s %31s %31s", 
                   blood, cstr, hstr, cons, scr, organ, age_str, gender, weight_str);
    if (n < 3) continue;

    if (count >= cap) {
      cap *= 2;
      arr = (Donor *)realloc(arr, sizeof(Donor) * (size_t)cap);
    }

    Donor d;
    d.id = count;
    safe_copy(d.blood_group, MAX_BLOOD, blood);
    d.condition_score = atoi(cstr);
    int h1 = atoi(hstr);
    d.hospital_id_0 = h1 > 0 ? (h1 - 1) : 0;
    d.consent = parse_int_or_default(n >= 4 ? cons : NULL, 1);
    d.screening = parse_int_or_default(n >= 5 ? scr : NULL, 1);
    safe_copy(d.organ, MAX_ORGAN, (n >= 6 ? organ : "KIDNEY"));
    d.age = parse_int_or_default(n >= 7 ? age_str : NULL, 30);
    safe_copy(d.gender, 16, (n >= 8 ? gender : "Other"));
    d.weight = parse_int_or_default(n >= 9 ? weight_str : NULL, 70);

    arr[count++] = d;
  }

  fclose(f);
  *out = arr;
  *out_count = count;
  return 1;
}
