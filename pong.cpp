#include "physics.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

// Allocate surfaces
SDL_Surface *screen = NULL;
SDL_Surface *numbers = NULL;
SDL_Surface *p1win = NULL;
SDL_Surface *p2win = NULL;

// SDL Rect for positions of numbers in numbers.png
SDL_Rect num[11];

// SDL event for handling input
SDL_Event input;

// SDL Rect for positioning sprites
SDL_Rect offset;

// Variables for player scores
int scoreplayer1;
int scoreplayer2;

struct game_obj
{
    SDL_Surface *sprite;
    phys_obj *phys;
};

void collision_callback(phys_obj *obj, phys_obj *obj2, int collide_axis, int area_x, int area_y)
{
    // Check if collision is with edge
    if (obj2 == NULL) {
        if (obj->pos_x <= 0) {
            scoreplayer2++;
            obj->pos_x = 300;
            obj->pos_y = 100;
            if (obj->step_y > 0) { obj->step_y = 1; } else { obj->step_y = -1; }
        } else if (obj->pos_x+obj->size_x >= area_x) {
            scoreplayer1++;
            obj->pos_x = 300;
            obj->pos_y = 100;
            if (obj->step_y > 0) { obj->step_y = 1; } else { obj->step_y = -1; }
        }
    } else if (collide_axis == 1) {
        int size = obj2->size_y+obj->size_y;
        int point = obj2->pos_y-obj->size_y;
        int seg = size/4;
        int y1 = point+seg;
        int y2 = y1+seg+seg;
        int y3 = y2+seg;

        if (obj->pos_y >= point) {
            if (obj->pos_y <= y1) {
                obj->step_y = -2;
            } else if (obj->pos_y <= y2) {
                if (obj->step_y > 0) { obj->step_y = 1; } else { obj->step_y = -1; }
            } else if (obj->pos_y <= y3) {
                obj->step_y = 2;
            }
        }
    }
}

