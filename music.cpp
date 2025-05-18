#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

int main() {

  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    printf("Error , %s", SDL_GetError());
    return -1;
  }

  SDL_AudioSpec wanted, have;
  SDL_AudioDeviceID device = NULL;
  SDL_memset(&wanted, 0, sizeof...(wanted));
  ee

      bool quit = false;
  while (!quit) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT) {
        quit = true;
      }
    }
  }

  SDL_Quit();

  return 0;
}
