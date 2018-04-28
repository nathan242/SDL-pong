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

struct phys_obj
{
    SDL_Surface *sprite;

    int pos_x;
    int pos_y;

    int size_x;
    int size_y;

    int step_x;
    int step_y;

    int delay;
    int delay_counter;

    int bounce;
    phys_obj *collided;

    void (*callback)(phys_obj *obj, phys_obj *obj2, int collide_axis, int area_x, int area_y);
};

class phys
{
    private:
        int area_x;
        int area_y;

        int list_len;

        struct obj_list
        {
            int id;
            phys_obj *obj;
            obj_list *next;
        };
        obj_list *list_head;

        void check_collide(phys_obj *obj, int id);
    public:
        phys(int x, int y);
        int add_object(phys_obj *obj);
        void advance();
        ~phys();
};

phys::phys(int x, int y)
{
    area_x = x;
    area_y = y;

    list_len = 0;

    list_head = NULL;
}

int phys::add_object(phys_obj *obj)
{
    obj_list *list = new obj_list;
    list->id = list_len;
    list->obj = obj;
    list->next = NULL;

    // First
    if (list_head == NULL) {
        list_head = list;
    } else {
        // Append to list
        obj_list *list_item = list_head;
        while (list_item->next != NULL) {
            list_item = list_item->next;
        }
        list_item->next = list;
    }

    return ++list_len;
}

void phys::advance()
{
    obj_list *list = NULL;
    phys_obj *obj = NULL;

    list = list_head;

    while (list != NULL) {
        // Get object
        obj = list->obj;

        // Check if object is colliding with another
        check_collide(obj, list->id);

        // Move object
        if (obj->delay_counter == obj->delay) {
            obj->pos_x += obj->step_x;
            obj->pos_y += obj->step_y;

            obj->delay_counter = 0;
        } else {
            obj->delay_counter++;
        }

        // Get next
        list = list->next;
    }
    
}

void phys::check_collide(phys_obj *obj, int id)
{
    obj_list *list = NULL;
    phys_obj *obj2 = NULL;

    int x1;
    int x2;
    int y1;
    int y2;

    int diff_x;
    int diff_y;

    list = list_head;

    // Check collision with other objects
    while (list != NULL) {
        if (list->id != id) {
            obj2 = list->obj;
            x1 = obj2->pos_x-obj->size_x;
            x2 = obj2->pos_x+obj2->size_x;
            y1 = obj2->pos_y-obj->size_y;
            y2 = obj2->pos_y+obj2->size_y;

            if (obj->pos_x >= x1 && obj->pos_x <= x2 && obj->pos_y >= y1 && obj->pos_y <= y2) {
                if (obj->collided != obj2) {

                    if (obj->pos_x-x1 > x2-obj->pos_x) { diff_x = x2-obj->pos_x; } else { diff_x = obj->pos_x-x1; }
                    if (obj->pos_y-y1 > y2-obj->pos_y) { diff_y = y2-obj->pos_y; } else { diff_y = obj->pos_y-y1; }

                    if (diff_y > diff_x) {
                        if (obj->callback != NULL) { obj->callback(obj, obj2, 1, area_x, area_y); }
                        if (obj->bounce > 0) {
                            obj->step_x = obj->step_x*-1;
                        } else {
                            obj->step_x = 0;
                            obj->step_y = 0;
                        }
                    } else {
                        if (obj->callback != NULL) { obj->callback(obj, obj2, 2, area_x, area_y); }
                        if (obj->bounce > 0) {
                            obj->step_y = obj->step_y*-1;
                        } else {
                            obj->step_x = 0;
                            obj->step_y = 0;
                        }
                    }
                    obj->collided = obj2;
                }
            } else if (obj->collided == obj2) {
                obj->collided = NULL;
            }
        }

        list = list->next;
    }

    // Check collision with edges
    if (obj->pos_x >= area_x-obj->size_x || obj->pos_x <= 0) { 
        if (obj->callback != NULL) { obj->callback(obj, NULL, 0, area_x, area_y); }
        if (obj->bounce > 0) {
            obj->step_x = obj->step_x*-1;
        } else if (obj->pos_x >= area_x-obj->size_x && obj->step_x > 0 || obj->pos_x <= 0 && obj->step_x < 0) {
            obj->step_x = 0;
            obj->step_y = 0;
        }
    }
    if (obj->pos_y >= area_y-obj->size_y || obj->pos_y <= 0) {
        if (obj->callback != NULL) { obj->callback(obj, NULL, 0, area_x, area_y); }
        if (obj->bounce > 0) {
            obj->step_y = obj->step_y*-1;
        } else if (obj->pos_y >= area_y-obj->size_y && obj->step_y > 0 || obj->pos_y <= 0 && obj->step_y < 0) {
            obj->step_x = 0;
            obj->step_y = 0;
        }
    }
}

