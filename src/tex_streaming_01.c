#include <SDL3/SDL.h>
#include <stdio.h>

/*
 * I could use a LL instead of array of pointers for objects?
 * So i would be able to delete objects easily without fragmentation
*/

#define TEXTURE_SIZE 150
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define OBJECTS_MAX 1000

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
/* SDL_Surface *surface = NULL; */

const bool* keyStates = NULL;
int* numkeys = NULL;
int keep_running = true;
bool lock = 0;

enum mode_list {
	NORMAL,
	POINT,
	LINE,
};
uint8_t mode = NORMAL;

typedef struct {
	int mouse_x;
	int mouse_y;
	bool mouse_left_down;
	bool mouse_right_down;
} Appstate;

typedef struct {
	double x;
	double y;
	uint32_t color;
} Point_2d;

typedef struct {
	Point_2d* p2d;
	uint32_t p2d_index;
} Objects;

int sdl_init();
int appstate_init(Appstate* appstate);
int objects_init(Objects* objects);
int objects_destroy(Objects* objects);
void process_event(Appstate* appstate, Objects* objects);

int objects_create(Appstate* appstate, Objects* objects);

int main()
{
	sdl_init();
	Appstate appstate;
	appstate_init(&appstate);
	Objects objects;
	objects_init(&objects);

	while (keep_running) {
		keyStates = SDL_GetKeyboardState(numkeys);
		process_event(&appstate, &objects);
		objects_create(&appstate, &objects);
		SDL_DelayNS(1000);
	}
	SDL_Quit();
	return 0;
}

int sdl_init()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/renderer/streaming-textures", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, TEXTURE_SIZE, TEXTURE_SIZE);
    if (!texture) {
        SDL_Log("Couldn't create streaming texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
	return 1;
}

int appstate_init(Appstate* appstate)
{
	appstate->mouse_x = 0;
	appstate->mouse_y = 0;
	appstate->mouse_left_down = 0;
	appstate->mouse_right_down = 0;
	return 1;
}

int objects_init(Objects* objects)
{
	objects->p2d = SDL_malloc(sizeof(Point_2d) * OBJECTS_MAX);
	for (int i = 0; i < OBJECTS_MAX; i++) {
		objects->p2d[i].x = 0.0;
		objects->p2d[i].y = 0.0;
	}
	objects->p2d_index = 0;
	return 1;
}

void process_event(Appstate* appstate, Objects* objects)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
       switch (event.type) {
          case SDL_EVENT_QUIT:
               keep_running = false;
               break;

          case SDL_EVENT_KEY_DOWN:
               switch(event.key.key) {
                  case SDLK_ESCAPE: 
					  if (!event.key.repeat) {
						  mode = NORMAL;
					  }
					  break;
				  case SDLK_P:
					  if (!event.key.repeat) {
						  mode = POINT;
					  }
					  break;
                  case SDLK_L:
					  if (!event.key.repeat) {
						  mode = LINE;
					  }
					  break;
                  case SDLK_D:
					  if (!event.key.repeat) {
						  printf("mode: %d\n", mode);
						  printf("mouse x,y: %d,%d\n", 
								  appstate->mouse_x, appstate->mouse_y);
						  printf("mouse down: %d\n", 
								  appstate->mouse_left_down);
						  printf("p1 x,y: %f,%f\n", objects->p2d[0].x, objects->p2d[0].y);
						  printf("p2 x,y: %f,%f\n", objects->p2d[1].x, objects->p2d[1].y);
					  }
					  break;
               }
               break;
		  case SDL_EVENT_MOUSE_MOTION:
			   appstate->mouse_x = event.motion.x;
			   appstate->mouse_y = event.motion.y;
			   break;
		  case SDL_EVENT_MOUSE_BUTTON_DOWN:
			   appstate->mouse_left_down = event.button.down;
			   break;
		  case SDL_EVENT_MOUSE_BUTTON_UP:
			   appstate->mouse_left_down = event.button.down;
			   lock = 0;
			   break;
      }
	}
}

int objects_create(Appstate* appstate, Objects* objects)
{
	switch(mode) {
		case NORMAL:
			return 1;
			break;
		case POINT:
			if (appstate->mouse_left_down && lock == 0) {
				lock = 1;
				objects->p2d[objects->p2d_index].x = appstate->mouse_x;
				objects->p2d[objects->p2d_index].y = appstate->mouse_y;
				objects->p2d_index++;
			} else if (!appstate->mouse_left_down) {
				lock = 0;
			}
			break;
		case LINE:
			break;
	}
	return 1;
}

int objects_destroy(Objects* objects)
{
	SDL_free(objects->p2d);
	return 1;
}
