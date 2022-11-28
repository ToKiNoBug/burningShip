#include "renders.h"

#include <colors.h>

#include <math.h>

#include <stdio.h>

void smooth_by_norm2(const mat_age *const age, const norm2_matc1 *const norm2,
                     mat_age_f32 *const dest) {
  //if (age == NULL || norm2 == NULL || dest == NULL)
    //return;

  static const float inv_log2_log2_6 =
      0.7298506383143850140271524651325307786464691162109375;
  ;
#pragma omp parallel for schedule(static)
  for (int r = 0; r < burning_ship_rows; r++) {
    for (int c = 0; c < burning_ship_cols; c++) {

      if (age->data[r][c] >= 0) {
        const float log2_norm_z = log2f(norm2->norm2[r][c]) / 2;
        const float log2_log2_norm_z = log2f(fabsf(log2_norm_z));

        dest->data[r][c] = log2_log2_norm_z * inv_log2_log2_6;
      } else {
        dest->data[r][c] = sqrtf(norm2->norm2[r][c] / 4);
      }
    }
  }
}
