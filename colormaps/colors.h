#ifndef COLORMAPS_H
#define COLORMAPS_H

#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint8_t R;
  uint8_t G;
  uint8_t B;
} pixel_u8c3;

typedef uint32_t pixel_u8c4;

static const size_t LUT_L = 512;

pixel_u8c4 interpolate_u8c4(float pos, const float (*const LUT)[3]);
pixel_u8c3 interpolate_u8c3(float pos, const float (*const LUT)[3]);

pixel_u8c3 autumn_u8c3(const float pos);
pixel_u8c4 autumn_u8c4(const float pos);

pixel_u8c3 bone_u8c3(const float pos);
pixel_u8c4 bone_u8c4(const float pos);

pixel_u8c3 color_cube_u8c3(const float pos);
pixel_u8c4 color_cube_u8c4(const float pos);

pixel_u8c3 cool_u8c3(const float pos);
pixel_u8c4 cool_u8c4(const float pos);

pixel_u8c3 copper_u8c3(const float pos);
pixel_u8c4 copper_u8c4(const float pos);

pixel_u8c3 flag_u8c3(const float pos);
pixel_u8c4 flag_u8c4(const float pos);

pixel_u8c3 gray_u8c3(const float pos);
pixel_u8c4 gray_u8c4(const float pos);

pixel_u8c3 hot_u8c3(const float pos);
pixel_u8c4 hot_u8c4(const float pos);

pixel_u8c3 hsv_u8c3(const float pos);
pixel_u8c4 hsv_u8c4(const float pos);

pixel_u8c3 jet_u8c3(const float pos);
pixel_u8c4 jet_u8c4(const float pos);

pixel_u8c3 lines_u8c3(const float pos);
pixel_u8c4 lines_u8c4(const float pos);

pixel_u8c3 pink_u8c3(const float pos);
pixel_u8c4 pink_u8c4(const float pos);

pixel_u8c3 prism_u8c3(const float pos);
pixel_u8c4 prism_u8c4(const float pos);

pixel_u8c3 spring_u8c3(const float pos);
pixel_u8c4 spring_u8c4(const float pos);

pixel_u8c3 winter_u8c3(const float pos);
pixel_u8c4 winter_u8c4(const float pos);

#ifdef __cplusplus
}
#endif

#endif // COLORMAPS_H