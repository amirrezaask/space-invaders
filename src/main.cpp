#include "SDL2/SDL.h"
#include "SDL2/SDL_Image.h"
#include "SDL2/SDL_events.h"
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
#define u8 Uint8
#define PLAYER_SHIP_ASSET_PATH "./assets/player.png"
#define ENEMY1_SHIP_ASSET_PATH "./assets/enemy1.png"
#define ENEMY2_SHIP_ASSET_PATH "./assets/enemy2.png"
#define ENEMY3_SHIP_ASSET_PATH "./assets/enemy3.png"
#define ROCKET_ASSET_PATH "./assets/rocket.png"
#define ENEMY_GRID_COLS 5
#define ENEMY_GRID_ROWS 5

struct Ship;
struct Rocket;

SDL_Rect* ship_get_rect(Ship* ship);
SDL_Surface* rocket_surface;
SDL_Texture* rocket_texture;
std::vector<Ship*> ships;
SDL_Renderer* main_renderer;

int random_number() {
    return rand() % 50 + 1;
}

void check_error() {
    printf("%s", SDL_GetError());
    exit(1);
}

SDL_Surface* optimize_surface(SDL_Surface* surface, SDL_PixelFormat* format) {
    SDL_Surface* optimized = SDL_ConvertSurface(surface, format, 0);
    SDL_FreeSurface(surface);
    return optimized;
}

SDL_Surface* load_png(std::string path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    return surface;

}

void set_background_color_for_surface(SDL_Surface* surface, u8 r, u8 g, u8 b) {
    if(SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, r, g, b)) < 0 ) check_error();
}



SDL_Window* init_main_window(){

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        check_error();
    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags)) {
        printf("%s", IMG_GetError());
        exit(1);
    }

    SDL_Window* window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    return window;
}

// ------------------------------- Game stuff -----------------------
const std::string YOU_WON_TEXT = "You won, nice job";
const std::string YOU_LOST_TEXT = "You lost, get gud";

enum Side {
    Side_Player,
    Side_Enemy,
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
    SDL_Surface* surface;
    bool destroyed;
    Direction direction;
    Side side;
};

Rocket* make_rocket(int x, int y, SDL_Surface* surface, Direction direction, Side side) {
    Rocket *rocket = new Rocket;
    Vector2 pos;
    pos.x = x;
    pos.y = y;
    rocket->pos = pos;
    rocket->side = side;
    rocket->surface = surface;
    rocket->direction = direction;
    return rocket;

}

Ship* make_ship(int x, int y, int w, int h, SDL_Texture* texture, Side side) {
    Ship *ship = new Ship;
    Vector2 pos;
    pos.x = x;
    pos.y = y;
    ship->w = w;
    ship->h = h;
    ship->pos = pos;
    ship->side = side;
    ship->texture = texture;
    return ship;
}
bool can_move_to_this(Ship* ship, Direction direction) {
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
        if(new_x < 0) return false;
        return true;
    case Direction_UpLeft:
        return can_move_to_this(ship, Direction_Up)
            &&
            can_move_to_this(ship, Direction_Left);

    case Direction_UpRight:
        return can_move_to_this(ship, Direction_Up)
            &&
            can_move_to_this(ship, Direction_Right);
        
    case Direction_DownLeft:
        return can_move_to_this(ship, Direction_Down)
            &&
            can_move_to_this(ship, Direction_Left);
        
    case Direction_DownRight:
        return can_move_to_this(ship, Direction_Down)
            &&
            can_move_to_this(ship, Direction_Right);
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
        rocket->pos.y -= 1;
        break;
    case Direction_Down:
        rocket->pos.y += 1;
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

void shoot_rocket(Ship* ship) {
    Direction direction;
    int delta;
    if (ship->side == Side_Enemy) {
        direction = Direction_Down;
        delta = 1;
    }
    
    
    if (ship->side == Side_Player){
        direction = Direction_Up;
        delta = -1;
    }

    make_rocket(ship->pos.x, ship->pos.y + delta, rocket_surface, direction, ship->side);
}

void draw_ship(Ship* ship) {
    //TODO @CheckError
    SDL_RenderCopy(main_renderer, ship->texture, NULL, ship_get_rect(ship));
}



void update_states() {
    // update rocket positions
    // find colisions of rockets
    // find colisions of ships
    // move ships
    for (int i = 1; i < ships.size(); i ++ ) {
        while (true) {
            Direction direction = get_random_direction();
            if (can_move_to_this(ships[i], direction)) {
                move_ship(ships[i], direction);
                break;
            }
        }
    }
    // check for win or lose
}

void render(SDL_Window* window) {
    SDL_RenderClear(main_renderer);
    update_states();
    for (Ship* ship: ships) {
        draw_ship(ship);
    }

    SDL_RenderPresent(main_renderer);
}

void game_loop(SDL_Window* window) {

    while(true) {
        SDL_Event event;
        if (SDL_PollEvent(&event) > 0) {
            switch(event.type) {
            case SDL_QUIT: {
                return;
            }
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_UP: {
                    if(can_move_to_this(ships[0], Direction_Up)) {
                        move_ship(ships[0], Direction_Up);
                    }
                    break;
                }
                case SDLK_DOWN: {
                    if(can_move_to_this(ships[0], Direction_Down)) {
                        move_ship(ships[0], Direction_Down);
                    }
                    break;
                }
                case SDLK_LEFT: {
                    if(can_move_to_this(ships[0], Direction_Left)) {
                        move_ship(ships[0], Direction_Left);
                    }
                    break;
                }
                case SDLK_RIGHT: {
                    if(can_move_to_this(ships[0], Direction_Right)) {
                        move_ship(ships[0], Direction_Right);
                    }
                    break;
                }

                }
                render(window);
            }

            default: {
                render(window);
            }
                
            };
      
        } else {
            render(window);
        }
    }
}
std::vector<SDL_Surface*> enemy_surfaces;
std::vector<SDL_Texture*> enemy_textures;

