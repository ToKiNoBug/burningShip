#include "burning_ship.h"
#include <complex.h>
#include <stdio.h>
#include <string.h>


#include "inline.h"

bool check_sizes() {
  printf("Checking the size of bs_uint,bs_float and bs_cplx...\n");
  if (sizeof(bs_cplx) != 2 * sizeof(bs_float)) {
    printf("Size check failed! Size of bs_cplx is %llu, while size of bs_float "
           "is %llu\n",
           sizeof(bs_cplx), sizeof(bs_float));
    return false;
  }

  if (sizeof(bs_float) != sizeof(bs_uint)) {
    printf("Size check failed! Size of bs_float is %llu, while size of bs_uint "
           "is %llu\n",
           sizeof(bs_float), sizeof(bs_uint));
    return false;
  }

  printf("Size check succeed, size of bs_float is %llu\n", sizeof(bs_float));
  return true;
}

BS_INLINE bs_float bs_real(cplx_d v) { return *((bs_float *)(&v) + 0); }
BS_INLINE bs_float bs_imag(cplx_d v) { return *((bs_float *)(&v) + 1); }

BS_INLINE void iterate(const cplx_d C, cplx_union_d *z) {
  const uint8_t mask = 0b0111111;

#ifndef BS_MANDELBROT
  z->bytes[size_of_bs_float - 1] &= mask;
  z->bytes[2 * size_of_bs_float - 1] &= mask;
#endif
  z->value = z->value * z->value + C;

  // const int size=sizeof(val);
}

BS_INLINE bool is_norm_less_than_4(const cplx_union_d z) {
  /*
const bs_float *real = ((bs_float *)(&z));
const bs_float *imag = ((bs_float *)(&z) + 1);
return (*real * *real + *imag * *imag) < 4.0;
*/
  return (z.fl[0] * z.fl[0] + z.fl[1] * z.fl[1]) < 4.0;
}

BS_INLINE int16_t compute_age(const cplx_d C, const int16_t max_iteration) {
  int16_t counter = 0;
  cplx_union_d z;
  z.value = C;

  while (is_norm_less_than_4(z) && counter <= max_iteration) {
    iterate(C, &z);
    counter++;
  }

  return (counter >= max_iteration) ? (-1) : (counter);
}

// compute the age and return the norm2 of last numbers
BS_INLINE int16_t compute_age_norm2c1(const cplx_d C,
                                      const int16_t max_iteration,
                                      bs_float *const final_norm2) {
  int16_t counter = 0;
  cplx_union_d z;
  z.value = C;

  while (is_norm_less_than_4(z) && counter <= max_iteration) {
    iterate(C, &z);
    counter++;
  }
  //*final = z.value;
  *final_norm2 = z.fl[0] * z.fl[0] + z.fl[1] * z.fl[1];

  return (counter >= max_iteration) ? (-1) : (counter);
}

// compute the age and return the last 3 numbers
BS_INLINE int16_t compute_age_cplxmatc3(const cplx_d C,
                                        const int16_t max_iteration,
                                        cplx_d *const dest_c3) {
  int16_t counter = 0;
  cplx_union_d z[4];
  z[0].value = C;

  while (is_norm_less_than_4(z[counter & 0b11]) && counter <= max_iteration) {
    z[(counter + 1) & 0b11] = z[counter & 0b11];
    iterate(C, &z[(counter + 1) & 0b11]);
    counter++;
  }
  //*final = z.value;

  if (counter < 3) {
    const int16_t counter_archive = counter;
    for (; counter >= 3; counter++) {
      z[counter + 1] = z[counter];
      iterate(C, &z[counter + 1]);
    }

    for (int16_t offset = -3; offset < 0; offset++) {
      dest_c3[offset + 3] = z[counter + offset].value;
    }

    return (counter_archive >= max_iteration) ? (-1) : (counter_archive);
  }

  for (int16_t offset = -3; offset < 0; offset++) {
    dest_c3[offset + 3] = z[(counter + offset) & 0b11].value;
  }

  return (counter >= max_iteration) ? (-1) : (counter);
}

