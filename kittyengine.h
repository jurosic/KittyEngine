#include <SDL2/SDL.h>
#include <string.h>
#include <stddef.h>

#ifndef KITTYENGINE_H
#define KITTYENGINE_H

#ifndef __STDBOOL_H
#include <stdbool.h>
#endif

/*
 * Kitty Engine API Data Structures and Enums
 */

///@brief Error codes for Kitty Engine functions.
///success - 0
///general failures: 1-99
///memory failures: 100-199
///SDL failures: 1000-1999
enum Kitty_ErrorCodes {
    KITTY_SUCCESS = 0,

    KITTY_INIT_FAILURE = 1,
    KITTY_SDL_WINDOW_NOT_INITIALIZED = 2,
    KITTY_SDL_RENDERER_NOT_INITIALIZED = 3,

    KITTY_MEMORY_ALLOCATION_FAILURE = 100,
    KITTY_MEMORYSPACE_NOT_INITIALIZED = 101,
    KITTY_MEMORYSPACE_DATA_NOT_FREED = 102,
    KITTY_INVALID_OBJECT_INDEX = 103,

    KITTY_SDL_INIT_ERROR = 1000,
    KITTY_SDL_WINDOW_CREATION_ERROR = 1001,
    KITTY_SDL_RENDERER_CREATION_ERROR = 1002,

    KITTY_UNKNOWN_ERROR = 9999
};

enum Kitty_ObjType {
    KITTY_OBJECT_CIRCLE,
    KITTY_OBJECT_RECTANGLE,
    KITTY_OBJECT_LINE
};

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} Kitty_Color;

typedef struct {
    Kitty_Color startColor;
    Kitty_Color endColor;
} Kitty_ColorGradient;

typedef struct {
    int x;
    int y;
    float radius;
    bool filled;
    Kitty_Color color;
} Kitty_ObjCircle;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    bool filled;
    Kitty_Color color;
} Kitty_ObjRectangle;

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
    Kitty_Color color;
} Kitty_ObjLine;

typedef struct {
    enum Kitty_ObjType type;
    void* data;

} Kitty_Object;

/*
 * Kitty Engine API Functions
 */

///@brief Initializes the Kitty Engine with a window of given title and dimensions.
///@param title The title of the window.
///@param width The width of the window.
///@param height The height of the window.
///@return Returns 0 on success, or an error code on failure.
int Kitty_Init(const char* title, int width, int height);

///@brief Cleans up and quits the Kitty Engine.
///@return Returns 0 on success, or an error code on failure.
int Kitty_Quit();

///@brief Clears the screen with the specified color.
///@param r Red component (0-255).
///@param g Green component (0-255).
///@param b Blue component (0-255).
///@param a Alpha component (0-255).
///@return Returns 0 on success, or an error code on failure.
int Kitty_ClearScreen(Kitty_Color color);

///@brief Flips the buffers to present the rendered content to the screen.
///@return Returns 0 on success, or an error code on failure.
int Kitty_FlipBuffers();

///@brief Updates the engine state. Should be called once per frame.
///@return Returns 0 on success, or an error code on failure.
int Kitty_UpdateObjectState();
int Kitty_RenderObjects();

void Kitty_Clock(int fps);

int Kitty_AddObject(Kitty_Object obj);

int Kitty_RemoveObject(size_t index);

int Kitty_GetObject(size_t index, Kitty_Object* out_obj);

Kitty_Object* Kitty_CreateCircle(int x, int y, float radius, bool filled, Kitty_Color color);
Kitty_Object* Kitty_CreateRectangle(int x, int y, int width, int height, bool filled, Kitty_Color color);
Kitty_Object* Kitty_CreateLine(int x1, int y1, int x2, int y2, Kitty_Color color);

#endif // KITTYENGINE_H