
Число кармайкла это такое целое составное число n, что (a, n) = 1; a^(n - 1) = 1(n) для любого целого a.
https://kconrad.math.uconn.edu/blurbs/ugradnumthy/carmichaelkorselt.pdf#:~:text=Definition%201,for%20all%20a%20%E2%88%88%20Z


 gcc main.c -o fermat_test \
  -I$(brew --prefix gmp)/include \
  -L$(brew --prefix gmp)/lib \
  -lgmp \
  -std=c99 -O2


