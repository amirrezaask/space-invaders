#include "SDL2/SDL.h"
#include "cstdio"
#include "iostream"
#include "string"
#include "cstdlib"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define u8 Uint8

void check_error() {
    printf("%s", SDL_GetError());
    exit(1);
}

SDL_Surface* load_bmp_file(std::string path, SDL_PixelFormat* format) {
    SDL_Surface* raw_surface = SDL_LoadBMP(path.c_str());
    SDL_Surface* optimized = SDL_ConvertSurface(raw_surface, format, 0);
    SDL_FreeSurface(raw_surface);
    return optimized;
}

void set_background_color_for_surface(SDL_Surface* surface, u8 r, u8 g, u8 b) {
    if(SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF)) < 0 ) check_error();
}


int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
      check_error();



  SDL_Window* window = SDL_CreateWindow("Pinball", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  
  SDL_Surface* main_surface = SDL_GetWindowSurface(window);

  set_background_color_for_surface(main_surface, 0xFF, 0xFF, 0xFF);

  SDL_UpdateWindowSurface(window);

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
