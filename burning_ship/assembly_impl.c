double __complex iterate_f64_mandelbrot(double __complex C,
                                        double __complex z) {
  // code below can't pass
  /*
  asm volatile("movapd xmm0 C"
               "movapd  xmm4, xmm0"
               "movapd  xmm0, xmm2"
               "mulsd   xmm0, xmm2"
               "mulsd   xmm2, xmm3"
               "addsd   xmm0, xmm4"
               "movapd  xmm4, xmm3"
               "mulsd   xmm4, xmm3"
               "addsd   xmm2, xmm2"
               "addsd   xmm1, xmm2"
               "subsd   xmm0, xmm4"
               "ret");

               */
  return 0;
}