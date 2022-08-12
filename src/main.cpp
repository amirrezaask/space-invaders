#include "SDL2/SDL_timer.h"
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_main.h"
#include "SDL2/SDL_rect.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_keycode.h"
#include "SDL2/SDL_pixels.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_surface.h"
#include "cstdio"
#include "iostream"
#include "vector"
#include "string"
#include "cstdlib"
#include <cassert>
#include <cstdlib>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define PLAYER_SHIP_ASSET_PATH "./assets/player.png"
#define ROCKET_ASSET_PATH "./assets/rocket.png"
#define ENEMY1_SHIP_ASSET_PATH "./assets/enemy1.png"
#define ENEMY2_SHIP_ASSET_PATH "./assets/enemy2.png"
#define ENEMY3_SHIP_ASSET_PATH "./assets/enemy3.png"
#define ENEMY_GRID_COLS 10
#define ENEMY_GRID_ROWS 10
#define ENEMY_CHANCE_TO_MOVE 100
#define FONT_PATH "./assets/Go-Bold.ttf"
#define FONT_SIZE 120
#define MSG_WIN "You won"
#define MSG_LOST "You lost"

struct Ship;
struct Rocket;
bool quit = false;

SDL_Rect* ship_get_rect(Ship* ship);

SDL_Texture* rocket_texture;

SDL_Rect center_result_text_rect;

std::vector<Ship*> ships;
std::vector<Rocket*> rockets;

SDL_Renderer* main_renderer;
SDL_Texture* win_msg_texture;
SDL_Texture* loss_msg_texture;
SDL_Rect win_msg_rect;
SDL_Rect lost_msg_rect;




int random_number() {
	return rand() % (ENEMY_CHANCE_TO_MOVE + 9 + 1);
}

void check_error(std::string prefix) {
	std::cout << "Checking Error: " << prefix << ": " << SDL_GetError() << "\n";
	exit(1);
}


void make_texts() {
	TTF_Init();
	TTF_Font* font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
	if (!font) check_error("opening font");

	SDL_Color text_color = { 0xFF, 0xFF, 0xFF, 0xFF };

	SDL_Surface* win_surface = TTF_RenderText_Solid(font, MSG_WIN, text_color);
	if (!win_surface) check_error("creating win msg surface");
	SDL_Surface* lost_surface = TTF_RenderText_Solid(font, MSG_LOST, text_color);
	if (!lost_surface) check_error("creating lost msg surface");

	SDL_Texture* win_texture = SDL_CreateTextureFromSurface(main_renderer, win_surface);
	if (!win_texture) check_error("creating win msg texture");
	SDL_Texture* lost_texture = SDL_CreateTextureFromSurface(main_renderer, lost_surface);
	if (!lost_surface) check_error("creating lost msg texture");

	win_msg_rect.x = (WINDOW_WIDTH - win_surface->w) * 0.5; // Center horizontaly
	win_msg_rect.y = (WINDOW_HEIGHT - win_surface->h) * 0.5; // Center verticaly
	win_msg_rect.w = win_surface->w;
	win_msg_rect.h = win_surface->h;

	lost_msg_rect.x = (WINDOW_WIDTH - lost_surface->w) * 0.5; // Center horizontaly
	lost_msg_rect.y = (WINDOW_HEIGHT - lost_surface->h) * 0.5; // Center verticaly
	lost_msg_rect.w = lost_surface->w;
	lost_msg_rect.h = lost_surface->h;

	// After you create the texture you can release the surface memory allocation because we actually render the texture not the surface.
	SDL_FreeSurface(win_surface);
	SDL_FreeSurface(lost_surface);
	TTF_Quit();
}

SDL_Surface* optimize_surface(SDL_Surface* surface, SDL_PixelFormat* format) {
	SDL_Surface* optimized = SDL_ConvertSurface(surface, format, 0);
	SDL_FreeSurface(surface);
	return optimized;
}

