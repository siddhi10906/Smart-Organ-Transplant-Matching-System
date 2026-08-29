#ifndef HEAP_H
#define HEAP_H

#include "types.h"

typedef struct {
  Patient *arr;
  int size;
  int cap;
} MaxHeap;

void heap_init(MaxHeap *h, int cap);
void heap_push(MaxHeap *h, Patient p);
int heap_empty(MaxHeap *h);
Patient heap_pop(MaxHeap *h);
void heap_free(MaxHeap *h);

#endif
