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