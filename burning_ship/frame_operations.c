#include "burning_ship.h"
#include <complex.h>

bs_range_wind to_range_wind(const bs_center_wind cw) {
  bs_range_wind rw;
  cplx_union_d *rw_minmin = (cplx_union_d *)&rw.minmin;
  cplx_union_d *rw_maxmax = (cplx_union_d *)&rw.maxmax;

  rw.maxmax = cw.center;
  rw.minmin = cw.center;

  rw_minmin->fl[0] -= cw.real_span / 2;
  rw_maxmax->fl[0] += cw.real_span / 2;

  rw_minmin->fl[1] -= cw.imag_span / 2;
  rw_maxmax->fl[1] += cw.imag_span / 2;

  return rw;
}

bs_center_wind to_center_wind(const bs_range_wind rw) {
  bs_center_wind cw;

  cw.center = (rw.maxmax + rw.minmin) / 2;

  cplx_union_d range;
  range.value = rw.maxmax - rw.minmin;
  //  take the abs value of range's real and imag parts
  static const uint8_t mask = 0b0111111;
  range.bytes[size_of_bs_float - 1] &= mask;
  range.bytes[2 * size_of_bs_float - 1] &= mask;

  cw.real_span = range.fl[0];
  cw.imag_span = range.fl[1];

  return cw;
}

void zoom_in_center(bs_center_wind *const cwind, double scale) {
  if (scale < 0) {
    scale = -scale;
  }

  if (scale == 0) {
    return;
  }

  if (cwind == NULL) {
    return;
  }

  cwind->imag_span /= scale;
  cwind->real_span /= scale;
}

void zoom_in_range(bs_range_wind *const rwind, double scale) {
  bs_center_wind temp = to_center_wind(*rwind);
  zoom_in_center(&temp, scale);

  *rwind = to_range_wind(temp);
}