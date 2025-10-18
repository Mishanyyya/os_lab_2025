#ifndef SUM_UTILS_H
#define SUM_UTILS_H

#include <stdint.h>

struct SumArgs {
  int *array;
  int begin;
  int end;
};

int64_t Sum(const struct SumArgs *args);

#endif