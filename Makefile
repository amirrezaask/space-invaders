build:
	c++ -std=c++17 -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lSDL2_Image -lSDL2_ttf -o space-invaders src/main.cpp



run: build
	./space-invaders
