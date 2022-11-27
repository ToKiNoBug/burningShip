#include "burning_ship.h"

#include <math.h>

#include <complex.h>

#include <stddef.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

bool render_u8c1(const mat_age *const src, uint8_t *const dest,
                 const int16_t max_iterations) {

  static const uint8_t color_of_negative_1 = 0;
  static const uint8_t color_min = 31;
  static const uint8_t color_max = 255;
  static const uint8_t color_span = color_max - color_min;

  if (src == NULL || dest == NULL) {
    return false;
  }

  int16_t max_positive = src->data[0][0];
  int16_t min_positive = src->data[0][0];

  for (int r = 0; r < burning_ship_rows; r++) {
    for (int c = 0; c < burning_ship_cols; c++) {
      if (src->data[r][c] >= 0) {
        max_positive = MAX(max_positive, src->data[r][c]);
        min_positive = MIN(min_positive, src->data[r][c]);
      }
    }
  }

  const int16_t span_positive = MAX(max_positive - min_positive, 1);

  //#pragma omp parallel for schedule(static)
  for (int r = 0; r < burning_ship_rows; r++) {
    for (int c = 0; c < burning_ship_cols; c++) {
      int idx = r * burning_ship_cols + c;
      if (src->data[r][c] <= -1) {
        dest[idx] = color_of_negative_1;
      } else {
        dest[idx] =
            color_span * (src->data[r][c] - min_positive) / span_positive +
            color_min;
      }
    }
  }

  return true;
}