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

#define burning_ship_rows 1080
#define burning_ship_cols 1920

typedef struct {
  int16_t data[burning_ship_rows][burning_ship_cols];
} mat_age;

void compute_frame(mat_age *, const cplx_d minmin, const cplx_d maxmax,
                   const int16_t max_iterations);

bool write_uncompressed(const mat_age *, const char *);

bool write_compressed(const mat_age *, const char *);
/*
typedef struct {
  uint8_t data[burning_ship_rows][burning_ship_cols];
} image_u16;

*/

void render(const mat_age *const, uint8_t *const, const int16_t max_iterations);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // BURNING_SHIP_H