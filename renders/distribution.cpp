#include <assert.h>

#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

#include "renders.h"

inline double age_L(const double x, const double q) {
  return 2.0 / M_PI * atan(q * x * x);
}

void make_histogram(const mat_age *age, const int16_t maxit,
                    const int skip_rows, const int skip_cols, double *const f,
                    int *const divergent_num_dest = nullptr) {
  if (f == nullptr) {
    return;
  }

  // x->resize(int(maxit) + 1);

  const int x_max = maxit;

  int divergent_num = 0;

  for (int i = 0; i <= maxit; i++) {
    f[i] = 0.0;
  }
  f[maxit]++;

  for (int r = skip_rows; r < burning_ship_rows - skip_rows; r++) {
    for (int c = skip_cols; c < burning_ship_cols - skip_cols; c++) {
      if (age->data[r][c] >= 0) {
        f[age->data[r][c]]++;
        divergent_num++;
      } else {
      }
    }
  }

  if (divergent_num_dest != nullptr) {
    *divergent_num_dest = divergent_num;
  }

  double sum = 0;
  for (int idx = 0; idx <= maxit; idx++) {
    f[idx] += 1e-2;
    sum += f[idx];
  }

  const double div = 1.0 / sum;
  // f->resize(int(maxit) + 1);
  for (int idx = 0; idx <= maxit; idx++) {
    f[idx] *= div;
  }
}

inline double max_L_mean(const int16_t maxit, const double *const f) {
  double mLm = 0;

  for (int idx = 1; idx <= maxit; idx++) {
    mLm += (f[idx] + f[idx - 1]) / 2;
  }

  return mLm;
}

double max_L_mean(const int maxit, const render_by_q_options *const opt) {
  return max_L_mean(maxit, opt->f_buffer);
}

inline double lim_h_div_q(const int16_t maxit, const double *const f) {
  double result = 0;
  double prev_fun_v = 0 * 0 * f[0];
  for (int idx = 1; idx <= maxit; idx++) {
    const double cur_fun_v = (idx * idx) * f[idx];
    result += (cur_fun_v + prev_fun_v) / 2;
    prev_fun_v = (idx * idx) * f[idx];
  }

  return result * 2 / M_PI;
}

inline void compute_h(const double q, const int16_t maxit,
                      const double *const f, double *const h_dest) {
  assert(f != nullptr);
  assert(h_dest != nullptr);

  double h = 0;
  double prev_fun_v = std::atan(q * 0 * 0) * f[0];
  for (int idx = 1; idx <= maxit; idx++) {
    const double cur_fun_c = std::atan(q * idx * idx) * f[idx];
    h += (cur_fun_c + prev_fun_v) / 2;
    prev_fun_v = cur_fun_c;
  }

  h *= 2 / M_PI;

  *h_dest = h;
}

// g_dest and dq must't be nullptr
inline void compute_g_dg(const double q, const double L_mean,
                         const int16_t maxit, const double *const f,
                         double *const g_dest, double *const dg_dest) {
  assert(f != nullptr);
  assert(g_dest != nullptr);
  assert(dg_dest != nullptr);

  double g = 0, dg = 0;
  double prev_v_g = std::atan(q * 0 * 0) * f[0];
  double prev_v_dg = 0 / (1.0 + q * q * 0 * 0) * f[0];
  for (int idx = 1; idx <= maxit; idx++) {
    const double x_square = idx * idx;
    const double cur_v_g = std::atan(q * x_square) * f[idx];
    const double cur_v_dg =
        x_square / (1.0 + q * q * x_square * x_square) * f[idx];
    g += (cur_v_g + prev_v_g) / 2;
    dg += (cur_v_dg + prev_v_dg);
    prev_v_g = cur_v_g;
    prev_v_dg = cur_v_dg;
  }

  g *= 2 / M_PI;
  dg *= 2 / M_PI;
  g -= L_mean;

  *g_dest = g;
  *dg_dest = dg;
}

