#include "burning_ship.h"
#include <omp.h>
#include <stdio.h>
#include <string.h>


bool check_sizes() {
  printf("Checking the size of bs_uint,bs_float and bs_cplx...\n");
  if (sizeof(bs_cplx) != 2 * sizeof(bs_float)) {
    printf("Size check failed! Size of bs_cplx is %u, while size of bs_float "
           "is %u\n",
           sizeof(bs_cplx), sizeof(bs_float));
    return false;
  }

  if (sizeof(bs_float) != sizeof(bs_uint)) {
    printf("Size check failed! Size of bs_float is %u, while size of bs_uint "
           "is %u\n",
           sizeof(bs_float), sizeof(bs_uint));
    return false;
  }

  printf("Size check succeed, size of bs_float is %u\n", sizeof(bs_float));
  return true;
}

void iterate(const cplx_d C, cplx_union_d *z) {
  static const uint8_t mask = 0b0111111;
  // take the abs value to real and imag
  /*
   *(bs_uint *)(z) &= mask;
   *((bs_uint *)(z) + 1) &= mask;
   */
  // z->u64[0] &= mask;
  // z->u64[1] &= mask;

  z->bytes[size_of_bs_float - 1] &= mask;
  z->bytes[2 * size_of_bs_float - 1] &= mask;

  z->value = z->value * z->value + C;

  // const int size=sizeof(val);
}

bool is_norm_less_than_4(const cplx_union_d z) {
  /*
const double *real = ((double *)(&z));
const double *imag = ((double *)(&z) + 1);
return (*real * *real + *imag * *imag) < 4.0;
*/
  return (z.fl[0] * z.fl[0] + z.fl[1] * z.fl[1]) < 4.0;
}

int16_t compute_age(const cplx_d C, const int16_t max_iteration) {
  int16_t counter = 0;
  cplx_union_d z;
  z.value = 0;

  while (is_norm_less_than_4(z) && counter <= max_iteration) {
    iterate(C, &z);
    counter++;
  }

  return (counter >= max_iteration) ? (-1) : (counter);
}

bs_float bs_real(cplx_d v) { return *((bs_float *)(&v) + 0); }
bs_float bs_imag(cplx_d v) { return *((bs_float *)(&v) + 1); }

void compute_frame(mat_age *m, const cplx_d minmin, const cplx_d maxmax,
                   const int16_t max_iterations) {
  // memset(m, 0, sizeof(mat));

  const double row_span = bs_imag(maxmax) - bs_imag(minmin);
  const double col_span = bs_real(maxmax) - bs_real(minmin);

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