SDL_Texture* load_png_as_texture(SDL_Renderer* render, std::string path) {
	SDL_Surface* surface = IMG_Load(path.c_str());
	if (!surface) check_error("loading png: " + path);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(render, surface);
	if (!texture) check_error("creating texture for png: " + path);
	return texture;

}

SDL_Window* init_main_window() {

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		check_error("initializing video subsystem");
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		printf("%s", IMG_GetError());
		exit(1);
	}

	SDL_Window* window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	return window;
}

// ------------------------------- Game stuff --------------------------------

enum Side {
	Side_Player,
	Side_Enemy,
};

enum GameResult {
	GameResult_Win,
	GameResult_Loss,
	GameResult_OnGoing
};

enum Direction {
	Direction_Up,
	Direction_Down,
	Direction_Left,
	Direction_Right,
	Direction_UpLeft,
	Direction_UpRight,
	Direction_DownLeft,
	Direction_DownRight,
	Direction_NoMove
};

Direction get_random_direction() {
	int num = random_number();
	switch (num) {
	case 1: return Direction_Up;
	case 2: return Direction_Down;
	case 3: return Direction_Left;
	case 4: return Direction_Right;
	case 5: return Direction_DownRight;
	case 6: return Direction_DownLeft;
	case 7: return Direction_UpRight;
	case 8: return Direction_UpLeft;
	default: return Direction_NoMove;
	}
}

struct Vector2 {
	int x, y;
};

struct Ship {
	Vector2 pos;
	int h;
	int w;
	SDL_Texture* texture;
	bool destroyed;
	Side side;

};

struct Rocket {
	Vector2 pos;
	SDL_Texture* texture;
	bool destroyed;
	Direction direction;
	Side side;
};

void make_rocket(int x, int y, Direction direction, Side side) {
	Rocket* rocket = new Rocket;
	Vector2 pos;
	pos.x = x;
	pos.y = y;
	rocket->pos = pos;
	rocket->side = side;
	rocket->texture = rocket_texture;
	rocket->direction = direction;
	rocket->destroyed = false;
	rockets.push_back(rocket);
}

void make_ship(int x, int y, int w, int h, SDL_Texture* texture, Side side) {
	Ship* ship = new Ship;
	Vector2 pos;
	pos.x = x;
	pos.y = y;
	ship->w = w;
	ship->h = h;
	ship->pos = pos;
	ship->side = side;
	ship->texture = texture;
	ship->destroyed = false;
	ships.push_back(ship);
}
bool player_can_move_to_this(Ship* ship, Direction direction) {
	int new_y;
	int new_x;
	switch (direction) {
	case Direction_Up:
		new_y = ship->pos.y - 20;
		if (new_y < 0) return false;
		return true;

	case Direction_Down:
		new_y = ship->pos.y + 20;
		if (new_y > WINDOW_HEIGHT - 100) return false;
		return true;

	case Direction_Right:
		new_x = ship->pos.x + 20;
		if (new_x > WINDOW_WIDTH - 100) return false;
		return true;

	case Direction_Left:
		new_x = ship->pos.x - 20;
		if (new_x < 0) return false;
		return true;
	case Direction_UpLeft:
		return player_can_move_to_this(ship, Direction_Up)
			&&
			player_can_move_to_this(ship, Direction_Left);

	case Direction_UpRight:
		return player_can_move_to_this(ship, Direction_Up)
			&&
			player_can_move_to_this(ship, Direction_Right);

	case Direction_DownLeft:
		return player_can_move_to_this(ship, Direction_Down)
			&&
			player_can_move_to_this(ship, Direction_Left);

	case Direction_DownRight:
		return player_can_move_to_this(ship, Direction_Down)
			&&
			player_can_move_to_this(ship, Direction_Right);
	case Direction_NoMove:
		return true;
	default:
		return false;
	}

}

