#include "renders.h"
#include <assert.h>
#include <cstring>

#include <cmath>

#include <cstdio>

#include <vector>

inline double age_L(const double x, const double q) {
  return 2.0 / M_PI * std::atan(q * x * x);
}

void make_histogram(const mat_age *age, const int16_t maxit,
                    const int skip_rows, const int skip_cols, double *const f) {
  if (f == nullptr) {
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

  const double div = 1.0 / (converge_num + 1e-2 * (maxit + 1));
  // f->resize(int(maxit) + 1);
  for (int idx = 0; idx <= maxit; idx++) {
    f[idx] += 1e-2;
    f[idx] *= div;
  }
}

inline double max_L_mean(const int16_t maxit, const double *const f) {
  double mLm = 0;
  for (int idx = 1; idx < maxit; idx++) {
    // idx=0 have no contribution to max L means
    const double x_square = idx * idx;
    mLm += f[idx];
  }

  return mLm * 2 / M_PI;
}

// g_dest and dq must't be nullptr
inline void compute_g_dg(const double q, const double L_mean,
                         const int16_t maxit, const double *const f,
                         double *const g_dest, double *const dg_dest) {
  assert(f != nullptr);
  assert(g_dest != nullptr);
  assert(dg_dest != nullptr);

  double g = 0, dg = 0;
  for (int idx = 0; idx <= maxit; idx++) {
    const double x_square = idx * idx;
    g += std::atan(q * x_square) * f[idx];
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
                     const double *const q_guess = nullptr,
                     const int recursive_counter = 2, bool *const ok = NULL) {

  if (ok != nullptr) {
    *ok = true;
  }

  if (recursive_counter < 0) {
    if (ok != nullptr)
      *ok = false;
    printf("Failed to find q after many many tries.\n");
    exit(1);
  }

  const double max_L_mean = ::max_L_mean(maxit, f);

  if (max_L_mean <= L_mean) {

    return std::numeric_limits<double>::infinity();
  }

  double q = L_mean;
  if (L_mean > 0.125 && q_guess != nullptr) {
    q = *q_guess;
  }
  const double q_start = q;

  // std::vector<double> q_history(newton_max_it + 1);
  // q_history.resize(0);

  double g = 1e5, dg = 1e5;

  int it = 0;
  while (it < newton_max_it) {
    // q_history.emplace_back(q);
    compute_g_dg(q, L_mean, maxit, f, &g, &dg);
    // printf("q=%f, g=%f, dg=%f\n", q, g, dg);

    // dg = (g_add - g_min) / (q_add - q_min);

    if (std::abs(g) <= std::abs(err_tolerance)) {
      break;
    }

    // q = q - g / dg;
    const double prev_q = q;
    q *= std::abs(q * dg / g);

    if ((!std::isnormal(q)) || (q <= 0)) {
      // q_start *= 0.75;

      static constexpr double q_start_candidates[] = {
          0.9, 1.1, 0.8,  1.2, 0.7,  1.3, 0.6, 1.4, 0.5,  1.5,
          0.4, 2,   0.33, 4,   0.25, 8,   0.1, 10,  1e-2, 100};
      for (int i = 0; i < sizeof(q_start_candidates) / sizeof(double); i++) {
        double q_s = q_start_candidates[i] * q_s;
        bool ok = true;
        const double temp_res =
            find_q(L_mean, f, maxit, newton_max_it, err_tolerance, &q_s,
                   recursive_counter - 1, &ok);
        if (ok) {
          return q;
        }
      }
      if (ok != nullptr)
        *ok = false;
      return 0;
      // printf("\n\n\nFatal error occured when iterating, q got to %f\n", q);
      /*
      printf("The history of q :[");
      for (double val : q_history) {
        printf("%f, ", val);
      }

      printf("];\n");*/

      // exit(1);
    }
    const double r_diff_q = std::abs(prev_q - q) / std::abs(prev_q);
    if (r_diff_q < 1e-4) {
      break;
    }

    // q = std::max(q, std::nextafter(0.0, 1.0));
    it++;
  }

  return q;
}

void smooth_age_by_q(
    const mat_age *const age, const mat_age_f32 *const smoothed_by_norm2,
    const int16_t bs_maxit, // the max iteration when computing fractal
    const render_by_q_options *const opt, mat_age_f32 *const dest,
    double *const q_dest) {
  if (opt->f_buffer == nullptr || dest == nullptr || age == nullptr) {
    return;
  }

  int skip_rows = std::min(opt->hist_skip_rows, burning_ship_rows / 2);
  skip_rows = std::max(skip_rows, 0);
  int skip_cols = std::min(opt->hist_skip_cols, burning_ship_cols / 2);
  skip_cols = std::max(skip_cols, 0);

  make_histogram(age, bs_maxit, skip_rows, skip_cols, opt->f_buffer);

  const double *const q_guess_ptr =
      (opt->q_guess > 0) ? (&opt->q_guess) : (nullptr);

  const double q = find_q(opt->L_mean, opt->f_buffer, bs_maxit,
                          opt->newton_max_it, opt->err_tolerence, q_guess_ptr);

  if (q_dest != nullptr) {
    *q_dest = q;
  }

  for (int r = 0; r < burning_ship_rows; r++) {
    for (int c = 0; c < burning_ship_cols; c++) {
      if (age->data[r][c] >= 0) {
        if (smoothed_by_norm2 == nullptr) {
          dest->data[r][c] = age_L(age->data[r][c], q);
        } else {

          dest->data[r][c] =
              age_L(age->data[r][c] + 1 - smoothed_by_norm2->data[r][c], q);
        }
      } else {

        if (smoothed_by_norm2 == nullptr) {

          dest->data[r][c] = 0;
        } else {
          dest->data[r][c] = smoothed_by_norm2->data[r][c];
        }
      }
    }
  }
}