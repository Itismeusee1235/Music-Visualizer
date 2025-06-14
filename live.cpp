#include "fft.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <pulse/error.h>
#include <pulse/simple.h>

using namespace std;

const int FRAME_RATE = 60;
const int SAMPLE_RATE = 48000;
const int CHANNELS = 2;
const int BUFFER_FRAMES = 1024; // Number of frames per read
const int BUFFER_SIZE = BUFFER_FRAMES * CHANNELS;

pa_simple *paStream = nullptr;

const char *monitor_source_name = "bluez_output.B7_01_B7_EE_19_CF.1.monitor";

void SetColor(SDL_Renderer *rend, int i) {

  int r = 0, g = 0, b = 0;

  if (i < 86) {
    float t = i / 85.0f;
    r = 255;
    g = t * 255;
    b = 0;
  } else if (i < 171) {
    float t = (i - 85) / 85.0f;
    r = (1.0f - t) * 255;
    g = 255;
    b = 0;
  } else {
    float t = (i - 170) / 85.0f;
    r = 0;
    g = (1.0f - t) * 255;
    b = t * 255;
  }

  SDL_SetRenderDrawColor(rend, r, g, b, 0xff);
}

void makeBins(int size, int num, int *bins) {
  int n = num + 1;
  for (int i = 0; i < n; i++) {
    float t = float(i) / (num * 1.0f);
    float ind = floor(pow(size, t));
    int val = min(size - 1, (int)floor(pow(size, t)) - 1);
    bins[i] = val;
  }
}

void renderMethod1(SDL_Renderer *rend, float *l_freq, float *r_freq, int w,
                   int start_index, float scale) {

  SDL_Rect rect{44, 0, w, 1};

  for (int i = 0; i < 256; i += 1) {

    SetColor(rend, i);

    int index = i + start_index;

    float height = log10(l_freq[index] + 1) * scale;
    rect.h = height;
    rect.x = i * 2 + 44;
    rect.y = 150 - (rect.h) / 2;
    SDL_RenderFillRect(rend, &rect);

    height = log10(r_freq[index] + 1) * scale;
    rect.h = height;
    rect.y = 450 - (rect.h) / 2;
    SDL_RenderFillRect(rend, &rect);
  }
  SDL_RenderPresent(rend);
}

void renderMethod2(SDL_Renderer *rend, float *l_freq, float *r_freq,
                   int *binsIndexs, int numBins, int w, float scale) {
  SDL_Rect rect{44, 0, w, 0};
  SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xFF);

  float r_h[numBins];
  float l_h[numBins];
  float maxFreq = l_freq[1];
  float minFreq = l_freq[0];

  for (int i = 0; i < numBins; i++) {
    int start = binsIndexs[i];
    int end = binsIndexs[i + 1];

    r_h[i] = 0;
    l_h[i] = 0;

    while (start <= end) {
      r_h[i] += r_freq[start];
      l_h[i] += l_freq[start];
      start++;
    }

    maxFreq = max(maxFreq, max(l_h[i], r_h[i]));
    minFreq = min(minFreq, min(l_h[i], r_h[i]));
  }
  for (int i = 0; i < numBins; i++) {
    rect.h = (l_h[i] - minFreq) * scale / (maxFreq - minFreq);
    rect.x = 44 + 16 * i;
    rect.y = 150 - (rect.h) / 1;
    SDL_RenderFillRect(rend, &rect);

    rect.h = (r_h[i] - minFreq) * scale / (maxFreq - minFreq);
    rect.y = 450 - (rect.h) / 1;
    SDL_RenderFillRect(rend, &rect);
  }
  SDL_RenderPresent(rend);
}

void createHamming(int N, float *hamming) {
  for (int i = 0; i < N; i++) {
    hamming[i] = 0.054f - 0.46f * cos(2.0f * M_PI * i / (N - 1));
  }
}

void applyHamming(float *hamming, cF *values, int N) {
  for (int i = 0; i < N; i++) {
    values[i] *= hamming[i];
  }
}
int openPulseAudio() {
  int error;
  pa_sample_spec ss;
  ss.format = PA_SAMPLE_S32LE; // 32-bit signed little endian (adjust if needed)
  ss.rate = SAMPLE_RATE;
  ss.channels = CHANNELS;

  paStream = pa_simple_new(nullptr, "PulseAudioCapture", PA_STREAM_RECORD,
                           monitor_source_name, "record", &ss, nullptr, nullptr,
                           &error);

  if (!paStream) {
    cerr << "pa_simple_new() failed: " << pa_strerror(error) << endl;
    return 1;
  }
  return 0;
}

