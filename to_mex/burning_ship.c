#include "D:\Matlab\extern\include\mex.h"
#include <stdint.h>
#include <stdio.h>

#include "../burning_ship/burning_ship.h"

// struct mxArray;

void mexFunction(int outC, mxArray *outV[], int inC, const mxArray *inV[]) {
  /*
if (outC <= 0) {
  return;
}
*/

  if (inC <= 1) {
    printf(
        "Syntax : int16_t mat = burning_ship(Cminmin,Cmaxmax,max_iter=3000)\n");
    return;
  }

  int16_t max_iter = 3000;

  if (inC >= 3) {
    max_iter = *mxGetPr(inV[2]);
  }

  outV[0] = mxCreateNumericMatrix_800(rows, cols, mxINT16_CLASS, mxREAL);

  int16_t *ptr = mxGetData(outV[0]);

  cplx_union_d minmin = *(cplx_union_d *)mxGetPr(inV[0]),
               maxmax = *(cplx_union_d *)mxGetPr(inV[1]);

  printf("minmin = %f+%fi\n", minmin.fl64[0], minmin.fl64[1]);
  printf("maxmax = %f+%fi\n", maxmax.fl64[0], maxmax.fl64[1]);
}