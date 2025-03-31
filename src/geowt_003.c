#include <SDL3/SDL.h>
#include <stdio.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480 

#define OBJECTS_MAX 1000
#define TEXT_BUF_MAX 1024

#define DEFAULT_FG_COLOR 0x00000000
#define DEFAULT_BG_COLOR 0xFFFFFFFF

/* #define DEBUG */

const bool *g_key_states = NULL;
int *g_num_keys = NULL;
int keep_running = true;
bool lock = 0;
int pix_cnt = 0;
bool line_first_point = 1;
bool first_run = 1;
int scale_var = 3;
bool OBJ_CREATED = false;


typedef enum {
  NORMAL,
  POINT,
  LINE,
	CIRCLE,
} AppMode;

typedef enum {
	SELECTED,
	HIGHLITED,
	STATUS_COUNT,
} ObjectStatus;

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *window_texture;
	double density;
	uint32_t w_pixels;
	uint32_t h_pixels;
	bool keep_running;
	AppMode mode;

  double mouse_x;
  double mouse_y;
  bool mouse_left_down;
  bool mouse_right_down;
	bool mouse_click;

  bool text_input;
  char text[TEXT_BUF_MAX];
} AppState;

typedef struct {
	void *data;
	uint32_t length;
	uint32_t capacity;
	size_t object_size;
} ObjectBuf;

typedef struct {
  double x;
  double y;
} Point2D;

typedef struct {
	double x;
	double y;
} Vector2D;

typedef struct {
  Point2D p0;
  Point2D p1;
  uint32_t color;
	ObjectStatus status_buf[STATUS_COUNT];
} Line2D;

typedef struct {
	Point2D center;
	double radius;
  uint32_t color;
	ObjectBuf is_point_buf;
	ObjectStatus status_buf[STATUS_COUNT];
} Circle2D;

typedef struct {
	ObjectBuf point_2D_buf;
	ObjectBuf line_2D_buf;
	ObjectBuf circle_2D_buf;
	ObjectBuf is_circle_buf;

	ObjectBuf line_2D_seg_buf;
} Objects;

void vector_append(ObjectBuf *ctx);
void vector_pop(ObjectBuf *ctx, uint32_t index);
void vector_destroy(ObjectBuf *ctx);

void calc_intersection(ObjectBuf *buf);

int app_init(AppState *app_state);
int objects_init(Objects *objects);
void process_event(AppState *app_state, Objects *objects);
void draw(AppState *app_state, Objects *objects);
int objects_create(AppState *app_state, Objects *objects);
int graphics(AppState *app_state, Objects *objects);

int create_line( Objects *objects, Point2D *p0, Point2D *p1, uint32_t color);
int create_circle( Objects *objects, Point2D *center, double radius, uint32_t color);

int main() {
  AppState app_state;
  Objects objects;
  app_init(&app_state);
  objects_init(&objects);

  while (keep_running) {
    g_key_states = SDL_GetKeyboardState(g_num_keys);
		app_state.mouse_click = false;
    process_event(&app_state, &objects);
    objects_create(&app_state, &objects);

		// probably should lock and unlock the texture in main
		// to be able to draw from many places
		graphics(&app_state, &objects);
#ifndef DEBUG
    draw(&app_state, &objects);
		/* float x,y; */
		/* SDL_GetGlobalMouseState(&x, &y); */
		/* printf("glob mouse pos: %f, %f\n", x,y); */
#endif
    SDL_DelayNS(1000);
  }
  SDL_Quit();
  return 0;
}

