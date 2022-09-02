#ifndef COLOR_SOURCES_H
#define COLOR_SOURCES_H
#include "colors.h"

typedef struct {
  uint8_t R;
  uint8_t G;
  uint8_t B;
} pixel;

const size_t LUT_L = 2048;

#ifdef __cplusplus
extern "C" {
#endif

extern const float (*const src_autumn)[3];
extern const float (*const src_bone)[3];
extern const float (*const src_color_cube)[3];
extern const float (*const src_cool)[3];
extern const float (*const src_copper)[3];
extern const float (*const src_flag)[3];
extern const float (*const src_gray)[3];
extern const float (*const src_hot)[3];
extern const float (*const src_hsv)[3];
extern const float (*const src_jet)[3];
extern const float (*const src_lines)[3];
extern const float (*const src_pink)[3];
extern const float (*const src_prism)[3];
extern const float (*const src_spring)[3];
extern const float (*const src_winter)[3];

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <vector>
namespace color_source {
extern const std::vector<float[3]> src_autumn;
extern const std::vector<float[3]> src_bone;
extern const std::vector<float[3]> src_color_cube;
extern const std::vector<float[3]> src_cool;
extern const std::vector<float[3]> src_copper;
extern const std::vector<float[3]> src_flag;
extern const std::vector<float[3]> src_gray;
extern const std::vector<float[3]> src_hot;
extern const std::vector<float[3]> src_hsv;
extern const std::vector<float[3]> src_jet;
extern const std::vector<float[3]> src_lines;
extern const std::vector<float[3]> src_pink;
extern const std::vector<float[3]> src_prism;
extern const std::vector<float[3]> src_spring;
extern const std::vector<float[3]> src_summer;
extern const std::vector<float[3]> src_winter;
} // namespace color_source
#endif

#endif //#ifndef COLOR_SOURCES_H