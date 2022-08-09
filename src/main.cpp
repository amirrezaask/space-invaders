#include "SDL2/SDL.h"
#include "cstdio"
#include "iostream"
#include <cstdlib>

void Check_Error() {
  printf("%s", SDL_GetError());
  exit(1);
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    Check_Error();


  
}