int app_init(AppState *app_state) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

	app_state->window = NULL;
	app_state->renderer = NULL;
  if (!SDL_CreateWindowAndRenderer("examples/renderer/streaming-textures",
				WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_HIGH_PIXEL_DENSITY |
				SDL_WINDOW_MOUSE_CAPTURE, &app_state->window, &app_state->renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
	app_state->w_pixels = WINDOW_WIDTH * SDL_GetWindowPixelDensity(app_state->window);
	app_state->h_pixels = WINDOW_HEIGHT * SDL_GetWindowPixelDensity(app_state->window);

	// texture create with pixels and not window size -> retina display scaling
  app_state->window_texture = SDL_CreateTexture(
			app_state->renderer, SDL_PIXELFORMAT_XRGB8888,
			SDL_TEXTUREACCESS_STREAMING, 
			app_state->w_pixels, app_state->h_pixels);

	if (!app_state->window_texture) {
    SDL_Log("Couldn't create streaming texture: %s", SDL_GetError());
    return SDL_APP_FAILURE;
	}

	app_state->keep_running = true;
  app_state->mouse_x = 0;
  app_state->mouse_y = 0;
  app_state->mouse_left_down = 0;
  app_state->mouse_right_down = 0;
	app_state->mouse_click = false;
  app_state->density = SDL_GetWindowPixelDensity(app_state->window);
  app_state->text_input = false;
  for (int i = 0; i < TEXT_BUF_MAX; i++) {
    app_state->text[i] = 0;
  }
  return 1;
}

// compound literal use (C99)
int objects_init(Objects *objects) {
	objects->point_2D_buf = (ObjectBuf){ .object_size = sizeof(Point2D) };
	objects->line_2D_buf = (ObjectBuf){ .object_size = sizeof(Line2D) };
	objects->circle_2D_buf = (ObjectBuf){ .object_size = sizeof(Circle2D) };

	objects->is_circle_buf = (ObjectBuf){ .object_size = sizeof(Circle2D) };

	objects->line_2D_seg_buf = (ObjectBuf){ .object_size = sizeof(Line2D) };
	/* objects->intersections_2D = (ObjectBuf){ .object_size = sizeof(Point2D) }; */

  return 1;
}

void process_event(AppState *app_state, Objects *objects) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      keep_running = false;
      break;
    case SDL_EVENT_TEXT_INPUT:
      SDL_strlcat(app_state->text, event.text.text, sizeof(app_state->text));
      printf("%s", event.text.text);
      fflush(stdout); // This will flush any pending printf output
      break;
    case SDL_EVENT_KEY_DOWN:
	  if (app_state->text_input) {
        if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_RETURN) {
					/* if (app_state->mode == LINE) { */
					/* 	double* p0x = &objects->l2d->p1.x; */
					/* 	double* p0y = &objects->l2d->p1.y; */
					/* 	double* p1x = &objects->l2d->p2.x; */
					/* 	double* p1y = &objects->l2d->p2.y; */
					/* 	int index = 0; */
					/* 	sscanf(app_state->text, "%d:(%lf,%lf)(%lf,%lf)", &index, */
					/* 			&objects->l2d[index].p1.x, &objects->l2d[index].p1.y, */
					/* 			&objects->l2d[index].p2.x, &objects->l2d[index].p2.y); */
					/* 	char* c = app_state->text; */
					/* 	for (int i = 0; *c != '\0' && i < TEXT_BUF_MAX; i++) { */
					/**/
					/* 	} */
					/* } */
					/*      printf("\ntext input stop\n"); */
					/*      app_state->text_input = false; */
					/*      SDL_StopTextInput(window); */
        }
	  } else {
        switch (event.key.key) {
        case SDLK_T:
					app_state->text_input = true;
					for (int i = 0; i < TEXT_BUF_MAX; i++) {
						app_state->text[i] = 0;
					}
					if (app_state->mode == LINE) {
						printf("line:(p0x,p0y)(p1x,p1y):\n");
					} else {
						printf("text input:\n");
					}
          SDL_StartTextInput(app_state->window);
          break;
        case SDLK_Y:
          printf("text:\n %s\n", app_state->text);
          break;
        case SDLK_ESCAPE:
          if (!event.key.repeat) {
            app_state->mode = NORMAL;
          }
          break;
				case SDLK_C:
          if (!event.key.repeat) {
            app_state->mode = CIRCLE;
          }
					break;
        case SDLK_P:
          if (!event.key.repeat) {
            app_state->mode = POINT;
          }
          break;
        case SDLK_L:
          if (!event.key.repeat) {
            app_state->mode = LINE;
          }
          break;
        case SDLK_D:
          if (!event.key.repeat) {
            printf("app_state->mode: %d\n", app_state->mode);
            printf("mouse x,y: %f,%f\n", app_state->mouse_x, app_state->mouse_y);
            printf("mouse down: %d\n", app_state->mouse_left_down);
						/*       printf("p1 x,y: %f,%f\n", objects->p2d[0].x, objects->p2d[0].y); */
						/*       printf("p2 x,y: %f,%f\n", objects->p2d[1].x, objects->p2d[1].y); */
						/*       printf("l1: (%f,%f/%f,%f)\n", objects->l2d[0].p1.x, */
						/*              objects->l2d[0].p1.y, objects->l2d[0].p2.x, */
						/*              objects->l2d[0].p2.y); */
						/* printf("pix dens: %f\n", SDL_GetWindowPixelDensity(window)); */
						/* int w,h; */
						/* SDL_GetWindowSizeInPixels(window, &w, &h); */
						/* printf("displayscale %f\n", SDL_GetWindowDisplayScale(window)); */
						/* printf("size in pix: %d,%d\n", w, h); */
						/* SDL_GetCurrentRenderOutputSize(renderer, &w, &h); */
						/* printf("output size w,h: %d,%d\n", w,h); */
						/* float x,y; */
						/* SDL_GetGlobalMouseState(&x, &y); */
						/* printf("glob mouse pos: %f, %f\n", x,y); */
          }
          break;
        case SDLK_R:
          draw(app_state, objects);
          break;
				case SDLK_H:
					SDL_SetWindowMouseGrab(app_state->window, 1);
					SDL_HideCursor();
					break;
				case SDLK_J:
					SDL_SetWindowMouseGrab(app_state->window, 0);
					SDL_ShowCursor();
					break;
        case SDLK_S:
          /* SDL_sscanf(stdin, "%lf,%lf %lf,%lf", &objects->l2d->p1.x, */
          /*            &objects->l2d->p1.y, &objects->l2d->p2.x, */
          /*            &objects->l2d->p2.y); */
          /* printf("converted: %lf,%lf %lf,%lf\n", objects->l2d->p1.x, */
          /*        objects->l2d->p1.y, objects->l2d->p2.x, objects->l2d->p2.y); */
          /**/
          break;
        }
      }
      break;
    case SDL_EVENT_MOUSE_MOTION:
      /* app_state->mouse_x = SDL_lround(event.motion.x * app_state->density); */
      /* app_state->mouse_y = SDL_lround(event.motion.y * app_state->density); */
      app_state->mouse_x = SDL_lround(event.motion.x * app_state->density);
      app_state->mouse_y = SDL_lround(event.motion.y * app_state->density);
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      app_state->mouse_left_down = event.button.down;
			app_state->mouse_click = true;
      break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
      app_state->mouse_left_down = event.button.down;
      lock = 0;
      break;
    }
  }
}