void add_enemies() {
    for (int i = 0;i<ENEMY_GRID_ROWS;i++){
        for (int j=0;j<ENEMY_GRID_COLS;j++) {
            Ship* ship = new Ship;
            int id = rand() % 3;
            printf("using rendering enemy: %d\n", id+1);

            SDL_Surface* surface = enemy_surfaces[id];
            SDL_Texture* texture = enemy_textures[id];
    
            int enemy_x_padding = 100;
            int enemy_y_padding = 100;
            int x_offset_from_edge = (WINDOW_WIDTH - ((enemy_x_padding * (ENEMY_GRID_COLS - 1)) + (ENEMY_GRID_COLS * surface->w))) / 2;

            ship->pos.x = x_offset_from_edge + (j * enemy_x_padding) + (j-1 * surface->w);
            ship->pos.y = (WINDOW_HEIGHT / 8) + i * enemy_y_padding;
            ship->h = surface->h;
            ship->w = surface->w;
            ship->texture = texture;
            ship->side = Side_Enemy;
            ship->destroyed = false;

            ships.push_back(ship);
        }
    }
}


int main() {
    SDL_Window *main_window = init_main_window();
    
    main_renderer = SDL_CreateRenderer(main_window, -1, 0);
    if (main_renderer == NULL) printf("cannot create main renderer: %s", SDL_GetError());


    SDL_Surface* player_ship_surface = load_png(PLAYER_SHIP_ASSET_PATH);
    SDL_Surface* enemy1_ship_surface = load_png(ENEMY1_SHIP_ASSET_PATH);
    SDL_Surface* enemy2_ship_surface = load_png(ENEMY2_SHIP_ASSET_PATH);
    SDL_Surface* enemy3_ship_surface = load_png(ENEMY3_SHIP_ASSET_PATH);

    SDL_Texture* player_ship_texture = SDL_CreateTextureFromSurface(main_renderer, player_ship_surface);
    SDL_Texture* enemy1_ship_texture = SDL_CreateTextureFromSurface(main_renderer, enemy1_ship_surface);
    SDL_Texture* enemy2_ship_texture = SDL_CreateTextureFromSurface(main_renderer, enemy2_ship_surface);
    SDL_Texture* enemy3_ship_texture = SDL_CreateTextureFromSurface(main_renderer, enemy3_ship_surface);

    if (player_ship_texture == NULL) printf("cannot create player ship texture\n: %s", SDL_GetError());

    rocket_surface = load_png(ROCKET_ASSET_PATH);

    Ship* player_ship = make_ship((WINDOW_WIDTH - player_ship_surface->w) / 2,
                                  (WINDOW_HEIGHT - (WINDOW_HEIGHT - player_ship_surface->h) / 6),
                                  player_ship_surface->w, player_ship_surface->h,
                                  player_ship_texture,
                                  Side_Player);
    
    ships.push_back(player_ship);
    enemy_surfaces.push_back(enemy1_ship_surface);
    enemy_surfaces.push_back(enemy2_ship_surface);
    enemy_surfaces.push_back(enemy3_ship_surface);
    
    enemy_textures.push_back(enemy1_ship_texture);
    enemy_textures.push_back(enemy2_ship_texture);
    enemy_textures.push_back(enemy3_ship_texture);
        
    add_enemies();
    printf("Window Width: %d Window height: %d\n", WINDOW_WIDTH, WINDOW_HEIGHT);
    printf("number of enemeis are: %lu\n", ships.size());

  
    game_loop(main_window);
   
}