bool enemy_can_move_to_this(Ship* ship, Direction direction) {
	int new_y;
	int new_x;
	switch (direction) {
	case Direction_Up:
		new_y = ship->pos.y - 20;
		if (new_y < 0) return false;
		return true;

	case Direction_Down:
		new_y = ship->pos.y + 20;
		if (new_y > (WINDOW_HEIGHT - (WINDOW_HEIGHT / 3))) return false;
		return true;

	case Direction_Right:
		new_x = ship->pos.x + 20;
		if (new_x > WINDOW_WIDTH - 100) return false;
		return true;

	case Direction_Left:
		new_x = ship->pos.x - 20;
		if (new_x < 0) return false;
		return true;
	case Direction_UpLeft:
		return enemy_can_move_to_this(ship, Direction_Up)
			&&
			enemy_can_move_to_this(ship, Direction_Left);

	case Direction_UpRight:
		return enemy_can_move_to_this(ship, Direction_Up)
			&&
			enemy_can_move_to_this(ship, Direction_Right);

	case Direction_DownLeft:
		return enemy_can_move_to_this(ship, Direction_Down)
			&&
			enemy_can_move_to_this(ship, Direction_Left);

	case Direction_DownRight:
		return enemy_can_move_to_this(ship, Direction_Down)
			&&
			enemy_can_move_to_this(ship, Direction_Right);
	case Direction_NoMove:
		return true;
	}
}
void move_ship(Ship* ship, Direction direction) {

	switch (direction) {
	case Direction_Up:
		ship->pos.y -= 20;
		break;
	case Direction_Down:
		ship->pos.y += 20;
		break;

	case Direction_Right:
		ship->pos.x += 20;
		break;
	case Direction_Left:
		ship->pos.x -= 20;
		break;
	case Direction_UpLeft:
		ship->pos.y -= 20;
		ship->pos.x -= 20;
		break;
	case Direction_UpRight:
		ship->pos.y -= 20;
		ship->pos.x += 20;
		break;
	case Direction_DownLeft:
		ship->pos.y += 20;
		ship->pos.x -= 20;
		break;
	case Direction_DownRight:
		ship->pos.y += 20;
		ship->pos.x += 20;
		break;

	case Direction_NoMove:
		break;
	}
}

void move_rocket(Rocket* rocket) {
	switch (rocket->direction) {
	case Direction_Up:
		rocket->pos.y -= 10;
		break;
	case Direction_Down:
		rocket->pos.y += 10;
		break;
	default:
		printf("unhandled direction for a rocket");
		exit(0);
	}
}

SDL_Rect* ship_get_rect(Ship* ship) {
	SDL_Rect* rect = new SDL_Rect;
	rect->x = ship->pos.x;
	rect->y = ship->pos.y;
	rect->h = ship->h;
	rect->w = ship->w;

	return rect;
}

SDL_Rect* get_rocket_rect(Rocket* rocket) {
	int h, w;
	if (SDL_QueryTexture(rocket_texture, NULL, NULL, &w, &h) < 0) check_error("cannot get rocket surface");
	SDL_Rect* rect = new SDL_Rect;
	rect->x = rocket->pos.x;
	rect->y = rocket->pos.y;
	rect->h = h;
	rect->w = w;

	return rect;
}

void shoot_rocket(Ship* ship) {
	Direction direction;
	int delta;
    if (!ship) return;
	if (ship->side == Side_Enemy) {
		direction = Direction_Down;
		delta = 1;
	}

	if (ship->side == Side_Player) {
		direction = Direction_Up;
		delta = -1;
	}

	make_rocket(ship->pos.x, ship->pos.y + delta, direction, ship->side);
}

void draw_ship(Ship* ship) {
	if (ship->destroyed) {
		return;
	}

	if(SDL_RenderCopy(main_renderer, ship->texture, NULL, ship_get_rect(ship)) < 0) check_error("cannot copy ship texture ");
}

void draw_rocket(Rocket* rocket) {
	if (rocket->destroyed) {
		return;
	}

	SDL_RenderCopy(main_renderer, rocket->texture, NULL, get_rocket_rect(rocket));
}

GameResult result = GameResult_OnGoing;

