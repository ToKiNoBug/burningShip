#ifndef BURNING_SHIP_H
#define BURNING_SHIP_H

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  double real;
  double imag;
} my_complex;

typedef union {
  double _Complex value;
  double fl64[2];
  uint64_t u64[2];
} cplx_union_d;

typedef double _Complex cplx_d;

#define rows 1080
#define cols 1920

typedef struct {
  int16_t data[rows][cols];
} mat_age;

void compute_frame(mat_age *, const cplx_d minmin, const cplx_d maxmax,
                   const int16_t max_iterations);

bool write_uncompressed(const mat_age *, const char *);

bool write_compressed(const mat_age *, const char *);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // BURNING_SHIP_H