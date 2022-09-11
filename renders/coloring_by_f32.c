#include "renders.h"
#include <colors.h>
#include <math.h>

const float nan_32 = NAN;

bool coloring_by_f32_u8c3(const mat_age *const age,
                          const mat_age_f32 *const smooth,
                          void *const dest_u8c3) {
  if (age == NULL || smooth == NULL || dest_u8c3 == NULL) {
    return false;
  }

  pixel_u8c3 *const dest = dest_u8c3;

#pragma omp parallel for schedule(static)
  for (int r = 0; r < burning_ship_rows; r++) {
    for (int c = 0; c < burning_ship_cols; c++) {
      const int idx = r * burning_ship_cols + c;
      if (age->data[r][c] < 0) {
        dest[idx] = copper_u8c3(smooth->data[r][c]);
      } else {

        dest[idx] = hot_u8c3(smooth->data[r][c]);
      }
    }
  }

  return true;
}

bool coloring_by_f32_u8c3_more(const mat_age *const age,
                               const mat_age_f32 *const smooth,
                               void *const dest_u8c3,
                               const float __lower_bound, // default nan
                               const float __upper_bound  // default nan
) {

  if (age == NULL || (smooth == NULL))
    return false;
  if (dest_u8c3 == NULL) {
    return false;
  }

  const bool compute_lower = _isnanf(__lower_bound);
  const bool compute_upper = _isnanf(__upper_bound);

  float lower_bound, upper_bound;

  bool is_buffer_provided_outside = false;

  if (!compute_upper) {
    upper_bound = __upper_bound;
  } else {
    upper_bound = smooth->data[0][0];
  }

  if (!compute_lower) {
    lower_bound = __lower_bound;
  } else {
    lower_bound = smooth->data[0][0];
  }
  // normalization
  if (compute_upper || compute_lower) {
    for (int r = 0; r < burning_ship_rows; r++) {
      for (int c = 0; c < burning_ship_cols; c++) {
        if (compute_upper)
          upper_bound = fmaxf(upper_bound, smooth->data[r][c]);
        if (compute_lower)
          lower_bound = fminf(lower_bound, smooth->data[r][c]);
      }
    }
  }

  if (upper_bound <= lower_bound) {
    return false;
  }

  const float inv_range = 1.0f / (upper_bound - lower_bound);

  pixel_u8c3 *const dest = dest_u8c3;
#pragma omp parallel for schedule(static)
  for (int r = 0; r < burning_ship_rows; r++) {
    for (int c = 0; c < burning_ship_cols; c++) {
      const int idx = r * burning_ship_cols + c;
      const float pos = (smooth->data[r][c] - lower_bound) * inv_range;
      // dest[idx] = copper_u8c3(pos);

      if (age->data[r][c] < 0) {
        dest[idx] = hot_u8c3(pos);
      } else {

        dest[idx] = bone_u8c3(pos);
      }
    }
  }

  return true;
}

bool coloring_by_f32_u8c1(const mat_age *const age,
                          const mat_age_f32 *const smooth,
                          void *const dest_u8c1) {
  if (age == NULL || smooth == NULL || dest_u8c1 == NULL) {
    return false;
  }

  uint8_t *const dest = dest_u8c1;

  const int16_t colormin = 32;
  const int16_t colormax = 255;

#pragma omp parallel for schedule(static)
  for (int r = 0; r < burning_ship_rows; r++) {
    for (int c = 0; c < burning_ship_cols; c++) {
      const int idx = r * burning_ship_cols + c;

      // const float temp = fminf(1.0f, fmaxf(0.0f, smooth->data[r][c]));
      const float temp = fminf(1.0f, smooth->data[r][c]);
      if (age->data[r][c] < 0) {
        dest[idx] = 0;
      } else {
        const int16_t val = (colormax - colormin) * temp + colormin;

        dest[idx] = val;
      }
    }
  }

  return true;
}