bool ships_have_colision(Ship* ship1, Ship* ship2) {

	int start_of_hitbox_y = ship1->pos.y;
	int end_of_hitbox_y = start_of_hitbox_y + ship1->h;

	int start_of_hitbox_x = ship1->pos.x;
	int end_of_hitbox_x = start_of_hitbox_x + ship1->w;

	if (start_of_hitbox_x < ship2->pos.x && ship2->pos.x < end_of_hitbox_x && start_of_hitbox_y < ship2->pos.y && ship2->pos.y < end_of_hitbox_y) {
		return true;
	}

	return false;
}
void update_states() {
	// update rocket positions
	for (Rocket* rocket : rockets) {
		move_rocket(rocket);
	}
	// @TODO: find colisions of rockets

	// find colisions of player and enemies

	auto player = ships[0];
	for (int i = 1; i < ships.size(); i++) {
		if (ships_have_colision(player, ships[i])) {
			player->destroyed = true;
			ships[i]->destroyed = true;
			printf("player and %d have colision\n", i);
		}
		
	}
	// find colisions of ships and rockets
	for (Ship* ship : ships) {
		for (Rocket* rocket : rockets) {

			int start_of_hitbox_y = ship->pos.y;
			int end_of_hitbox_y = start_of_hitbox_y + ship->h;

			int start_of_hitbox_x = ship->pos.x;
			int end_of_hitbox_x = start_of_hitbox_x + ship->w;

			if (start_of_hitbox_x < rocket->pos.x && rocket->pos.x < end_of_hitbox_x && start_of_hitbox_y < rocket->pos.y && rocket->pos.y < end_of_hitbox_y) {
				ship->destroyed = true;
				rocket->destroyed = true;
			}
		}
	}
	// remove destroyed entities
	std::vector<Ship*> new_ships;
	for (Ship* ship : ships) {
		if (!ship->destroyed) new_ships.push_back(ship);
		else delete ship;
	}
	std::vector<Rocket*> new_rockets;
	for (Rocket* rocket : rockets) {
		if (!rocket->destroyed) new_rockets.push_back(rocket);
		else delete rocket;

	}

	ships = new_ships;
	rockets = new_rockets;

	// move ships
	for (int i = 1; i < ships.size(); i++) {
		while (true) {
			Direction direction = get_random_direction();
			if (enemy_can_move_to_this(ships[i], direction)) {
				move_ship(ships[i], direction);
				break;
			}
		}
	}
	// check for win or lose

	if (ships.size() > 0 && ships[0]->side != Side_Player) {
		result = GameResult_Loss;
	}
	if (ships.size() == 1 && ships[0]->side == Side_Player) result = GameResult_Win;
}

void print_ships() {
	for (int i = 0; i < ships.size(); i++) {
		std::cout << "id: " << i << " side: " << ships[i]->side << "\n";
	}
}

void render() {
    int frameStart = SDL_GetTicks();
    if (SDL_RenderClear(main_renderer) < 0) check_error("cannot clear render");
	update_states();

    if (result != GameResult_OnGoing) {
		std::cout << "ending game ... " << result << "\n";
		quit = true;
		if (result == GameResult_Win) {
			SDL_RenderCopy(main_renderer, win_msg_texture, NULL, &win_msg_rect);
		}
		else {
			SDL_RenderCopy(main_renderer, loss_msg_texture, NULL, &lost_msg_rect);
		}
        SDL_RenderPresent(main_renderer);
        return;
	}
	for (Rocket* rocket : rockets) {
		draw_rocket(rocket);
	}
	for (Ship* ship : ships) {
		draw_ship(ship);
	}


    int frame_end = SDL_GetTicks();
    float elapsedMS = (frame_end - frameStart) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
    SDL_Delay(floor(16.666f - elapsedMS)); // 60 FPS
	SDL_RenderPresent(main_renderer);
}