int create_line( Objects *objects, Point2D *p0, Point2D *p1, uint32_t color)
{
	vector_append(&objects->line_2D_buf);
	Line2D *line = ((Line2D *)objects->line_2D_buf.data);
	int index = objects->line_2D_buf.length - 1;
	if (!p0) {
		line[index].p0.x = 0.0; 
		line[index].p0.y = 0.0; 
	} else {
		line[index].p0.x = p0->x; 
		line[index].p0.y = p0->y; 
	}
	if (!p1) {
		line[index].p1.x = 0.0; 
		line[index].p1.y = 0.0; 
	} else {
		line[index].p1.x = p1->x; 
		line[index].p1.y = p1->y; 
	}
	if (!color) {
		line[index].color = DEFAULT_FG_COLOR;
	} else {
		line[index].color = color;
	}
	for (int i = 0; i < STATUS_COUNT; i++) {
		line[index].status_buf[i] = false;
	}
	printf("New line %d created!\n", index);
	return 0;
}

int create_line_segment( Objects *objects, Point2D *p0, Point2D *p1, uint32_t color)
{
	vector_append(&objects->line_2D_seg_buf);
	Line2D *line = ((Line2D *)objects->line_2D_seg_buf.data);
	int index = objects->line_2D_seg_buf.length - 1;
	if (!p0) {
		line[index].p0.x = 0.0; 
		line[index].p0.y = 0.0; 
	} else {
		line[index].p0.x = p0->x; 
		line[index].p0.y = p0->y; 
	}
	if (!p1) {
		line[index].p1.x = 0.0; 
		line[index].p1.y = 0.0; 
	} else {
		line[index].p1.x = p1->x; 
		line[index].p1.y = p1->y; 
	}
	if (!color) {
		line[index].color = DEFAULT_FG_COLOR;
	} else {
		line[index].color = color;
	}
	for (int i = 0; i < STATUS_COUNT; i++) {
		line[index].status_buf[i] = false;
	}
	printf("New line %d created!\n", index);
	return 0;
}

int create_circle( Objects *objects, Point2D *center, double radius, uint32_t color)
{
	// if no radius then 0, if no color then default fg (define it)
	vector_append(&objects->circle_2D_buf);
	Circle2D *circle = ((Circle2D *)objects->circle_2D_buf.data);
	int index = objects->circle_2D_buf.length - 1;

	circle[index].center.x = center->x; 
	circle[index].center.y = center->y;

	if (!radius) {
		circle[index].radius = 0.0;
	} else {
		circle[index].radius = radius;
	}
	if (!color) {
		circle[index].color = DEFAULT_FG_COLOR;
	} else {
		circle[index].color = color;
	}
	for (int i = 0; i < STATUS_COUNT; i++) {
		circle[index].status_buf[i] = false;
	}
	circle[index].is_point_buf = (ObjectBuf){ .object_size = sizeof(Point2D) };
	printf("New circle %d created!\n", index);
	return 1;
}

