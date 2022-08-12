# Space Invaders

This is a clone of famous Space Invaders game.

# Features
- basic movement of player and enemies.
- enemies will shoot rocket randomly.

# Building
## Visual Studio
You just need to change the post build script to point to right address of SDL2.dll, SDL2_image.dll SDL2_ttf.dll.
and just press build, remember to edit project configuration to include sdl headers and also add lib path where sdl lib objects are.

## Makefile
having a C++ compiler and again include and lib paths.