void game_loop(SDL_Window* window) {
	while (!quit) {
		render();

		SDL_Event event;
		if (SDL_PollEvent(&event) > 0) {
			switch (event.type) {
			case SDL_QUIT: {
				quit = true;
				break;
			}
			case SDL_KEYDOWN: {
				switch (event.key.keysym.sym) {
				case SDLK_SPACE: {
					shoot_rocket(ships[0]);
					break;
				}
				case SDLK_UP: {
					if (player_can_move_to_this(ships[0], Direction_Up)) {
						move_ship(ships[0], Direction_Up);
					}
					break;
				}
				case SDLK_DOWN: {
					if (player_can_move_to_this(ships[0], Direction_Down)) {
						move_ship(ships[0], Direction_Down);
					}
					break;
				}
				case SDLK_LEFT: {
					if (player_can_move_to_this(ships[0], Direction_Left)) {
						move_ship(ships[0], Direction_Left);
					}
					break;
				}
				case SDLK_RIGHT: {
					if (player_can_move_to_this(ships[0], Direction_Right)) {
						move_ship(ships[0], Direction_Right);
					}
					break;
				}

				}
				break;
			}
			};

		}
	}

	if (quit) {
		SDL_Delay(1000);
	}
}

void add_enemies() {
	SDL_Texture* enemy1_ship_texture = load_png_as_texture(main_renderer, ENEMY1_SHIP_ASSET_PATH);
	SDL_Texture* enemy2_ship_texture = load_png_as_texture(main_renderer, ENEMY2_SHIP_ASSET_PATH);
	SDL_Texture* enemy3_ship_texture = load_png_as_texture(main_renderer, ENEMY3_SHIP_ASSET_PATH);
	for (int i = 0; i < ENEMY_GRID_ROWS; i++) {
		for (int j = 0; j < ENEMY_GRID_COLS; j++) {
			SDL_Texture* texture;

			int id = rand() % 3;
			if (id == 0) {
				texture = enemy1_ship_texture;
			}
			else if (id == 1) {
				texture = enemy2_ship_texture;
			}
			else if (id == 2) {
				texture = enemy3_ship_texture;
			}
			else {
				printf("invalid id for enemy texture\n");
				exit(1);
			}
			int texture_width;
			int texture_height;
			SDL_QueryTexture(texture, nullptr, nullptr, &texture_width, &texture_height);

			int enemy_x_padding = 100;
			int enemy_y_padding = 100;
			int x_offset_from_edge = (WINDOW_WIDTH - ((enemy_x_padding * (ENEMY_GRID_COLS - 1)) + (ENEMY_GRID_COLS * texture_width))) / 2;

			make_ship(x_offset_from_edge + (j * enemy_x_padding) + (j - 1 * texture_width),
				(WINDOW_HEIGHT / 8) + i * enemy_y_padding,
				texture_width, texture_height,
				texture,
				Side_Player);
		}
	}
}


int main(int argc, char* argv[], char* environment[]) {
	SDL_Window* main_window = init_main_window();

	main_renderer = SDL_CreateRenderer(main_window, -1, 0);
	if (main_renderer == NULL) check_error("creating renderer");

	
	SDL_Texture* player_ship_texture = load_png_as_texture(main_renderer, PLAYER_SHIP_ASSET_PATH);
	
	rocket_texture = load_png_as_texture(main_renderer, ROCKET_ASSET_PATH);

	if (!player_ship_texture ||
		!rocket_texture) check_error("texture creation failed: ");
	
	int player_ship_h, player_ship_w;
	
	if (SDL_QueryTexture(player_ship_texture, NULL, NULL, &player_ship_w, &player_ship_h) < 0) check_error("cannot query player ship");
	make_texts();

	make_ship((WINDOW_WIDTH - player_ship_h) / 2,
		(WINDOW_HEIGHT - (WINDOW_HEIGHT - player_ship_h) / 6),
		player_ship_w, player_ship_h,
		player_ship_texture,
		Side_Player);

	add_enemies();
	std::cout << "Window width: " << WINDOW_WIDTH << " Window Height: " << WINDOW_HEIGHT << "\n";
	std::cout << "We have #" << ships.size() << " in game.\n";
	game_loop(main_window);
	/*update_states();
	for (Ship* ship : ships) {
		std::cout << ship->side << "-> side \n";
	}*/
	return 0;

}
