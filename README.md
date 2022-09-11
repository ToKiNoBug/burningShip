# Burning ship fractal

This repo implementes generating, rendering and zooming the burning ship fractal.

## Compiler support
To zoom about over 1e-14, **float128** is required. Because this repo uses `__float128` and `_Complex128`, which is the compiler-extension of gcc, only gcc and a x64 arch is able to compile this project.

## Candidate zooming points:
Center in hex can be of bi-precision or quar-precision, and scale in height is the greatest zoom when the image is pixelized(no enough precision)

|       Center        |                           Center in hex                            | Scale in height |
| :-----------------: | :----------------------------------------------------------------: | --------------- |
| -0.591827-1.05382i  | 0xdddb3abfbc71ac5c49c65fea032ffebfbc022feb8231c7f4c255f375c70dffbf | 2.23408e-32     |
| -0.648157-0.899817i | 0xd21580ba4ea4b56dc5cbd83adb4bfebf36c5b5eb9e68eceab798b0d7b4ccfebf | 4.93038e-32     |
|  -0.7609-1.13011i   | 0xb3b1d59e2534ba138d0aa7a49485febfbc27a52ad5707df94eb39ff24e21ffbf | 3.62075e-32     |
| -0.758907-1.12959i  | 0xa3998341cd58aef7b7bb0e818f84febfdc497cc0aac9fbf4c79b84bd2c21ffbf | 4.93038e-32     |

## Format of exported binaries:
Currently there are 3+3 formats(3 not compressed and 3 gzipped). All of them stores some kind of matrix in row-major.

- If the file is not gzipped, datas are stored in following syntax:
  1. The first byte could be 8 or 16, tells the bytes of `bs_float`. If 16, it is `__float128`, otherwise `double`.
  2. The comming 16 bytes stores the rows and cols of a matrix in `uint64_t`.
  3. After that it is the matrix data. Type of element can be deducted from the file extension name.
- If the file is gzipped, inflate it and read the file as above.

|           Extension Name           | Element type | Descrption                                                      |
| :--------------------------------: | :----------: | :-------------------------------------------------------------- |
|   `.bs_frame` and `.bs_frame.gz`   |  `int16_t`   | The matrix of escape time. -1 for can't escapt.                 |
|   `.bs_norm2` and `.bs_norm2.gz`   |  `bs_float`  | The matrix of 2-norm when iteration terminates.                 |
| `.bs_cplx_c3` and `.bs_cplx_c3.gz` | `bs_cplx[3]` | The matrix of last 3 complex numbers when iteration terminates. |

## Sample commands:
```
bsCompute -centerhex 0x00000000000000000000000000000000 -framecount 16 -compress -preview -zoomspeed 4 -j 4 -mode norm2 -filenameprefix test/ -maxit 255
```