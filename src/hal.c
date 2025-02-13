#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stdbool.h>

// Input state structure
typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
    bool action;
    bool quit;
} InputState;

// Platform API structure
typedef struct {
    void (*init_graphics)(int width, int height);
    void (*draw_pixel)(int x, int y, uint32_t color);
    void (*clear_screen)(uint32_t color);
    InputState (*process_input)(void);
    void (*delay)(uint32_t ms);
} PlatformAPI;

// Function to get the platform API
const PlatformAPI* get_platform_api(void);

#endif
