#ifndef BURNING_SHIP_H
#define BURNING_SHIP_H

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

typedef double bs_float;
typedef uint64_t bs_uint;
typedef double _Complex bs_cplx;

static const int size_of_bs_float = sizeof(bs_float);

// typedef __float128 bs_float;
// typedef __uint128_t bs_uint;

typedef struct {
  bs_float real;
  bs_float imag;
} my_complex;

typedef bs_cplx cplx_d;

typedef union {
  cplx_d value;
  bs_float fl64[2];
  bs_uint u64[2];
  uint8_t bytes[sizeof(bs_float) * 2];
} cplx_union_d;

#define burning_ship_rows 480
#define burning_ship_cols 480

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