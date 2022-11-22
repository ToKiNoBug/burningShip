#ifndef BS_RENDERS_H
#define BS_RENDERS_H

#include "burning_ship.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  float data[burning_ship_rows][burning_ship_cols];
} mat_age_f32;

extern const float nan_32;

void smooth_by_norm2(const mat_age *const age, const norm2_matc1 *const norm2,
                     mat_age_f32 *const dest);

// smooth should be in range [0,1]
bool coloring_by_f32_u8c1(const mat_age *const age,
                          const mat_age_f32 *const smooth,
                          void *const dest_u8c1);

// smooth should be in range [0,1]
bool coloring_by_f32_u8c3(const mat_age *const age,
                          const mat_age_f32 *const smooth,
                          void *const dest_u8c3);

// If the upper or lower bound is nan, they will be computed
bool coloring_by_f32_u8c3_more(const mat_age *const age,
                               const mat_age_f32 *const smooth,
                               void *const dest_u8c3,
                               const float lower_bound, // default nan
                               const float upper_bound  // default nan
);

// write png
bool write_png_u8c1(const void *const u8c1_rowmajor, const size_t rows,
                    const size_t cols, const char *const filename);

bool write_png_u8c1_rowptrs(const uint8_t *const *const u8c1_row_ptrs,
                            const size_t rows, const size_t cols,
                            const char *const filename);

bool write_png_u8c3(const void *const u8c3_rowmajor, const size_t rows,
                    const size_t cols, const char *const filename);

bool write_png_u8c3_rowptrs(const void *const *const u8c3_rows_ptrs,
                            const size_t rows, const size_t cols,
                            const char *const filename);

typedef struct {
  int newton_max_it;
  double err_tolerence;
  double L_mean_div_L_max;
  double q_guess; // q_guess<=0 for invalid value
  double *f_buffer;
  int hist_skip_rows;
  int hist_skip_cols;
} render_by_q_options;

double max_L_mean(const int maxit, const render_by_q_options *const opt);

void smooth_age_by_q(
    const mat_age *const age,
    const mat_age_f32 *const smoothed_by_norm2, // can be NULL
    const int16_t bs_maxit, // the max iteration when computing fractal
    const render_by_q_options *const opt, mat_age_f32 *const dest,
    double *const q_dest, double *const L_mean_dest);

typedef struct {
  int newton_max_it;
  double q_guess; // q_guess<=0 for invalid value
  double *f_buffer;
  int hist_skip_rows;
  int hist_skip_cols;
} render_entropy_options;

void smooth_age_by_q_entropy(
    const mat_age *const age,
    const mat_age_f32 *const smoothed_by_norm2, // can be NULL
    const int16_t bs_maxit, // the max iteration when computing fractal
    const render_entropy_options *const opt, mat_age_f32 *const dest,
    double *const q_dest);

#ifdef __cplusplus
}

#endif

#endif // BS_RENDERS_H