#include "renders.h"
#include <assert.h>
#include <cstring>

#include <cmath>

inline double age_L(const double x, const double q) {
  return 2.0 / M_PI * std::atan(q * x * x);
}

void make_histogram(const mat_age *age, const int16_t maxit,
                    const int skip_rows, const int skip_cols, double *const f) {
  if (f == NULL) {
    return;
  }

  // x->resize(int(maxit) + 1);

  const int x_max = maxit;

  int non_converge_num = 0;

  for (int i = 0; i <= maxit; i++) {
    f[i] = 0.0;
  }

  for (int r = skip_rows; r < burning_ship_rows - skip_rows; r++) {
    for (int c = skip_cols; c < burning_ship_cols - skip_cols; c++) {
      if (age->data[r][c] >= 0) {
        f[age->data[r][c]]++;
      } else {
        non_converge_num++;
      }
    }
  }

  const int converge_num =
      burning_ship_rows * burning_ship_cols - non_converge_num;

  // f->resize(int(maxit) + 1);
  for (int idx = 0; idx <= maxit; idx++) {
    f[idx] /= double(converge_num);
  }
}

// g_dest and dq must't be NULL
inline void compute_g_dg(const double q, const double L_mean,
                         const int16_t maxit, const double *const f,
                         double *const g_dest, double *const dg_dest) {
  assert(f != NULL);
  assert(g_dest != NULL);
  assert(dg_dest != NULL);

  double g = 0, dg = 0;
  for (int idx = 0; idx <= maxit; idx++) {
    const double x_square = idx * idx;
    g += ::atan(q * x_square) * f[idx];
    dg += x_square / (1.0 + q * q * x_square * x_square) * f[idx];
  }

  g *= 2 / M_PI;
  dg *= 2 / M_PI;
  g -= L_mean;

  *g_dest = g;
  *dg_dest = dg;
}

inline double find_q(const double L_mean, const double *const f,
                     const int16_t maxit, const int newton_max_it = 3000,
                     const double err_tolerance = 1e-4,
                     const double *const q_guess = NULL) {
  double q = 0.5;
  if (q_guess != NULL) {
    q = *q_guess;
  }

  double g = 1e5, dg = 1e5;

  int it = 0;
  while ((fabs(g) >= fabs(err_tolerance)) && (it < newton_max_it)) {
    compute_g_dg(q, L_mean, maxit, f, &g, &dg);

    q -= g / dg;
    it++;
  }

  return q;
}

void smooth_age_by_q(
    const mat_age *const age, const mat_age_f32 *const smoothed_by_norm2,
    const int16_t bs_maxit, // the max iteration when computing fractal
    const render_by_q_options *const opt, mat_age_f32 *const dest,
    double *const q_dest) {
  if (opt->f_buffer == NULL || dest == NULL || age == NULL) {
    return;
  }

  int skip_rows = std::min(opt->hist_skip_rows, burning_ship_rows / 2);
  skip_rows = std::max(skip_rows, 0);
  int skip_cols = std::min(opt->hist_skip_cols, burning_ship_cols / 2);
  skip_cols = std::max(skip_cols, 0);

  make_histogram(age, bs_maxit, skip_rows, skip_cols, opt->f_buffer);

  const double *const q_guess_ptr =
      (opt->q_guess > 0) ? (&opt->q_guess) : (NULL);

  const double q = find_q(opt->L_mean, opt->f_buffer, bs_maxit,
                          opt->newton_max_it, opt->err_tolerence, q_guess_ptr);

  if (q_dest != NULL) {
    *q_dest = q;
  }

  for (int r = 0; r < burning_ship_rows; r++) {
    for (int c = 0; c < burning_ship_cols; c++) {
      if (age->data[r][c] >= 0) {
        if (smoothed_by_norm2 == NULL) {
          dest->data[r][c] = age_L(age->data[r][c], q);
        } else {

          dest->data[r][c] =
              age_L(age->data[r][c] + 1 - smoothed_by_norm2->data[r][c], q);
        }
      } else {

        if (smoothed_by_norm2 == NULL) {

          dest->data[r][c] = 0;
        } else {
          dest->data[r][c] = smoothed_by_norm2->data[r][c];
        }
      }
    }
  }
}