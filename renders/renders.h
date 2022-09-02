#ifndef BS_RENDERS_H
#define BS_RENDERS_H

#include "burning_ship.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  float data[burning_ship_rows][burning_ship_cols];
} mat_age_f32;

void render_smooth_by_norm2(const mat_age *const age,
                            const norm2_matc1 *const norm2,
                            mat_age_f32 *const dest);

#ifdef __cplusplus
}
#endif

#endif // BS_RENDERS_H