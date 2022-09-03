#include "colors.h"
#include "color_sources.h"

#include <math.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

uint32_t interpolate_u8c4(float pos, const float (*const LUT)[3]) {

  pos = fmin(pos, 1.0f);
  pos = fmax(pos, 0.0f);

  const float pos_v = pos * pos * (LUT_L - 1);
  const float pos_l = floorf(pos_v);
  const float pos_g = ceilf(pos_v);

  float c3[3];
  uint32_t u8[3];

  for (int i = 0; i < 3; i++) {
    c3[i] =
        LUT[(int)(pos_l)][i] + (LUT[(int)(pos_g)][i] - LUT[(int)(pos_l)][i]) /
                                   (pos_g - pos_l) * (pos_v - pos_l);

    u8[i] = 255 * fmax(c3[i], 0.0f);
    u8[i] = MIN(u8[i], 255U);
  }

  return (0xFFUL << 24) | (u8[0] << 16) | (u8[1] << 8) | (u8[2]);
}

pixel_u8c3 interpolate_u8c3(float pos, const float (*const LUT)[3]) {
  pos = fmin(pos, 1.0f);
  pos = fmax(pos, 0.0f);

  const float pos_v = pos * pos * (LUT_L - 1);
  const float pos_l = floorf(pos_v);
  const float pos_g = ceilf(pos_v);

  float c3[3];
  uint32_t u8[3];

  for (int i = 0; i < 3; i++) {
    c3[i] =
        LUT[(int)(pos_l)][i] + (LUT[(int)(pos_g)][i] - LUT[(int)(pos_l)][i]) /
                                   (pos_g - pos_l) * (pos_v - pos_l);

    u8[i] = 255 * fmax(c3[i], 0.0f);
    u8[i] = MIN(u8[i], 255U);
  }

  pixel_u8c3 result;
  result.R = u8[0];
  result.G = u8[1];
  result.B = u8[2];

  return result;
}

#define BS_make_function_implementation(channel_num, srcname)                  \
  pixel_u8c##channel_num srcname##_u8c##channel_num(const float pos) {         \
    return interpolate_u8c##channel_num(pos, src_##srcname);                   \
  };

#define BS_make_function_impl_c3c4(srcname)                                    \
  BS_make_function_implementation(3, srcname)                                  \
      BS_make_function_implementation(4, srcname)

BS_make_function_impl_c3c4(autumn)

    BS_make_function_impl_c3c4(bone)

        BS_make_function_impl_c3c4(color_cube)

            BS_make_function_impl_c3c4(cool)

                BS_make_function_impl_c3c4(copper)

                    BS_make_function_impl_c3c4(flag)

                        BS_make_function_impl_c3c4(gray)

                            BS_make_function_impl_c3c4(hot)

                                BS_make_function_impl_c3c4(hsv)

                                    BS_make_function_impl_c3c4(jet)

                                        BS_make_function_impl_c3c4(lines)

                                            BS_make_function_impl_c3c4(pink)

                                                BS_make_function_impl_c3c4(
                                                    prism)

                                                    BS_make_function_impl_c3c4(
                                                        spring)

                                                        BS_make_function_impl_c3c4(
                                                            winter)
