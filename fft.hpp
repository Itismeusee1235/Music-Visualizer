
#include <bits/stdc++.h>
#include <cmath>
#include <complex>

using namespace std;
using cF = complex<float>;

void FFT(cF *time, cF *freq, int N);

void convertFtoCF(cF *out, float *in, int size);
void convertCFtoF(cF *out, float *in, int size);
void fourier(float *time, float *freq, int N);