void pong()
{
    // Variables

    // Constant variables
    const int resX = 640;
    const int resY = 480;
    const int bpp = 32;

    // Variables for player scores
    scoreplayer1 = 0;
    scoreplayer2 = 0;

    // Positions for numbers.png
    // 0
    num[0].x = 0;
    num[0].y = 0;
    num[0].w = 50;
    num[0].h = 50;

    // 1
    num[1].x = 50;
    num[1].y = 0;
    num[1].w = 50;
    num[1].h = 50;

    // 2
    num[2].x = 100;
    num[2].y = 0;
    num[2].w = 50;
    num[2].h = 50;

    // 3
    num[3].x = 150;
    num[3].y = 0;
    num[3].w = 50;
    num[3].h = 50;

    // 4
    num[4].x = 200;
    num[4].y = 0;
    num[4].w = 50;
    num[4].h = 50;

    // 5
    num[5].x = 250;
    num[5].y = 0;
    num[5].w = 50;
    num[5].h = 50;

    // 6
    num[6].x = 300;
    num[6].y = 0;
    num[6].w = 50;
    num[6].h = 50;

    // 7
    num[7].x = 350;
    num[7].y = 0;
    num[7].w = 50;
    num[7].h = 50;

    // 8
    num[8].x = 400;
    num[8].y = 0;
    num[8].w = 50;
    num[8].h = 50;

    // 9
    num[9].x = 450;
    num[9].y = 0;
    num[9].w = 50;
    num[9].h = 50;

    // 10
    num[10].x = 450;
    num[10].y = 0;
    num[10].w = 50;
    num[10].h = 50;


    // bool vars for control directions and quit event
    bool quit = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(resX, resY, bpp, SDL_HWSURFACE);
    SDL_WM_SetCaption( "SDL PONG", NULL );

    // Set screen clearing colour
    Uint32 clearColor = SDL_MapRGB(screen->format, 0, 0, 0);

    // Physics objects
    game_obj *paddle_left = new game_obj;
    game_obj *paddle_right = new game_obj;
    game_obj *ball = new game_obj;

    // Left paddle
    paddle_left->sprite = SDL_DisplayFormat(IMG_Load("paddle.png"));
    paddle_left->phys = new phys_obj;
    paddle_left->phys->pos_x = 50;
    paddle_left->phys->pos_y = 100;
    paddle_left->phys->size_x = 20;
    paddle_left->phys->size_y = 100;
    paddle_left->phys->step_x = 0;
    paddle_left->phys->step_y = 0;
    paddle_left->phys->delay = 0;
    paddle_left->phys->delay_counter = 0;
    paddle_left->phys->bounce = 0;
    paddle_left->phys->collided = NULL;
    paddle_left->phys->callback = NULL;
    paddle_left->phys->active = true;

    // Right paddle
    paddle_right->sprite = SDL_DisplayFormat(IMG_Load("paddle.png"));
    paddle_right->phys = new phys_obj;
    paddle_right->phys->pos_x = 570;
    paddle_right->phys->pos_y = 350;
    paddle_right->phys->size_x = 20;
    paddle_right->phys->size_y = 100;
    paddle_right->phys->step_x = 0;
    paddle_right->phys->step_y = 0;
    paddle_right->phys->delay = 0;
    paddle_right->phys->delay_counter = 0;
    paddle_right->phys->bounce = 0;
    paddle_right->phys->collided = NULL;
    paddle_right->phys->callback = NULL;
    paddle_right->phys->active = true;

    int paddle_mid = paddle_right->phys->size_y/2;

    // Ball
    ball->sprite = SDL_DisplayFormat(IMG_Load("ball.png"));
    ball->phys = new phys_obj;
    ball->phys->pos_x = 300;
    ball->phys->pos_y = 100;
    ball->phys->size_x = 20;
    ball->phys->size_y = 20;
    ball->phys->step_x = 1;
    ball->phys->step_y = 1;
    ball->phys->delay = 0;
    ball->phys->delay_counter = 0;
    ball->phys->bounce = 1;
    ball->phys->collided = NULL;
    ball->phys->callback = collision_callback;
    ball->phys->active = true;

    // Load images
    numbers = SDL_DisplayFormat(IMG_Load("numbers.png"));
    p1win = SDL_DisplayFormat(IMG_Load("p1win.png"));
    p2win = SDL_DisplayFormat(IMG_Load("p2win.png"));

    // Physics
    phys *physics = new phys(resX, resY);
    physics->add_object(paddle_left->phys);
    physics->add_object(paddle_right->phys);
    physics->add_object(ball->phys);

    // Timer
    //unsigned int timer = SDL_GetTicks();

    // Main loop
    while (quit==false)
    {
        // Check for win
        if (scoreplayer1 == 10) {
            offset.x = 240;
            offset.y = 216;
            SDL_BlitSurface(p1win, NULL, screen, &offset );
            SDL_Flip(screen);
            while (quit==false) {
                SDL_Delay(20);
                while (SDL_PollEvent(&input)) {
                    switch(input.type) {
                        case SDL_KEYDOWN:
                            switch(input.key.keysym.sym) {
                                case SDLK_q:
                                    quit = true;
                                    break;
                            }
                            break;
                    }
                }
            }
            continue;
        }
        if (scoreplayer2 == 10) {
            offset.x = 240;
            offset.y = 216;
            SDL_BlitSurface(p2win, NULL, screen, &offset );
            SDL_Flip(screen);
            while (quit==false) {
                SDL_Delay(20);
                while (SDL_PollEvent(&input)) {
                    switch(input.type) {
                        case SDL_KEYDOWN:
                            switch(input.key.keysym.sym) {
                                case SDLK_q:
                                    quit = true;
                                    break;
                            }
                            break;
                    }
                }
            }
            continue;
        }

        // Read inputs
        while (SDL_PollEvent(&input))
        {
            switch (input.type)
            {
                case SDL_KEYDOWN:
                    switch (input.key.keysym.sym)
                        {
                            case SDLK_LEFT:
                                left = true;
                                break;
                            case SDLK_RIGHT:
                                right = true;
                                break;
                            case SDLK_UP:
                                up = true;
                                break;
                            case SDLK_DOWN:
                                down = true;
                                break;
                            case SDLK_q:
                                quit = true;
                                break;
                        }
                        break;
                case SDL_KEYUP:
                    switch (input.key.keysym.sym)
                        {
                            case SDLK_LEFT:
                                left = false;
                                break;
                            case SDLK_RIGHT:
                                right = false;
                                break;
                            case SDLK_UP:
                                up = false;
                                break;
                            case SDLK_DOWN:
                                down = false;
                                break;
                        }
            }
        }

        // Move left paddle
        paddle_left->phys->step_y = 0;
        if (up) { paddle_left->phys->step_y = -1; }
        if (down) { paddle_left->phys->step_y = 1; }

        // Move right paddle
        if (ball->phys->pos_y > paddle_right->phys->pos_y+paddle_mid) {
            paddle_right->phys->step_y = 1;
        } else {
            paddle_right->phys->step_y = -1;
        }

        // Advance physics
        physics->advance();

        // Redraw screen
        // Clear screen
        SDL_FillRect(screen, NULL, clearColor);

        // First number
        offset.x = 200;
        offset.y = 0;
        SDL_BlitSurface(numbers, &num[scoreplayer1], screen, &offset );

        // Second number
        offset.x = 390;
        offset.y = 0;
        SDL_BlitSurface(numbers, &num[scoreplayer2], screen, &offset );

        // Ball
        offset.x = ball->phys->pos_x;
        offset.y = ball->phys->pos_y;
        SDL_BlitSurface(ball->sprite, NULL, screen, &offset );

        // Left paddle
        offset.x = paddle_left->phys->pos_x;
        offset.y = paddle_left->phys->pos_y;
        SDL_BlitSurface(paddle_left->sprite, NULL, screen, &offset );

        // Right paddle
        offset.x = paddle_right->phys->pos_x;
        offset.y = paddle_right->phys->pos_y;
        SDL_BlitSurface(paddle_right->sprite, NULL, screen, &offset );

        // Flip screen
        SDL_Flip(screen);
        SDL_Delay(2);
    }

    SDL_Quit();

    delete physics;
    delete paddle_left->phys;
    delete paddle_left;
    delete paddle_right->phys;
    delete paddle_right;
    delete ball->phys;
    delete ball;

    return;
}

int main (int argc, char *argv[])
{
    /*
    phys_obj *a = new phys_obj;
    phys_obj *b = new phys_obj;
    phys_obj *c = new phys_obj;

    a->delay = 10;
    b->delay = 20;
    c->delay = 30;

    phys p = phys(640, 480);
    p.add_object(a);
    p.add_object(b);
    p.add_object(c);

    p.advance();
    */

    pong();
    return 0;
}

