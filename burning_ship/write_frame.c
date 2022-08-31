#include "burning_ship.h"
#include <stdio.h>
#include <string.h>
#include <zlib.h>

bool write_uncompressed(const mat_age *img, const char *path) {

  if (strlen(path) <= 0)
    return false;

  FILE *ofile = fopen(path, "wb");

  if (ofile == NULL) {

    return false;
  }

  fwrite(img, 1, (burning_ship_rows * burning_ship_cols), ofile);

  return true;
}

bool write_compressed(const mat_age *img, const char *path) {
  gzFile zFile = gzopen(path, "wb");

  if (zFile == NULL) {
    return false;
  }

  gzfwrite(img, 1, (burning_ship_rows * burning_ship_cols), zFile);

  gzclose(zFile);

  return true;
}