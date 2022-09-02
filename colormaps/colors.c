#include "colors.h"

#include "color_sources.h"

#include <math.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

uint32_t autumn(float pos) {
  pos = fmin(pos, 1.0f);
  pos = fmax(pos, 0.0f);

  const float pos_v = pos * pos * (LUT_L - 1);
  const float pos_l = floorf(pos_v);
  const float pos_g = ceilf(pos_v);

  float c3[3];
  uint32_t u8[3];

  for (int i = 0; i < 3; i++) {
    c3[i] = src_autumn[(int)(pos_l)][i] +
            (src_autumn[(int)(pos_g)][i] - src_autumn[(int)(pos_l)][i]) /
                (pos_l - pos_g) * (pos_v - pos_l);

    u8[i] = 255 * fmax(c3[i], 0.0f);
    u8[i] = MIN(u8[i], 255U);
  }

  return (0xFFUL << 24) | (u8[0] << 16) | (u8[1] << 8) | (u8[2]);
}