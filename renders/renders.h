#ifndef BS_RENDERS_H
#define BS_RENDERS_H

#include "burning_ship.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  float data[burning_ship_rows][burning_ship_cols];
} mat_age_f32;

extern const float nan_32;

void smooth_by_norm2(const mat_age *const age, const norm2_matc1 *const norm2,
                     mat_age_f32 *const dest);

// smooth should be in range [0,1]
bool coloring_by_f32_u8c3(const mat_age *const age,
                          const mat_age_f32 *const smooth,
                          void *const dest_u8c3);

// If the upper or lower bound is nan, they will be computed
bool coloring_by_f32_u8c3_more(const mat_age *const age,
                               const mat_age_f32 *const smooth,
                               void *const dest_u8c3,
                               const float lower_bound, // default nan
                               const float upper_bound  // default nan
);

#ifdef __cplusplus
}
#endif

#endif // BS_RENDERS_H