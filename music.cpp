#include "WavReader.hpp"
#include "fft.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

struct buf {
  float *data;
  int front;
  int back;
  int size;
  int count;

  buf(int n) {
    size = n;
    data = new float[n];
    front = 0;
    back = 0;
  };

  ~buf() { delete[] data; };

  void put(float value) {
    data[back] = value;
    back = (back + 1) % size;
    if (size == count) {
      front = (front + 1) % size;
    } else {
      count++;
    }
  };

  int pull(float &value) {
    if (count <= 0) {
      return -1;
    }
    value = data[front];
    front = (front + 1) % size;
    count--;
    return 0;
  };

  void getRecent(int n) {
    count += n;
    front = ((size + back) - n) % size;
  }
};

struct udata {
  float *l_data;
  float *r_data;
  buf l_buf;
  buf r_buf;
  int index;
  int totalFrames;

  udata(int n, int buf_size, int s) : l_buf(buf_size), r_buf(buf_size) {
    totalFrames = n;
    index = 0;
    l_data = new float[s];
    r_data = new float[s];
  };
  ~udata() {
    delete[] l_data;
    delete[] r_data;
  }
};

void callback(void *userdata, Uint8 *stream, int len) {
  udata *info = (udata *)userdata;
  int num_frames = (len / sizeof(float)) / 2; // assuming 2 channels
  float *buf = (float *)stream;

  if (num_frames + info->index > info->totalFrames) {
    for (int i = 0; i < num_frames; i++) {
      buf[2 * i] = 0;
      buf[2 * i + 1] = 0;
      info->l_buf.put(0);
      info->r_buf.put(0);
    }
  } else {
    for (int i = 0; i < num_frames; i++) {
      float l = info->l_data[info->index];
      float r = info->r_data[info->index];
      buf[2 * i] = l;
      buf[2 * i + 1] = r;
      info->index++;
      info->l_buf.put(l);
      info->r_buf.put(r);
    }
  }
}

int main() {

  Wav wav("output.wav");
  WavHeader head = wav.getHeader();

  udata userdata(wav.getFrames(), 1024, 1024);

  float *l_freq = new float[1024];
  float *r_freq = new float[1024];

  wav.getData(userdata.l_data, userdata.r_data);

  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    printf("Error , %s", SDL_GetError());
    return -1;
  }

  SDL_AudioSpec wanted, have;
  SDL_AudioDeviceID device = NULL;
  SDL_memset(&wanted, 0, sizeof(wanted));

  wanted.format = AUDIO_F32SYS;
  wanted.samples = 2048;
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
  int scale = 80;

  while (!quit) {
    // printf("%d\n", scale);
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

    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rend, 0x0, 0x0, 0x0, 0xff);
    SDL_RenderClear(rend);
    SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xff);

    cF *l_time = new cF[1024];
    cF *r_time = new cF[1024];

    for (int i = 0; i < 1024; i++) {
      float l, r;
      if (userdata.l_buf.pull(l) != 0) {
        userdata.l_buf.getRecent(1024);
        userdata.l_buf.pull(l);
      }
      if (userdata.r_buf.pull(r) != 0) {

        userdata.r_buf.getRecent(1024);
        userdata.r_buf.pull(r);
      }

      l_time[i] = cF{l, 0};
      r_time[i] = cF{r, 0};
    }

    cF *l_Freq = new cF[1024];
    cF *r_Freq = new cF[1024];

    FFT(l_time, l_Freq, 1024);
    FFT(r_time, r_Freq, 1024);

    convertCFtoF(l_Freq, l_freq, 1024);
    convertCFtoF(r_Freq, r_freq, 1024);

    for (int i = 0; i < 256; i += 1) {

      float height = log10(l_freq[2 * i] + l_freq[2 * i + 1] + 1) * scale;
      rect.h = height;

      rect.x = i * 2 + 44;
      rect.y = 150 - (rect.h) / 2;

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
      // SDL_SetRenderDrawColor(rend, 0xFF, 0, 0, 0x7F);
      SDL_RenderFillRect(rend, &rect);

      height = log10(r_freq[2 * i] + r_freq[2 * i + 1] + 1) * scale;
      rect.h = height;

      rect.x = i * 2 + 44;
      rect.y = 450 - (rect.h) / 2;

      // SDL_SetRenderDrawColor(rend, 0, 0xFF, 0, 0x7F);
      SDL_RenderFillRect(rend, &rect);
    }
    SDL_RenderPresent(rend);

    userdata.l_buf.getRecent(512);
    userdata.r_buf.getRecent(512);
  }

  SDL_Quit();

  return 0;
}
