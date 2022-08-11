build:
	c++ -std=c++17 -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lSDL2_Image -o space-invaders src/main.cpp



run: build
	./space-invaders