void compute_frame(mat_age *m, const cplx_d minmin, const cplx_d maxmax,
                   const int16_t max_iterations) {
  // memset(m, 0, sizeof(mat));

  const bs_float row_span = bs_imag(maxmax) - bs_imag(minmin);
  const bs_float col_span = bs_real(maxmax) - bs_real(minmin);

#pragma omp parallel for schedule(dynamic)
  for (int r = 0; r < burning_ship_rows; r++) {
    // printf("%d", omp_get_thread_num());
    // int16_t cache[cache_size];
    for (int c = 0; c < burning_ship_cols; c++) {

      const cplx_d C = minmin + (col_span * (c)) / (burning_ship_cols - 1) +
                       (row_span * (burning_ship_rows - 1 - r)) /
                           (burning_ship_rows - 1) * 1i;

      // cache[c % cache_size]
      m->data[r][c] = compute_age(C, max_iterations);
    }
  }
}

void compute_frame_range(mat_age *mat, bs_range_wind wind,
                         const int16_t max_iterations) {
  compute_frame(mat, wind.minmin, wind.maxmax, max_iterations);
}

void compute_frame_center(mat_age *mat, bs_center_wind wind,
                          const int16_t max_iterations) {
  bs_range_wind rw = to_range_wind(wind);

  compute_frame_range(mat, rw, max_iterations);
}

void compute_frame_norm2c1_range(mat_age *const m, bs_range_wind range,
                                 const int16_t max_iteration,
                                 norm2_matc1 *const norm2) {

  const bs_float row_span = bs_imag(range.maxmax) - bs_imag(range.minmin);
  const bs_float col_span = bs_real(range.maxmax) - bs_real(range.minmin);

#pragma omp parallel for schedule(dynamic)
  for (int r = 0; r < burning_ship_rows; r++) {
    // printf("%d", omp_get_thread_num());
    // int16_t cache[cache_size];
    for (int c = 0; c < burning_ship_cols; c++) {

      const cplx_d C = range.minmin +
                       (col_span * (c)) / (burning_ship_cols - 1) +
                       (row_span * (burning_ship_rows - 1 - r)) /
                           (burning_ship_rows - 1) * 1i;

      // cache[c % cache_size]
      m->data[r][c] =
          compute_age_norm2c1(C, max_iteration, &norm2->norm2[r][c]);
    }
  }
}

void compute_frame_cplxmatc3_range(mat_age *const m, bs_range_wind range,
                                   const int16_t max_iteration,
                                   cplx_matc3 *const cplx_c3) {

  const bs_float row_span = bs_imag(range.maxmax) - bs_imag(range.minmin);
  const bs_float col_span = bs_real(range.maxmax) - bs_real(range.minmin);

#pragma omp parallel for schedule(dynamic)
  for (int r = 0; r < burning_ship_rows; r++) {
    // printf("%d", omp_get_thread_num());
    // int16_t cache[cache_size];
    for (int c = 0; c < burning_ship_cols; c++) {

      const cplx_d C = range.minmin +
                       (col_span * (c)) / (burning_ship_cols - 1) +
                       (row_span * (burning_ship_rows - 1 - r)) /
                           (burning_ship_rows - 1) * 1i;

      // cache[c % cache_size]
      m->data[r][c] =
          compute_age_cplxmatc3(C, max_iteration, cplx_c3->c3[r][c]);
    }
  }
}

void compute_frame_norm2c1_center(mat_age *const mat, bs_center_wind cw,
                                  const int16_t max_iteration,
                                  norm2_matc1 *const norm2) {
  compute_frame_norm2c1_range(mat, to_range_wind(cw), max_iteration, norm2);
}

void compute_frame_cplxmatc3_center(mat_age *const m, bs_center_wind cw,
                                    const int16_t max_iteration,
                                    cplx_matc3 *const cplx_c3) {
  compute_frame_cplxmatc3_range(m, to_range_wind(cw), max_iteration, cplx_c3);
}