phys::~phys()
{
    if (list_head != NULL) {
        obj_list *list = NULL;
        obj_list *prev = NULL;
        phys_obj *obj = NULL;

        list = list_head;

        while (list != NULL) {
            obj = list->obj;
            delete obj;
            prev = list;
            list = list->next;
            delete prev;
        }
    }
}

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
    phys_obj *paddle_left = new phys_obj;
    phys_obj *paddle_right = new phys_obj;
    phys_obj *ball = new phys_obj;

    // Left paddle
    paddle_left->sprite = SDL_DisplayFormat(IMG_Load("paddle.png"));
    paddle_left->pos_x = 50;
    paddle_left->pos_y = 100;
    paddle_left->size_x = 20;
    paddle_left->size_y = 100;
    paddle_left->step_x = 0;
    paddle_left->step_y = 0;
    paddle_left->delay = 0;
    paddle_left->delay_counter = 0;
    paddle_left->bounce = 0;
    paddle_left->collided = NULL;
    paddle_left->callback = NULL;

    // Right paddle
    paddle_right->sprite = SDL_DisplayFormat(IMG_Load("paddle.png"));
    paddle_right->pos_x = 570;
    paddle_right->pos_y = 350;
    paddle_right->size_x = 20;
    paddle_right->size_y = 100;
    paddle_right->step_x = 0;
    paddle_right->step_y = 0;
    paddle_right->delay = 0;
    paddle_right->delay_counter = 0;
    paddle_right->bounce = 0;
    paddle_right->collided = NULL;
    paddle_right->callback = NULL;

    int paddle_mid = paddle_right->size_y/2;

    // Ball
    ball->sprite = SDL_DisplayFormat(IMG_Load("ball.png"));
    ball->pos_x = 300;
    ball->pos_y = 100;
    ball->size_x = 20;
    ball->size_y = 20;
    ball->step_x = 1;
    ball->step_y = 1;
    ball->delay = 0;
    ball->delay_counter = 0;
    ball->bounce = 1;
    ball->collided = NULL;
    ball->callback = collision_callback;

    // Load images
    numbers = SDL_DisplayFormat(IMG_Load("numbers.png"));
    p1win = SDL_DisplayFormat(IMG_Load("p1win.png"));
    p2win = SDL_DisplayFormat(IMG_Load("p2win.png"));

    // Physics
    phys *physics = new phys(resX, resY);
    physics->add_object(paddle_left);
    physics->add_object(paddle_right);
    physics->add_object(ball);

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
        paddle_left->step_y = 0;
        if (up) { paddle_left->step_y = -1; }
        if (down) { paddle_left->step_y = 1; }

        // Move right paddle
        if (ball->pos_y > paddle_right->pos_y+paddle_mid) {
            paddle_right->step_y = 1;
        } else {
            paddle_right->step_y = -1;
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
        offset.x = ball->pos_x;
        offset.y = ball->pos_y;
        SDL_BlitSurface(ball->sprite, NULL, screen, &offset );

        // Left paddle
        offset.x = paddle_left->pos_x;
        offset.y = paddle_left->pos_y;
        SDL_BlitSurface(paddle_left->sprite, NULL, screen, &offset );

        // Right paddle
        offset.x = paddle_right->pos_x;
        offset.y = paddle_right->pos_y;
        SDL_BlitSurface(paddle_right->sprite, NULL, screen, &offset );

        // Flip screen
        SDL_Flip(screen);
        SDL_Delay(2);
    }

    delete physics;

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