inline double find_q_dichotomy(const double L_mean, const double *const f,
                               const int16_t maxit, const int newton_max_it,
                               const double error_tolerence,
                               const double q_guess = -1,
                               bool *const ok = nullptr) {
  double lnq_high = std::numeric_limits<double>::infinity();
  double lnq_low = -std::numeric_limits<double>::infinity();
  double lnq_mid = std::log(0.5);

  if (q_guess <= 0) {
    lnq_mid = std::log(L_mean) - std::log(::lim_h_div_q(maxit, f));
  } else {
    lnq_mid = std::log(q_guess);
  }

  // double h_high = ::max_L_mean(maxit, f), h_low = 0,
  double h_mid;
  ::compute_h(std::exp(lnq_mid), maxit, f, &h_mid);

  int it = 0;
  if (ok != nullptr) {
    *ok = false;
  }

  while (it <= newton_max_it) {
    // std::cout << "[" << (lnq_high) << ", " << (lnq_mid) << ", " <<
    // (lnq_low)<< "]  ";

    if (h_mid > L_mean) {
      // std::cout << "Updating mid and high\n";
      //  update lnq_high and corresponding h
      lnq_high = lnq_mid;
      // h_high = h_mid;
      //  update lnq_mid
      if (std::isfinite(lnq_low)) {
        lnq_mid = (lnq_mid + lnq_low) / 2;
      } else {
        lnq_mid -= std::log(10);
      }
    } else {
      // std::cout << "Updating mid and low\n";
      //  update lnq_low and corrresponding h
      lnq_low = lnq_mid;
      // h_low = h_mid;
      //  update lnq_mid
      if (std::isfinite(lnq_high)) {
        lnq_mid = (lnq_mid + lnq_high) / 2;
      } else {
        lnq_mid += std::log(10);
      }
    }

    ::compute_h(std::exp(lnq_mid), maxit, f, &h_mid);
    it++;
    if (std::abs((lnq_high - lnq_low)) <= 1e-9) {
      lnq_mid = (lnq_high + lnq_low) / 2;
      if (ok != nullptr) *ok = true;
      return std::exp(lnq_mid);
    }
  }

  if (std::isfinite(lnq_high) && std::isfinite(lnq_low)) {
    lnq_mid = (lnq_high + lnq_low) / 2;
    if (ok != nullptr) *ok = true;
    return std::exp(lnq_mid);
  }

  return 0;
}

inline double find_q_newton(const double L_mean, const double *const f,
                            const int16_t maxit, const int newton_max_it,
                            const double err_tolerance,
                            const double q_guess = -1,
                            bool *const ok = nullptr) {
  if (ok != nullptr) {
    *ok = false;
  }
  double q = 0.1;
  if (std::isfinite(q_guess) && (q_guess > 0)) {
    q = q_guess;
  } else {
    q = L_mean / ::lim_h_div_q(maxit, f);
  }

  double g, dg;

  int it = 0;

  while (it < newton_max_it) {
    ::compute_g_dg(q, L_mean, maxit, f, &g, &dg);
    const double prev_q = q;
    q *= std::abs(q * dg / g);

    if ((!std::isfinite(q)) || (q <= 0)) {
      return 0;
    }

    if (std::abs((prev_q - q) / prev_q) < err_tolerance) {
      if (ok != nullptr) *ok = true;
      break;
    }
  }

  return q;
}

