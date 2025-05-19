#include "WavReader.hpp"
#include "fft.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

typedef struct {
  float *l_data;
  float *r_data;
  float *amp;
  int index;
  int totalFrames;
} udata;

void callback(void *userdata, Uint8 *stream, int len) {
  udata *info = (udata *)userdata;
  int num_frames = (len / sizeof(float)) / 2; // assuming 2 channels
  float *buf = (float *)stream;

  if (num_frames + info->index > info->totalFrames) {
    for (int i = 0; i < num_frames; i++) {
      buf[2 * i] = 0;
      buf[2 * i + 1] = 0;
      info->amp[i] = (buf[2 * i] + buf[2 * i + 1]) / 2;
    }
    // put noise
  } else {

    for (int i = 0; i < num_frames; i++) {
      buf[2 * i] = info->l_data[info->index];
      buf[2 * i + 1] = info->r_data[info->index];
      info->index++;
      info->amp[i] = (buf[2 * i] + buf[2 * i + 1]) / 2;
    }
  }
}

int main() {
  Wav wav("output.wav");
  WavHeader head = wav.getHeader();
  udata userdata;
  userdata.index = 0;
  userdata.totalFrames = wav.getFrames();
  userdata.amp = new float[512];
  float *freq = new float[512];
  wav.getData(userdata.l_data, userdata.r_data);

  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    printf("Error , %s", SDL_GetError());
    return -1;
  }

  SDL_AudioSpec wanted, have;
  SDL_AudioDeviceID device = NULL;
  SDL_memset(&wanted, 0, sizeof(wanted));

  wanted.format = AUDIO_F32SYS;
  wanted.samples = 512;
  wanted.channels = head.numChannels;
  wanted.freq = head.sampleRate;
  wanted.callback = callback;
  wanted.userdata = &userdata;

  device = SDL_OpenAudioDevice(nullptr, 0, &wanted, NULL, 0);
  if (device == 0) {
    printf("Failed to open device");
  }

  SDL_Window *win;
  SDL_Renderer *rend;
  win = SDL_CreateWindow("Music", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, 600, 600, 0);
  rend = SDL_CreateRenderer(win, -1, 0);

  SDL_PauseAudioDevice(device, 0);
  SDL_Rect rect{0, 0, 2, 1};

  bool quit = false;
  bool on = true;
  int scale = 20;

  while (!quit) {
    printf("%d\n", scale);
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT) {
        quit = true;
      } else if (ev.type == SDL_KEYDOWN) {
        if (ev.key.keysym.sym == SDLK_q) {
          quit = true;
        } else if (ev.key.keysym.sym == SDLK_SPACE) {

          if (on) {
            SDL_PauseAudioDevice(device, 1);
            on = false;
          } else {
            on = true;
            SDL_PauseAudioDevice(device, 0);
          }
        } else if (ev.key.keysym.sym == SDLK_UP) {
          scale += 1;
        } else if (ev.key.keysym.sym == SDLK_DOWN) {
          scale -= 1;
        }
      }
    }

    SDL_SetRenderDrawColor(rend, 0x0, 0x0, 0x0, 0xff);
    SDL_RenderClear(rend);
    SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xff);
    fourier(userdata.amp, freq, 512);
    for (int i = 0; i < 256; i++) {
      // float height = freq[i] * scale;
      // rect.h = log(height);
      float height = log10(freq[i] + 1) * scale;
      rect.h = height;
      rect.x = i * 2 + 44;
      rect.y = 300 - (rect.h / 2);
      SDL_RenderFillRect(rend, &rect);
    }
    SDL_RenderPresent(rend);
  }

  SDL_Quit();

  return 0;
}
