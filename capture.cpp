#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <pulse/error.h>
#include <pulse/simple.h>

using namespace std;

const int SAMPLE_RATE = 48000;
const int CHANNELS = 2;
const int BUFFER_FRAMES = 1024; // Number of frames per read
const int BUFFER_SIZE = BUFFER_FRAMES * CHANNELS;

pa_simple *paStream = nullptr;

// Replace this with your actual monitor source from `pactl list short sources`
const char *monitor_source_name = "bluez_output.B7_01_B7_EE_19_CF.1.monitor";

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

bool getAudioData(int32_t *buffer, size_t frames) {
  int error;
  size_t bytes = frames * CHANNELS * sizeof(int32_t);
  if (pa_simple_read(paStream, buffer, bytes, &error) < 0) {
    cerr << "pa_simple_read() failed: " << pa_strerror(error) << endl;
    return false;
  }
  return true;
}

int main(int argc, char *argv[]) {
  if (openPulseAudio() != 0) {
    return -1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    cerr << "SDL_Init Error: " << SDL_GetError() << endl;
    closePulseAudio();
    return -1;
  }

  SDL_Window *window =
      SDL_CreateWindow("Audio Capture Visualizer", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 800, 400, SDL_WINDOW_SHOWN);
  if (!window) {
    cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
    SDL_Quit();
    closePulseAudio();
    return -1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    closePulseAudio();
    return -1;
  }

  bool quit = false;
  SDL_Event e;

  int32_t audioBuffer[BUFFER_SIZE];

  while (!quit) {
    // Handle events
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
      if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_q) {
        quit = true;
      }
    }

    if (!getAudioData(audioBuffer, BUFFER_FRAMES)) {
      cerr << "Failed to get audio data, exiting..." << endl;
      break;
    }

    // Simple visualization: compute average absolute sample value for left and
    // right channels
    double leftSum = 0.0;
    double rightSum = 0.0;
    for (int i = 0; i < BUFFER_FRAMES; i++) {
      leftSum += abs(audioBuffer[2 * i]);      // Left channel sample
      rightSum += abs(audioBuffer[2 * i + 1]); // Right channel sample
    }
    double leftAvg = leftSum / BUFFER_FRAMES;
    double rightAvg = rightSum / BUFFER_FRAMES;

    // Debug print average amplitude
    cout << "Left avg: " << leftAvg << ", Right avg: " << rightAvg << "\r"
         << flush;

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw left channel bar
    int leftBarHeight =
        min(400,
            int(leftAvg / 10000)); // scale amplitude to pixels (adjust divisor)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect leftRect = {100, 400 - leftBarHeight, 50, leftBarHeight};
    SDL_RenderFillRect(renderer, &leftRect);

    // Draw right channel bar
    int rightBarHeight = min(400, int(rightAvg / 10000));
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_Rect rightRect = {200, 400 - rightBarHeight, 50, rightBarHeight};
    SDL_RenderFillRect(renderer, &rightRect);

    SDL_RenderPresent(renderer);

    SDL_Delay(16); // ~60 FPS
  }

  cout << endl;
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  closePulseAudio();
  return 0;
}