void smooth_age_by_q(
    const mat_age *const age, const mat_age_f32 *const smoothed_by_norm2,
    const int16_t bs_maxit,  // the max iteration when computing fractal
    const render_by_q_options *const opt, mat_age_f32 *const dest,
    double *const q_dest, double *const L_mean_dest) {
  if (opt->f_buffer == nullptr || dest == nullptr || age == nullptr) {
    return;
  }

  int skip_rows = std::min(opt->hist_skip_rows, burning_ship_rows / 2);
  skip_rows = std::max(skip_rows, 0);
  int skip_cols = std::min(opt->hist_skip_cols, burning_ship_cols / 2);
  skip_cols = std::max(skip_cols, 0);

  int divergent_num = 0;
  make_histogram(age, bs_maxit, skip_rows, skip_cols, opt->f_buffer,
                 &divergent_num);

  double q = 0;
  if (divergent_num <= 0) {
    printf("All pixels are convergable.\n");
  } else {
    // find q
    if (opt->L_mean_div_L_max >= 1 || opt->L_mean_div_L_max <= 0) {
      printf("The value of L_mean_div_L_max is not in range (0,1)");
      exit(1);
    }
    const double L_mean =
        ::max_L_mean(bs_maxit, opt->f_buffer) * opt->L_mean_div_L_max;

    if (L_mean_dest != nullptr) {
      *L_mean_dest = L_mean;
    }

    bool ok = true;
    q = find_q_newton(L_mean, opt->f_buffer, bs_maxit, opt->newton_max_it,
                      opt->err_tolerence, opt->q_guess, &ok);
    if (!ok) {
      printf(
          "Newton method failed. Trying dichotomy method to get a better "
          "q_guess...\n");
      const double better_q_guess =
          find_q_dichotomy(L_mean, opt->f_buffer, bs_maxit, opt->newton_max_it,
                           opt->err_tolerence, opt->q_guess, &ok);
      if (!ok) {
        printf("Both newton and dichotomy method failed to find q\n");
        exit(1);
      } else {
        printf("Trying newton method with better q_guess(%e) again.\n",
               better_q_guess);
        q = find_q_newton(L_mean, opt->f_buffer, bs_maxit, opt->newton_max_it,
                          opt->err_tolerence, better_q_guess, &ok);
      }
    }
    if (!ok) {
      printf("Both newton and dichotomy method failed to find q\n");
      exit(1);
    }
  }
  //

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

bool find_q_newton_entropy(const double *const fx, const int16_t maxit,
                           double q_start, double &q_dest,
                           const int newton_maxit) {
  double lnq;
  if (q_start <= 0) {
    double ln_q_start = 0;
    for (int x = 1; x < maxit; x++) {
      ln_q_start += fx[x] * std::log(x);
    }
    ln_q_start *= -2 / (1 - fx[0]);
    lnq = ln_q_start;
  } else {
    lnq = std::log(q_start);
  }
  double prev_lnq = -1e9;
  double deta_dlnq = 0, ddeta_ddlnq = 0;

  int it = 0;

  while (it <= newton_maxit) {
    if (std::abs(lnq - prev_lnq) < 1e-5) {
      break;
    }

    deta_dlnq = 0;
    ddeta_ddlnq = 0;
    const double q = std::exp(lnq);
    for (int x = 1; x < maxit; x++) {
      const double q2x4 = q * q * x * x * x * x;

      deta_dlnq += fx[x] * (1 - q2x4) / (1 + q2x4);
      ddeta_ddlnq += fx[x] * -4 * q2x4 / (1 + q2x4);
    }

    prev_lnq = lnq;
    lnq = lnq - deta_dlnq / ddeta_ddlnq;

    it++;
  }

  q_dest = std::exp(lnq);

  if (it > newton_maxit)
    return false;
  else
    return true;
}

void smooth_age_by_q_entropy(
    const mat_age *const age,
    const mat_age_f32 *const smoothed_by_norm2,  // can be NULL
    const int16_t bs_maxit,  // the max iteration when computing fractal
    const render_entropy_options *const opt, mat_age_f32 *const dest,
    double *const q_dest) {
  if (opt->f_buffer == nullptr || dest == nullptr || age == nullptr) {
    return;
  }

  int skip_rows = std::min(opt->hist_skip_rows, burning_ship_rows / 2);
  skip_rows = std::max(skip_rows, 0);
  int skip_cols = std::min(opt->hist_skip_cols, burning_ship_cols / 2);
  skip_cols = std::max(skip_cols, 0);

  int divergent_num = 0;
  make_histogram(age, bs_maxit, skip_rows, skip_cols, opt->f_buffer,
                 &divergent_num);

  double q = 0;
  if (divergent_num <= 0) {
    printf("All pixels are convergable.\n");
  } else {
    // find q

    bool ok = true;
    ok = find_q_newton_entropy(opt->f_buffer, bs_maxit, opt->q_guess, q,
                               opt->newton_max_it);
    if (!ok) {
      printf("Entropy method failed to find q.\n");
      exit(1);
    }
  }
  // q *= 0.5;
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
  //
}