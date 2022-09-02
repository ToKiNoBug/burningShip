#ifndef BURNING_SHIP_H
#define BURNING_SHIP_H

// Uncomment this line to generate mandelbrot fractal
//#define BS_MANDELBROT

//#define BS_FLOAT128

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef BS_FLOAT128

#include <quadmath.h>
typedef __float128 bs_float;
typedef __uint128_t bs_uint;
typedef __complex128 bs_cplx;

#else

typedef double bs_float;
typedef uint64_t bs_uint;
typedef double _Complex bs_cplx;

#endif

#define size_of_bs_float sizeof(bs_float)

// typedef __float128 bs_float;
// typedef __uint128_t bs_uint;

typedef struct {
  bs_float real;
  bs_float imag;
} my_complex;

typedef bs_cplx cplx_d;

typedef union {
  cplx_d value;
  bs_float fl[2];
  bs_uint ui[2];
  uint8_t bytes[sizeof(bs_float) * 2];
} cplx_union_d;

#define burning_ship_rows 1080
#define burning_ship_cols 1920

typedef struct {
  int16_t data[burning_ship_rows][burning_ship_cols];
} mat_age;

typedef struct {
  bs_float norm2[burning_ship_rows][burning_ship_cols];
} norm2_matc1;

typedef struct {
  cplx_d c3[burning_ship_rows][burning_ship_cols][3];
} cplx_matc3;

typedef struct {
  bs_cplx minmin;
  bs_cplx maxmax;
} bs_range_wind;

typedef struct {
  bs_cplx center;
  bs_float real_span;
  bs_float imag_span;
} bs_center_wind;

// here are functions to convert between different frame types

bs_range_wind to_range_wind(const bs_center_wind);

bs_center_wind to_center_wind(const bs_range_wind);

/// scale > 1 for zooming in, and scale < 1 for zooming away from center
void zoom_in_center(bs_center_wind *const, double scale);
/// scale > 1 for zooming in, and scale < 1 for zooming away from center
void zoom_in_range(bs_range_wind *const, double scale);

// here are functions to compute a frame

void compute_frame(mat_age *, const cplx_d minmin, const cplx_d maxmax,
                   const int16_t max_iterations);

void compute_frame_range(mat_age *, bs_range_wind,
                         const int16_t max_iterations);

void compute_frame_center(mat_age *, bs_center_wind,
                          const int16_t max_iterations);

void compute_frame_norm2c1_range(mat_age *const, bs_range_wind,
                                 const int16_t max_iteration,
                                 norm2_matc1 *const);

void compute_frame_norm2c1_center(mat_age *const, bs_center_wind,
                                  const int16_t max_iteration,
                                  norm2_matc1 *const);

void compute_frame_cplxmatc3_range(mat_age *const, bs_range_wind,
                                   const int16_t max_iteration,
                                   cplx_matc3 *const);

void compute_frame_cplxmatc3_center(mat_age *const, bs_center_wind,
                                    const int16_t max_iteration,
                                    cplx_matc3 *const);

// here are functions to save a frame

// the first 16 bytes are the rows and cols in size_t
bool write_uncompressed(const mat_age *, const char *);

// the first 16 bytes are the rows and cols in size_t
bool write_compressed(const mat_age *, const char *);

bool write_abstract_matrix(const void *const, const uint16_t element_bytes,
                           const uint64_t rows, const uint64_t cols,
                           const char *const filename, const bool compress);

// here are functions to render a frame

// render a u8c1 image linearly
void render_u8c1(const mat_age *const, uint8_t *const u8c1_row_major,
                 const int16_t max_iterations);

// check the size of bs_float,bs_uint and bs_cplx
bool check_sizes();

#ifdef __cplusplus
}
#if __cplusplus >= 201103L
static_assert(sizeof(bs_cplx) == 2 * sizeof(bs_float), "Size check failed!");
static_assert(sizeof(bs_float) == sizeof(bs_uint), "Size check failed!");
#endif

#endif // #ifdef __cplusplus

#endif // BURNING_SHIP_H