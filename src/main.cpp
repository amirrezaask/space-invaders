#include "SDL2/SDL.h"
#include "SDL2/SDL_Image.h"
#include "SDL2/SDL_pixels.h"
#include "cstdio"
#include "iostream"
#include "string"
#include "cstdlib"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define u8 Uint8

SDL_PixelFormat* PIXEL_FORMAT;

void check_error() {
    printf("%s", SDL_GetError());
    exit(1);
}

SDL_Surface* optimize_surface(SDL_Surface* surface, SDL_PixelFormat* format) {
    SDL_Surface* optimized = SDL_ConvertSurface(surface, format, 0);
    SDL_FreeSurface(surface);
    return optimized;
}

void initiate_image_subsystem() {
    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags)) {
        printf("%s", IMG_GetError());
        exit(1);
    }
}

SDL_Surface* load_png(std::string path) {
    SDL_Surface* surface = IMG_Load(path.c_str());

    return optimize_surface(surface, PIXEL_FORMAT);
}

SDL_Surface* load_bmp_file(std::string path) {
    SDL_Surface* raw_surface = SDL_LoadBMP(path.c_str());
    return optimize_surface(raw_surface, PIXEL_FORMAT);
}

void set_background_color_for_surface(SDL_Surface* surface, u8 r, u8 g, u8 b) {
    if(SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, r, g, b)) < 0 ) check_error();
}

SDL_Surface* create_player_ship() {
    SDL_Surface* player_surface = load_png("./assets/player.png");

    return player_surface;
}


int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
      check_error();

  SDL_Window* window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  
  SDL_Surface* main_surface = SDL_GetWindowSurface(window);
  PIXEL_FORMAT = main_surface->format;

  SDL_Surface* player_ship = create_player_ship();
  
  // game background
  set_background_color_for_surface(main_surface, 0, 0, 0);
  
  SDL_BlitSurface(player_ship, NULL, main_surface, NULL);
  
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