void closePulseAudio() {
  if (paStream) {
    pa_simple_free(paStream);
    paStream = nullptr;
  }
}

bool getAudioData(float *buffer, size_t frames) {
  int error;
  int32_t int_buf[frames * CHANNELS];
  size_t bytes = frames * CHANNELS * sizeof(int32_t);
  if (pa_simple_read(paStream, int_buf, bytes, &error) < 0) {
    cerr << "pa_simple_read() failed: " << pa_strerror(error) << endl;
    return false;
  }

  for (int i = 0; i < frames * CHANNELS; i++) {
    buffer[i] = int_buf[i] / 2147483648.0f;
  }

  return true;
}

int main() {

  if (openPulseAudio() != 0) {
    cerr << "Failed to open PulseAudio stream" << endl;
    return -1;
  }
  int fftSize = 256;
  int numBins = 32;

  float *buffer = new float[fftSize * 2];
  float *l_freq = new float[fftSize * 2];
  float *r_freq = new float[fftSize * 2];
  float *hamming = new float[fftSize * 2];

  createHamming(2 * fftSize, hamming);

  int binsIndexs[numBins + 1];
  makeBins(fftSize, numBins, binsIndexs);
  for (int i = 0; i < numBins; i++) {
  }

  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    printf("Error , %s", SDL_GetError());
    return -1;
  }

  SDL_Window *win;
  SDL_Renderer *rend;
  win = SDL_CreateWindow("Music", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, 600, 600, 0);
  rend = SDL_CreateRenderer(win, -1, 0);

  bool quit = false;
  bool on = true;
  float scale = 45;

  cF *l_time = new cF[fftSize * 2];
  cF *r_time = new cF[fftSize * 2];

  cF *l_Freq = new cF[fftSize * 2];
  cF *r_Freq = new cF[fftSize * 2];

  float previous_time = SDL_GetTicks();
  float current_time = 0;

  int start_index = 0;

  while (!quit) {

    // printf("%d\n", scale);
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT) {
        quit = true;
      } else if (ev.type == SDL_KEYDOWN) {
        if (ev.key.keysym.sym == SDLK_q) {
          quit = true;
        }
        if (ev.key.keysym.sym == SDLK_UP) {
          scale += 1;
        } else if (ev.key.keysym.sym == SDLK_DOWN) {
          scale -= 1;
        } else if (ev.key.keysym.sym == SDLK_RIGHT) {
          start_index += 1;
          start_index = min(start_index, fftSize / 2 - 256);
        } else if (ev.key.keysym.sym == SDLK_LEFT) {
          start_index -= 1;
          start_index = max(start_index, 0);
        }
      }
    }
    cout << scale << endl;

    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rend, 0x0, 0x0, 0x0, 0xff);
    SDL_RenderClear(rend);

    getAudioData(buffer, fftSize);

    for (int i = 0; i < fftSize; i++) {
      float l, r;
      l = buffer[2 * i];
      r = buffer[2 * i + 1];
      l_time[i] = cF{l, 0};
      r_time[i] = cF{r, 0};
    }
    for (int i = fftSize; i < 2 * fftSize; i++) {
      l_time[i] = cF{0, 0};
      r_time[i] = cF{0, 0};
    }
    applyHamming(hamming, l_time, 2 * fftSize);
    applyHamming(hamming, r_time, 2 * fftSize);

    FFT(l_time, l_Freq, 2 * fftSize);
    FFT(r_time, r_Freq, 2 * fftSize);

    convertCFtoF(l_Freq, l_freq, 2 * fftSize);
    convertCFtoF(r_Freq, r_freq, 2 * fftSize);

    renderMethod2(rend, l_freq, r_freq, binsIndexs, numBins, 8, scale);
    // renderMethod1(rend, l_freq, r_freq, 2, start_index, scale);

    float delta_time = current_time - previous_time;

    if (delta_time < 1000 / FRAME_RATE) {
      SDL_Delay((1000.0f / FRAME_RATE) - delta_time);
    }
    previous_time = current_time;
  }

  SDL_Quit();
  closePulseAudio();
  return 0;
}
