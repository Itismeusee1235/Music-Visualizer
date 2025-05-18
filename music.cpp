#include <SDL2/SDL.h>

int main() {

  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    return -1;
  }

  return 0;
}
