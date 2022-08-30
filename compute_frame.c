#include "ship_iteration.h"
#include <omp.h>
#include <string.h>

inline void iterate(const cplx_d C, cplx_union_d *z) {

  static const uint64_t mask = ~(1ULL << 63);
  // take the abs value to real and imag
  /*
   *(uint64_t *)(z) &= mask;
   *((uint64_t *)(z) + 1) &= mask;
   */
  z->u64[0] &= mask;
  z->u64[1] &= mask;

  z->value = z->value * z->value + C;

  // const int size=sizeof(val);
}

bool is_norm_less_than_4(const cplx_union_d z) {
  /*
const double *real = ((double *)(&z));
const double *imag = ((double *)(&z) + 1);
return (*real * *real + *imag * *imag) < 4.0;
*/
  return (z.fl64[0] * z.fl64[0] + z.fl64[1] * z.fl64[1]) < 4.0;
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

void compute_frame(mat_age *m, const cplx_d minmin, const cplx_d maxmax,
                   const int16_t max_iterations) {
  // memset(m, 0, sizeof(mat));

  const double row_span = cimag(maxmax) - cimag(minmin);
  const double col_span = creal(maxmax) - creal(minmin);

  omp_set_num_threads(4);

#pragma omp parallel for schedule(dynamic)
  for (int r = 0; r < rows; r++) {
    // printf("%d", omp_get_thread_num());
    // int16_t cache[cache_size];
    for (int c = 0; c < cols; c++) {

      const cplx_d C = minmin + (col_span * (cols - 1 - c)) / (cols - 1) +
                       (row_span * (r)) / (rows - 1) * 1i;

      // cache[c % cache_size]
      m->data[r][c] = compute_age(C, max_iterations);
    }
  }
}