int create_is_circle( Objects *objects, Point2D *center, double radius, uint32_t color)
{
	// if no radius then 0, if no color then default fg (define it)
	vector_append(&objects->is_circle_buf);
	Circle2D *circle = ((Circle2D *)objects->is_circle_buf.data);
	int index = objects->is_circle_buf.length - 1;

	circle[index].center.x = center->x; 
	circle[index].center.y = center->y;

	if (!radius) {
		circle[index].radius = 0.0;
	} else {
		circle[index].radius = radius;
	}
	if (!color) {
		circle[index].color = DEFAULT_FG_COLOR;
	} else {
		circle[index].color = color;
	}
	for (int i = 0; i < STATUS_COUNT; i++) {
		circle[index].status_buf[i] = false;
	}
	circle[index].is_point_buf = (ObjectBuf){ .object_size = sizeof(Point2D) };
	return 1;
}


int objects_create(AppState *app_state, Objects *objects) {
  switch (app_state->mode) {
  case NORMAL:
		// TODO: clear object vars
    return 1;
    break;
  case POINT:
    if (app_state->mouse_left_down && lock == 0) {
      lock = 1;
			vector_append(&objects->point_2D_buf);
			Point2D *point = ((Point2D *)objects->point_2D_buf.data);
			uint32_t index = objects->point_2D_buf.length - 1;
			printf("point %d created\n", index);
			point[index].x = app_state->mouse_x; 
			point[index].y = app_state->mouse_y; 
    } else if (!app_state->mouse_left_down) {
      lock = 0;
    }
    break;
  case LINE:
    if (app_state->mouse_left_down && !lock) { // make point
      if (line_first_point) {
				vector_append(&objects->line_2D_buf);
				Line2D *l = ((Line2D *)objects->line_2D_buf.data);
				int index = objects->line_2D_buf.length - 1;
				l[index].p0.x = app_state->mouse_x; 
				l[index].p0.y = app_state->mouse_y; 
				l[index].color = 0x00000000;
				for (int i = 0; i < STATUS_COUNT; i++) {
					l[index].status_buf[i] = false;
				}
        line_first_point = 0;
        lock = 1;
      } else {
				Line2D *l = ((Line2D *)objects->line_2D_buf.data);
				uint32_t index = objects->line_2D_buf.length - 1;
				printf("index: %d color: %d\n", index, l[index].color);
				l[index].p1.x = app_state->mouse_x; 
				l[index].p1.y = app_state->mouse_y; 
        line_first_point = 1;
        lock = 1;
				printf("line %d created\n", index);
				OBJ_CREATED = true;
				printf("%f,%f,%f,%f\n", l[index].p0.x, l[index].p0.y, l[index].p1.x, l[index].p1.y);
      }
    } else if (!app_state->mouse_left_down && lock) {
      lock = 0;
    }
    break;
	case CIRCLE:
    if (app_state->mouse_left_down && !lock) { // make point
      if (line_first_point) {
				Point2D mouse = {app_state->mouse_x, app_state->mouse_y};
				create_circle(objects, &mouse, 0.0, 0);
        line_first_point = 0;
        lock = 1;
      } else {
				Circle2D *c = ((Circle2D *)objects->circle_2D_buf.data);
				uint32_t index = objects->circle_2D_buf.length - 1;
				int x = app_state->mouse_x;
				int y = app_state->mouse_y;
				c[index].radius = SDL_sqrt(SDL_pow((c[index].center.x - x), 2.0) + 
						SDL_pow((c[index].center.y - y), 2.0)); 
        line_first_point = 1;
        lock = 1;
				printf("circle %d created\n", index);
				OBJ_CREATED = true;
				printf("%f,%f,%f\n", c[index].center.x, c[index].center.y, c[index].radius );
      }
    } else if (!app_state->mouse_left_down && lock) {
      lock = 0;
    }
		break;
  }
  return 1;
}

int compare( const void* a, const void* b)
{
    const Point2D *pA = (const Point2D *)a;
    const Point2D *pB = (const Point2D *)b;
    if (pA->x < pB->x) return -1;
    else if (pA->x > pB->x) return 1;
    else {
        if (pA->y < pB->y) return -1;
        else if (pA->y > pB->y) return 1;
        else return 0;
    }
}



