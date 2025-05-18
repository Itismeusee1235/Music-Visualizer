#include <bits/stdc++.h>
#include <cmath>
#include <complex>

using namespace std;

const float PI = 3.14159265358979323846f;

using cF = complex<float>;

void FFT(cF *time, cF *freq, int N) {
  if (N == 1) {
    freq[0] = time[0];
  } else {
    cF *even_time = new cF[N / 2];
    cF *odd_time = new cF[N / 2];

    for (int i = 0; i < N / 2; i++) {
      even_time[i] = time[2 * i];
    }
    for (int i = 0; i < N / 2; i++) {
      odd_time[i] = time[2 * i + 1];
    }

    cF twiddle = cF(1.0, 0);
    cF root = exp(cF(0, -2.0f * PI / N));

    cF *odd_freq = new cF[N / 2];
    cF *even_freq = new cF[N / 2];

    FFT(even_time, even_freq, N / 2);
    FFT(odd_time, odd_freq, N / 2);

    for (int i = 0; i < N / 2; i++) {
      freq[i] = even_freq[i] + twiddle * odd_freq[i];
      freq[i + N / 2] = even_freq[i] - twiddle * odd_freq[i];
      twiddle *= root;
    }

    delete[] even_time;
    delete[] even_freq;
    delete[] odd_time;
    delete[] odd_freq;
  }
}

int main() {

  cF *time = new cF[4];
  time[0] = cF(1, 0);
  time[1] = cF(0, 0);
  time[2] = cF(0, 0);
  time[3] = cF(0, 0);

  cF *freq = new cF[4];
  FFT(time, freq, 4);

  for (int i = 0; i < 4; i++) {
    cout << freq[i] << endl;
  }

  return 0;
}
