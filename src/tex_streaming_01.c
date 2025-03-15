#include <SDL3/SDL.h>
#include <stdio.h>

#define TEXTURE_SIZE 150
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

typedef struct {
	int x;
	int y;
} point_2d;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
/* SDL_Surface *surface = NULL; */

const bool* keyStates = NULL;
int* numkeys = NULL;
int keep_running = true;

int sdl_init();
void process_event();
/* void update(); */
/* void draw(); */
/* void destroy(); */

int main()
{
	sdl_init();
	while (keep_running) {
		keyStates = SDL_GetKeyboardState(numkeys);
		process_event();
		/* update(); */
		/* draw(); */
		SDL_DelayNS(1000);
	}
	SDL_Quit();
	/* destroy(); */
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

void process_event()
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
					  keep_running = false;
					  break;
                  case SDLK_L: 
					  printf("keystate D: %d\n", keyStates[SDL_SCANCODE_D]);
					  break;
                  case SDLK_D: 
					  if (!event.key.repeat) {
						printf("keystate D: %d\n", keyStates[SDL_SCANCODE_D]);
					  }
					  break;
               }
               break;
       }
    }
}

/* void destry() */
/* { */
/* 	SDL_Quit(); */
/* } */