int graphics(AppState *app_state, Objects *objects)
{
	// alles nur für segmente erstmal nicht für base objects
	// change object color (highlight) based on mouse distance
	Line2D *line = (Line2D *)objects->line_2D_buf.data;
	Line2D *seg = (Line2D *)objects->line_2D_seg_buf.data;

	// if objects count changed, run once and update objectscount watcher

	for (int i = 0; i < objects->line_2D_seg_buf.length; i++) {
		/* int x0, y0, x1, y1, ax, ay; */
		Point2D p0 = { seg[i].p0.x, seg[i].p0.y };
		Point2D p1 = { seg[i].p1.x, seg[i].p1.y };
		Vector2D a = { -(p1.y - p0.y), (p1.x - p0.x) }; // senkrecht zu (p1 - p0)
		Point2D mouse = { app_state->mouse_x, app_state->mouse_y };

		// relative to pixel desnsity
		double distance = SDL_abs((a.x * mouse.x + a.y * mouse.y + (-a.x * p0.x - a.y * p0.y)) / SDL_sqrt(SDL_pow(a.x, 2.0) + SDL_pow(a.y, 2.0)));
		/* printf("%f,%f,%f,%f,%f\n", distance, mouse.x, mouse.y, a.x, a.y); */
		SDL_assert(distance <= SDL_max(app_state->w_pixels, app_state->h_pixels));
		if (distance < 20.0 && mouse.x >= p0.x && mouse.x <= p1.x) {
			seg[i].status_buf[HIGHLITED] = true;
		} else {
			seg[i].status_buf[HIGHLITED] = false;
		}
		if (seg[i].status_buf[HIGHLITED] && app_state->mouse_click) {
			if (seg[i].status_buf[SELECTED]) {
				seg[i].status_buf[SELECTED] = false;
			}
			seg[i].status_buf[SELECTED] = true;
		}
		if (seg[i].status_buf[SELECTED]) {
			seg[i].color = 0xFF00FF00;
		} else if (seg[i].status_buf[HIGHLITED]) {
			seg[i].color = 0xFFFF0000;
		} else {
			seg[i].color = DEFAULT_FG_COLOR;
		}
	}


	/* Circle2D *circle = (Circle2D *)objects->line_2D_buf.data; */
	/* for (int i = 0; i < objects->circle_2D_buf.length; i++) { */
	/* 	Point2D m = { circle[i].center.x, circle[i].center.y }; */
	/* 	double radius = circle[i].radius; */
	/* 	Point2D mouse = { app_state->mouse_x, app_state->mouse_y }; */
	/**/
	/* 	distance = SDL_abs((m.x - mouse.x) + (m.y - mouse.y)); */
	/* 	if ( distance < 20.0 ) { */
	/* 		line[i].color = 0xFFFF0000; */
	/* 		line[i].status_buf[HIGHLITED] = true; */
	/* 	} else { */
	/* 		line[i].color = 0x00000000; */
	/* 		line[i].status_buf[HIGHLITED] = false; */
	/* 	} */
	/* } */


	// intersections TODO: not using linear system, refactor later?
	if (OBJ_CREATED == true) {
		objects->line_2D_seg_buf.length = 0;

		Point2D end_points[1000];
		int end_point_counter;

		// for each line, compare with all others for intersection points
		for (int i = 0; i < objects->line_2D_buf.length; i++) {
			end_point_counter = 0;
			// if this line[i] intersects with any other afterwards, gen circle around
			// end_points first two are p0, p1
			Point2D p0 = { line[i].p0.x, line[i].p0.y };
			Point2D p1 = { line[i].p1.x, line[i].p1.y };
			Vector2D a = { -(p1.y - p0.y), (p1.x - p0.x) }; // senkrecht zu (p1 - p0)

			// get outer endpoints
			end_points[end_point_counter++] = p0;
			end_points[end_point_counter++] = p1;
			for ( int j = 0 ; j < objects->line_2D_buf.length; j++) {
				if (j == i) { continue; }
				Point2D p2 = { line[j].p0.x, line[j].p0.y };
				Point2D p3 = { line[j].p1.x, line[j].p1.y };
				Vector2D v = { (p3.x - p2.x), (p3.y - p2.y)};
				double t = 0.0;
				Point2D is_point = { 0.0, 0.0 };
				// check denominator first to not divide trough 0
				t = (-(-a.x * p0.x - a.y * p0.y) - a.x * p2.x - a.y * p2.y) 
					/ ( a.x * v.x + a.y * v.y);
				is_point.x = p2.x + t * v.x;
				is_point.y = p2.y + t * v.y;
				// test if inside the line endpoints
				if (is_point.x >= p2.x && is_point.x <= p3.x
						&& is_point.x >= p0.x && is_point.x <= p1.x) {
					// test for duplicate intersections
					bool duplicate = false;
					// need epsilon here
					for (int k = 0; k < end_point_counter; k++) {
						if (end_points[k].x == is_point.x && end_points[k].y == is_point.y) {
							duplicate = true;
						}
					}
					if (!duplicate) {
						end_points[end_point_counter++] = is_point;
						/* printf("is_point: %f, %f, %f\n", t, is_point.x, is_point.y); */
						/* create_circle(objects, &is_point, 20.0, 0); */
					}
				}
				for (int k = 0; k < end_point_counter; k++) {
						printf("%f,%f\n", end_points[k].x, end_points[k].y);
				}
				printf("---now sort!\n");
				SDL_qsort(&end_points, end_point_counter, sizeof(Point2D), compare );
				for (int k = 0; k < end_point_counter; k++) {
						printf("%f,%f\n", end_points[k].x, end_points[k].y);
				}
				for (int k = 0; k < end_point_counter-1; k++) {
						create_line_segment(objects, &end_points[k], &end_points[k+1], 0);
				}
				for (int k = 0; k < end_point_counter; k++) {
						create_is_circle(objects, &end_points[k], 20.0, 0);
				}
			}
		} 
	}


	// circle intersections over all lines
	Circle2D *circle = (Circle2D *)objects->circle_2D_buf.data;
	if (OBJ_CREATED == true) {

		for (int i = 0; i < objects->circle_2D_buf.length; i++) {
			Point2D m = { circle[i].center.x, circle[i].center.y };
			double radius = circle[i].radius;
			Point2D *is_point_buf = (Point2D *)circle[i].is_point_buf.data;
			// reset intersections for each circle to 0
			circle[i].is_point_buf.length = 0;

			//draw


			// test every line and save intersections if not duplicate
			for (int j = 0; j < objects->line_2D_buf.length; j++) {
				Point2D p0 = { line[j].p0.x, line[j].p0.y };
				Point2D p1 = { line[j].p1.x, line[j].p1.y };

				Point2D v = {p1.x - p0.x, p1.y - p0.y };

				double a = SDL_pow(v.x, 2.0) + SDL_pow(v.y, 2.0);
				double b = 2.0 * v.x * (p0.x - m.x) + 2 * v.y * (p0.y - m.y);
				double c = SDL_pow(p0.x - m.x, 2.0) + SDL_pow(p0.y - m.y, 2.0) - SDL_pow(radius, 2.0);

				double det = SDL_pow(b, 2.0) - 4 * a * c;
				double t0 = 0.0;
				double t1 = 0.0;

				printf("det: %f\n", det);
				

				// check if 0 and 1 are the same with epsilon
				Point2D is_point_0 = { 0.0, 0.0 };
				Point2D is_point_1= { 0.0, 0.0 };
				bool duplicate = false;
				bool has_is = false;
				bool double_is = false;


				if (det > 0.0) {
					t0 = (-b +	SDL_sqrt(det)) / (2 * a);
					t1 = (-b -	SDL_sqrt(det)) / (2 * a);
					is_point_0.x = p0.x + t0 * v.x;
					is_point_0.y = p0.y + t0 * v.y;

					is_point_1.x = p0.x + t1 * v.x;
					is_point_1.y = p0.y + t1 * v.y;

					double_is = true;
					has_is = true;
				} else if (det == 0.0) { // need epsilon
					t0 = -b / (2 * a);

					is_point_0.x = p0.x + t0 * v.x;
					is_point_0.y = p0.y + t0 * v.y;

					double_is = false;
					has_is = true;
				} else {
					has_is = false;
				}

				printf("is_0: %f,%f is_1: %f,%f\n", is_point_0.x, is_point_0.y, is_point_1.x, is_point_1.y);

				// check duplicate intersection need epsilon here
				if (has_is) {
					/* if (double_is) { */
					/* 	for (int k = 0; k < circle[i].is_point_buf.length; k++) { */
					/* 		if (is_point_buf[k].x == is_point_0.x && is_point_buf[k].y == is_point_0.y) { */
					/* 			duplicate = true; */
					/* 		} */
					/* 		if (is_point_buf[k].x == is_point_1.x && is_point_buf[k].y == is_point_1.y) { */
					/* 			duplicate = true; */
					/* 		} */
					/* 	} */
					/* } else { */
					/* 	for (int k = 0; k < circle[i].is_point_buf.length; k++) { */
					/* 		if (is_point_buf[k].x == is_point_0.x && is_point_buf[k].y == is_point_0.y) { */
					/* 			duplicate = true; */
					/* 		} */
					/* 	} */
					/* } */
					if (!duplicate) {
						if (double_is) {
							vector_append(&circle[i].is_point_buf);
							is_point_buf = (Point2D *)circle[i].is_point_buf.data; // because of REALOC!!!!
							is_point_buf[circle[i].is_point_buf.length-1].x = is_point_0.x;
							is_point_buf[circle[i].is_point_buf.length-1].y = is_point_0.y;
							vector_append(&circle[i].is_point_buf);
							is_point_buf = (Point2D *)circle[i].is_point_buf.data; 
							is_point_buf[circle[i].is_point_buf.length-1].x = is_point_1.x;
							is_point_buf[circle[i].is_point_buf.length-1].y = is_point_1.y;
						} else {
							vector_append(&circle[i].is_point_buf);
							is_point_buf = (Point2D *)circle[i].is_point_buf.data; 
							is_point_buf[circle[i].is_point_buf.length-1].x = is_point_0.x;
							is_point_buf[circle[i].is_point_buf.length-1].y = is_point_0.y;
						}
					}
				} else {
					continue;
				}
			}
			for (int k = 0; k < circle[i].is_point_buf.length; k++) {
					is_point_buf = (Point2D *)circle[i].is_point_buf.data; 
					printf("is_point: %f,%f\n", is_point_buf[k].x, is_point_buf[k].y);
					create_is_circle(objects, &is_point_buf[k], 20.0, 0);
			}
		}
	}

	OBJ_CREATED = false; // this should be somwhere global
	return 0;
}


