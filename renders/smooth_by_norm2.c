#include "renders.h"

#include <colors.h>

#include <math.h>

void smooth_by_norm2(const mat_age *const age, const norm2_matc1 *const norm2,
                     mat_age_f32 *const dest) {
  if (age == NULL || norm2 == NULL || dest == NULL)
    return;
#pragma omp parallel for schedule(static)
  for (int r = 0; r < burning_ship_rows; r++) {
    for (int c = 0; c < burning_ship_cols; c++) {
      if (age->data[r][c] >= 0) {
        const float log2_norm_z_square = log2f((norm2->norm2[r][c]));
        const float log2_log2_norm_z = log2f(fabsf(log2_norm_z_square)) - 1.0f;
        dest->data[r][c] = log2_log2_norm_z / log2f(log2f(6.0f));
      } else {
        dest->data[r][c] = sqrtf(norm2->norm2[r][c] / 4);
      }
    }
  }
}
