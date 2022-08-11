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
#define ENEMY_GRID_COLS 1
#define ENEMY_GRID_ROWS 1
#define ENEMY_CHANCE_TO_MOVE 100
#define FONT_PATH "./assets/Go-Bold.ttf"
#define FONT_SIZE 120
#define MSG_WIN "You won"
#define MSG_LOST "You lost"

struct Ship;
struct Rocket;

SDL_Rect* ship_get_rect(Ship* ship);
SDL_Surface* rocket_surface;
SDL_Texture* rocket_texture;
SDL_Rect center_result_text_rect;
std::vector<Ship*> ships;
SDL_Renderer* main_renderer;
std::vector<Rocket*> rockets;
SDL_Texture* win_msg_texture;
SDL_Texture* loss_msg_texture;
SDL_Rect win_msg_rect;
SDL_Rect lost_msg_rect;




int random_number() {
    return rand() % (ENEMY_CHANCE_TO_MOVE + 9 + 1);
}

void check_error() {
    std::cout << "Checking Error: " << SDL_GetError() << "\n";
    exit(1);
}


void make_texts() {
    TTF_Init();
    TTF_Font *font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
    if (font == NULL) check_error();

    SDL_Color text_color = { 0xFF, 0xFF, 0xFF, 0xFF};
    
    SDL_Surface* win_surface = TTF_RenderText_Solid(font, MSG_WIN, text_color);
    if (win_surface == NULL) check_error();
    SDL_Surface* lost_surface = TTF_RenderText_Solid(font, MSG_LOST, text_color);
    if (lost_surface == NULL) check_error();
    
    SDL_Texture* win_texture = SDL_CreateTextureFromSurface(main_renderer, win_surface);
    if (win_texture == NULL) check_error();
    SDL_Texture* lost_texture = SDL_CreateTextureFromSurface(main_renderer, lost_surface);
    if (lost_surface == NULL) check_error();

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

SDL_Surface* load_png(std::string path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    return surface;

}

void set_background_color_for_surface(SDL_Surface* surface, Uint8 r, Uint8 g, Uint8 b) {
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
    Rocket *rocket = new Rocket;
    Vector2 pos;
    pos.x = x;
    pos.y = y;
    rocket->pos = pos;
    rocket->side = side;
    rocket->texture = rocket_texture;
    rocket->direction = direction;
    rockets.push_back(rocket);
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
        if(new_x < 0) return false;
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
        if (new_y > (WINDOW_HEIGHT - (WINDOW_HEIGHT/3))) return false;
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
    SDL_Rect* rect = new SDL_Rect;
    rect->x = rocket->pos.x;
    rect->y = rocket->pos.y;
    rect->h = rocket_surface->h;
    rect->w = rocket_surface->w;

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

    make_rocket(ship->pos.x, ship->pos.y + delta, direction, ship->side);
}

void draw_ship(Ship* ship) {
    //TODO @CheckError
    if (ship->destroyed) {
        return;
    }
    
    SDL_RenderCopy(main_renderer, ship->texture, NULL, ship_get_rect(ship));
}

void draw_rocket(Rocket* rocket) {
    if (rocket->destroyed) {
        return;
    }
    
    SDL_RenderCopy(main_renderer, rocket->texture, NULL, get_rocket_rect(rocket));
}

GameResult result = GameResult_OnGoing;


void update_states() {
    // update rocket positions
    for (Rocket* rocket: rockets) {
        move_rocket(rocket);
    }
    // @TODO: find colisions of rockets
    
    // find colisions of player and enemies
    for (int i = 1; i<ships.size(); i++) {
        auto player = ships[0];
        auto enemy = ships[i];

        int start_of_hitbox_y = player->pos.y; 
        int end_of_hitbox_y = start_of_hitbox_y + player->h;
        
        int start_of_hitbox_x = player->pos.x; 
        int end_of_hitbox_x = start_of_hitbox_x + player->w;

        if (start_of_hitbox_x < enemy->pos.x && enemy->pos.x < end_of_hitbox_x && start_of_hitbox_y < enemy->pos.y && enemy->pos.y < end_of_hitbox_y) {
            player->destroyed = true;
            enemy->destroyed = true;
        }
      
    }
    // find colisions of ships and rockets
    for (Ship* ship: ships) {
        for (Rocket* rocket: rockets) {
            
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
    for (Ship* ship: ships) {
        if (!ship->destroyed) new_ships.push_back(ship);
    }
    std::vector<Rocket*> new_rockets;
    for (Rocket* rocket: rockets) {
        if (!rocket->destroyed) new_rockets.push_back(rocket);
    }

    ships = new_ships;
    rockets = new_rockets;
    
    // move ships
    for (int i = 1; i < ships.size(); i ++ ) {
        while (true) {
            Direction direction = get_random_direction();
            if (enemy_can_move_to_this(ships[i], direction)) {
                move_ship(ships[i], direction);
                break;
            }
        }
    }
    // check for win or lose

    if (ships[0]->side != Side_Player) result = GameResult_Loss;
    if (ships.size() == 1 && ships[0]->side == Side_Player) result = GameResult_Win;
}

void render() {
    SDL_RenderClear(main_renderer);
    update_states();

    if (result != GameResult_OnGoing) {
        if (result == GameResult_Win) {
            SDL_RenderCopy(main_renderer, win_msg_texture, NULL, &win_msg_rect);
            SDL_RenderPresent(main_renderer);
            return;
        } else {
            SDL_RenderCopy(main_renderer, loss_msg_texture, NULL, &lost_msg_rect);
            SDL_RenderPresent(main_renderer);
            return;
        }
    }

    for (Rocket* rocket: rockets) {
        draw_rocket(rocket);
    }
    
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
                case SDLK_SPACE: {
                    shoot_rocket(ships[0]);
                    break;
                }
                case SDLK_UP: {
                    if(player_can_move_to_this(ships[0], Direction_Up)) {
                        move_ship(ships[0], Direction_Up);
                    }
                    break;
                }
                case SDLK_DOWN: {
                    if(player_can_move_to_this(ships[0], Direction_Down)) {
                        move_ship(ships[0], Direction_Down);
                    }
                    break;
                }
                case SDLK_LEFT: {
                    if(player_can_move_to_this(ships[0], Direction_Left)) {
                        move_ship(ships[0], Direction_Left);
                    }
                    break;
                }
                case SDLK_RIGHT: {
                    if(player_can_move_to_this(ships[0], Direction_Right)) {
                        move_ship(ships[0], Direction_Right);
                    }
                    break;
                }

                }
                render();
            }

            default: {
                render();
            }
                
            };
      
        } else {
            render();
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


int main(int argc, char* argv[], char* environment[]) {
    SDL_Window *main_window = init_main_window();
    
    main_renderer = SDL_CreateRenderer(main_window, -1, 0);
    if (main_renderer == NULL) check_error();

    SDL_Surface* player_ship_surface = load_png(PLAYER_SHIP_ASSET_PATH);
    SDL_Surface* enemy1_ship_surface = load_png(ENEMY1_SHIP_ASSET_PATH);
    SDL_Surface* enemy2_ship_surface = load_png(ENEMY2_SHIP_ASSET_PATH);
    SDL_Surface* enemy3_ship_surface = load_png(ENEMY3_SHIP_ASSET_PATH);
    rocket_surface = load_png(ROCKET_ASSET_PATH);
    if (player_ship_surface == NULL ||
        enemy1_ship_surface == NULL ||
        enemy2_ship_surface == NULL ||
        enemy3_ship_surface == NULL ||
        rocket_surface == NULL) check_error();

    SDL_Texture* player_ship_texture = SDL_CreateTextureFromSurface(main_renderer, player_ship_surface);
    SDL_Texture* enemy1_ship_texture = SDL_CreateTextureFromSurface(main_renderer, enemy1_ship_surface);
    SDL_Texture* enemy2_ship_texture = SDL_CreateTextureFromSurface(main_renderer, enemy2_ship_surface);
    SDL_Texture* enemy3_ship_texture = SDL_CreateTextureFromSurface(main_renderer, enemy3_ship_surface);
    rocket_texture = SDL_CreateTextureFromSurface(main_renderer, rocket_surface);
    if (player_ship_texture == NULL ||
        enemy1_ship_texture == NULL ||
        enemy2_ship_texture == NULL ||
        enemy3_ship_texture == NULL ||
        rocket_texture == NULL) check_error();


    make_texts();
    printf("center text: x %d\n", win_msg_rect.x);
    printf("center text: y %d\n", win_msg_rect.y);
    
    if (player_ship_texture == NULL) check_error();

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
    printf("number of enemeis are: %ld\n", ships.size());

  
    game_loop(main_window);

    return 0;
   
}
