#include "SDL2/SDL.h"
#include "cstdio"
#include "iostream"
#include "string"
#include "cstdlib"

void Check_Error() {
    printf("%s", SDL_GetError());
    exit(1);
}

SDL_Surface* loadBMP(std::string path) {
    auto surface = SDL_LoadBMP(path.c_str());
    return surface;
}

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT =  900;

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
      Check_Error();

  auto background = loadBMP("./hello_world.bmp");

  auto window = SDL_CreateWindow("Pinball", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

  auto main_surface = SDL_GetWindowSurface(window);

  if (SDL_BlitSurface(background, NULL, main_surface, NULL) < 0) Check_Error();

  while(true) {
      SDL_Event event;
      if (SDL_PollEvent(&event) > 0) {
          switch(event.type) {
          case SDL_QUIT: {
              return 0;
          }
          }
      };
      
  }
}
