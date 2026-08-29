#ifndef IO_H
#define IO_H

#include "types.h"

int read_patients(const char *path, Patient **out, int *out_count);
int read_donors(const char *path, Donor **out, int *out_count);

#endif