void draw(AppState *app_state, Objects *objects) {
  SDL_FRect dst_rect;
  dst_rect.x = 0.0;
  dst_rect.y = 0.0;
  dst_rect.w = app_state->w_pixels;
  dst_rect.h = app_state->h_pixels;
  SDL_FRect src_rect;
  src_rect.x = 0.0;
  src_rect.y = 0.0;
  src_rect.w = app_state->w_pixels;
  src_rect.h = app_state->h_pixels;

  void *pixels;
  int pitch;
	/* SDL_SetTextureScaleMode(app_state->window_texture, SDL_SCALEMODE_NEAREST); */
	/* SDL_SetRenderLogicalPresentation(app_state->renderer, app_state->w_pixels, */
	/* 		app_state->h_pixels, SDL_LOGICAL_PRESENTATION_DISABLED); */
	/* SDL_SetTextureBlendMode(app_state->window_texture, SDL_BLENDMODE_NONE); */
	/* SDL_SetRenderDrawColor(app_state->renderer, 255, 255, 255, 255); */
  /* SDL_RenderClear(app_state->renderer); */
  if (SDL_LockTexture(app_state->window_texture, NULL, &pixels, &pitch)) {
		/* printf("pitch: %d\n", pitch); */
    uint32_t *pixs = pixels;
		for (int i = 0; i < app_state->w_pixels*app_state->h_pixels; i++) {
			pixs[i] = 0xFFFFFFFF;
		}

    for (int p2d_cnt = 0; p2d_cnt < objects->point_2D_buf.length; p2d_cnt++) {
			int x = SDL_lround(((Point2D *)objects->point_2D_buf.data)[p2d_cnt].x);
			int y = SDL_lround(((Point2D *)objects->point_2D_buf.data)[p2d_cnt].y);
			pixs[x + y * app_state->w_pixels] = 0x00000000;
		}

		// draw all segments
		Line2D *l_seg = (Line2D *)objects->line_2D_seg_buf.data;
		for (int l2d_cnt = 0; l2d_cnt < objects->line_2D_seg_buf.length; l2d_cnt++) {
			int x0 = SDL_lround(l_seg[l2d_cnt].p0.x);
			int y0 = SDL_lround(l_seg[l2d_cnt].p0.y);
			int x1 = SDL_lround(l_seg[l2d_cnt].p1.x);
			int y1 = SDL_lround(l_seg[l2d_cnt].p1.y);
			double dx = x1 - x0;
			double dy = y1 - y0;
			double y;
			int x;
			double m = dy/dx;
			for (x = x0; x < x1; x++) {
				y = m * (double)(x - x0) + (double)y0;
				if (x < app_state->w_pixels-1 && y < app_state->h_pixels-1) {

					pixs[x + SDL_lround(y) * app_state->w_pixels] = l_seg[l2d_cnt].color;
				}
			}
		}

		// draw all primitive lines
		Line2D *l = (Line2D *)objects->line_2D_buf.data;
		// not draw atm
		/* for (int l2d_cnt = 0; l2d_cnt < objects->line_2D_buf.length; l2d_cnt++) { */
		/* 	int x0 = SDL_lround(l[l2d_cnt].p0.x); */
		/* 	int y0 = SDL_lround(l[l2d_cnt].p0.y); */
		/* 	int x1 = SDL_lround(l[l2d_cnt].p1.x); */
		/* 	int y1 = SDL_lround(l[l2d_cnt].p1.y); */
		/* 	double dx = x1 - x0; */
		/* 	double dy = y1 - y0; */
		/* 	double y; */
		/* 	int x; */
		/* 	double m = dy/dx; */
		/* 	for (x = x0; x < x1; x++) { */
		/* 		y = m * (double)(x - x0) + (double)y0; */
		/* 		if (x < app_state->w_pixels-1 && y < app_state->h_pixels-1) { */
		/* 			pixs[x + SDL_lround(y) * app_state->w_pixels] = l[l2d_cnt].color; */
		/* 		} */
		/* 	} */
		/* } */


		Circle2D *c = (Circle2D *)objects->circle_2D_buf.data;
		for (int c2d_cnt = 0; c2d_cnt < objects->circle_2D_buf.length; c2d_cnt++) {
			int cx = SDL_lround(c[c2d_cnt].center.x);
			int cy = SDL_lround(c[c2d_cnt].center.y);
			for (int x = SDL_lround(cx - c[c2d_cnt].radius); x < SDL_lround(cx + c[c2d_cnt].radius); x++) {
				double val = SDL_pow((c[c2d_cnt].radius), 2.0) - SDL_pow((double)(x - cx), 2.0);
				if (val < 0) {
					val = 0.0;
				}
				/* double y = SDL_sqrt(val); */
				/* printf("%f\n", y); */
        double y_offset = SDL_sqrt(val);
        int y_top = cy - SDL_lround(y_offset);
        int y_bottom = cy + SDL_lround(y_offset);

				/* printf("y_top:%d, r:%f, \n", y_top, c[c2d_cnt].radius); */
        // Draw the top and bottom points of the circle's circumference:
        if (x >= 0 && x < app_state->w_pixels) {
            if (y_top >= 0 && y_top < app_state->h_pixels)
                pixs[x + y_top * app_state->w_pixels] = 0x00000000;
            if (y_bottom >= 0 && y_bottom < app_state->h_pixels)
                pixs[x + y_bottom * app_state->w_pixels] = 0x00000000;
        }
				/* pixs[x + SDL_lround(y_offset) * app_state->w_pixels] = 0x00000000; */
			}
		}


		// is_circles
		Circle2D *is_c = (Circle2D *)objects->is_circle_buf.data;
		for (int c2d_cnt = 0; c2d_cnt < objects->is_circle_buf.length; c2d_cnt++) {
			int cx = SDL_lround(is_c[c2d_cnt].center.x);
			int cy = SDL_lround(is_c[c2d_cnt].center.y);
			for (int x = SDL_lround(cx - is_c[c2d_cnt].radius); x < SDL_lround(cx + is_c[c2d_cnt].radius); x++) {
				double val = SDL_pow((is_c[c2d_cnt].radius), 2.0) - SDL_pow((double)(x - cx), 2.0);
				if (val < 0) {
					val = 0.0;
				}
				/* double y = SDL_sqrt(val); */
				/* printf("%f\n", y); */
        double y_offset = SDL_sqrt(val);
        int y_top = cy - SDL_lround(y_offset);
        int y_bottom = cy + SDL_lround(y_offset);

				/* printf("y_top:%d, r:%f, \n", y_top, c[c2d_cnt].radius); */
        // Draw the top and bottom points of the circle's circumference:
        if (x >= 0 && x < app_state->w_pixels) {
            if (y_top >= 0 && y_top < app_state->h_pixels)
                pixs[x + y_top * app_state->w_pixels] = 0x00000000;
            if (y_bottom >= 0 && y_bottom < app_state->h_pixels)
                pixs[x + y_bottom * app_state->w_pixels] = 0x00000000;
        }
				/* pixs[x + SDL_lround(y_offset) * app_state->w_pixels] = 0x00000000; */
			}
		}

    SDL_UnlockTexture(app_state->window_texture);
  }

	/* SDL_SetRenderDrawColor(app_state->renderer, 255, 0, 0, 255); */
	/* SDL_RenderLine(app_state->renderer, 10, 10, 400, 400); */
  SDL_RenderTexture(app_state->renderer, app_state->window_texture, NULL, NULL);
  SDL_RenderPresent(app_state->renderer);
}

void vector_append(ObjectBuf *buf)
{
	if (buf->length >= buf->capacity) {
		buf->capacity += 32;
		buf->data = SDL_realloc(buf->data, buf->object_size * buf->capacity);
	}
	buf->length++;
}

void vector_destroy(ObjectBuf *buf)
{
	SDL_free(buf);
}
