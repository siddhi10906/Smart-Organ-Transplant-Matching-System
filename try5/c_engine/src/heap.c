#include <stdlib.h>
#include "heap.h"

static int patient_key(Patient p) {
  return p.urgency;
}

void heap_init(MaxHeap *h, int cap) {
  h->arr = (Patient *)malloc(sizeof(Patient) * (size_t)cap);
  h->size = 0;
  h->cap = cap;
}

static void swap(Patient *a, Patient *b) {
  Patient t = *a;
  *a = *b;
  *b = t;
}

void heap_push(MaxHeap *h, Patient p) {
  if (h->size >= h->cap) {
    int new_cap = h->cap * 2;
    if (new_cap < 8) new_cap = 8;
    h->arr = (Patient *)realloc(h->arr, sizeof(Patient) * (size_t)new_cap);
    h->cap = new_cap;
  }

  int i = h->size++;
  h->arr[i] = p;

  while (i != 0) {
    int parent = (i - 1) / 2;
    if (patient_key(h->arr[parent]) >= patient_key(h->arr[i])) break;
    swap(&h->arr[parent], &h->arr[i]);
    i = parent;
  }
}

int heap_empty(MaxHeap *h) { return h->size == 0; }

Patient heap_pop(MaxHeap *h) {
  Patient out = h->arr[0];
  h->arr[0] = h->arr[h->size - 1];
  h->size--;

  int i = 0;
  for (;;) {
    int l = 2 * i + 1;
    int r = 2 * i + 2;
    int largest = i;

    if (l < h->size && patient_key(h->arr[l]) > patient_key(h->arr[largest])) largest = l;
    if (r < h->size && patient_key(h->arr[r]) > patient_key(h->arr[largest])) largest = r;

    if (largest == i) break;
    swap(&h->arr[i], &h->arr[largest]);
    i = largest;
  }

  return out;
}

void heap_free(MaxHeap *h) {
  free(h->arr);
  h->arr = NULL;
  h->size = 0;
  h->cap = 0;
}
