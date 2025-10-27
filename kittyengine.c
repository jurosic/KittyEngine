#include <stddef.h>
#include <SDL2/SDL.h>

#include "kittyengine.h"

typedef struct {
    size_t total_allocated;
    size_t allocation_count;
    Kitty_Object* objects;
} k_ObjectMSpace;

static const char* window_title = "Kitty Engine Window";
static int window_width = 800;
static int window_height = 600;

static const size_t K_DEFAULT_OBJECT_MSPACE_SIZE = 1024 * 1024; // 1 MB

static k_ObjectMSpace* object_mspace = NULL;

//SDL VARS
static SDL_Window* sdl_window = NULL;
static SDL_Renderer* sdl_renderer = NULL;

///@brief Creates the memory space (dynamic array) that houses objects.
static int k_CreateObjectMSpace();
///@brief Allocates more space in the object memory space.
static int k_AllocObjectMSpace();
///@brief Allocates less space in the object memory space.
static int k_UnallocObjectMSpace();
///@brief Checks whether reallocation is needed and performs it.
static int k_ReallocObjectMSpace();
///@brief Frees (resets) the object memory space.
static int k_FreeObjectMSpace();
///@brief Destroys the object memory space.
static int k_DestroyObjectMSpace();


int Kitty_Init(const char* title, int width, int height){
    window_title = title;
    window_width = width;
    window_height = height;

    size_t result = k_CreateObjectMSpace();
    if (result != KITTY_SUCCESS){
        return result; // Return error code
    }
    result = k_AllocObjectMSpace();
    if (result != KITTY_SUCCESS){
        return result; // Return error code
    }

    //create window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return KITTY_SDL_INIT_ERROR; // SDL initialization failed
    }

    sdl_window = SDL_CreateWindow(window_title,
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  window_width, window_height,
                                  SDL_WINDOW_SHOWN);

    if (!sdl_window) {
        SDL_Quit();
        return KITTY_SDL_WINDOW_CREATION_ERROR; // SDL window creation failed
    }

    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl_renderer) {
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return KITTY_SDL_RENDERER_CREATION_ERROR; // SDL renderer creation failed
    }

    return KITTY_SUCCESS; // Success
}

int Kitty_Quit() {
    size_t result = k_FreeObjectMSpace();
    if (result != KITTY_SUCCESS){
        return result; // Return error code
    }
    result = k_DestroyObjectMSpace();
    if (result != KITTY_SUCCESS){
        return result; // Return error code
    }

    //destroy SDL stuff
    if (sdl_renderer) {
        SDL_DestroyRenderer(sdl_renderer);
        sdl_renderer = NULL;
    }
    if (sdl_window) {
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }
    SDL_Quit();

    return KITTY_SUCCESS; // Success
}

int Kitty_ClearScreen(Kitty_Color color) {
    if (!sdl_renderer) {
        return KITTY_SDL_RENDERER_NOT_INITIALIZED; // SDL renderer not initialized
    }
    SDL_SetRenderDrawColor(sdl_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(sdl_renderer);
    return KITTY_SUCCESS; // Success
}

int Kitty_FlipBuffers() {
    if (!sdl_renderer) {
        return KITTY_SDL_RENDERER_NOT_INITIALIZED; // SDL renderer not initialized
    }
    SDL_RenderPresent(sdl_renderer);
    return KITTY_SUCCESS; // Success
}

int Kitty_UpdateObjectState() {
    // Placeholder for future update logic
    return KITTY_SUCCESS; // Success
}

int Kitty_RenderObjects() {
    for (size_t i = 0; i < object_mspace->allocation_count; i++) {
        Kitty_Object obj = object_mspace->objects[i];
        // Render based on object type
        switch (obj.type) {
            case KITTY_OBJECT_CIRCLE:
                // Render circle
                Kitty_ObjCircle* c_obj = (typeof(Kitty_ObjCircle)*)obj.data;
                Kitty_Color col = c_obj->color;

                SDL_SetRenderDrawColor(sdl_renderer, col.r, col.g, col.b, col.a);
                
                if (c_obj->filled) {
                    // Render filled circle
                    for (int w = 0; w < c_obj->radius * 2; w++) {
                        for (int h = 0; h < c_obj->radius * 2; h++) {
                            int dx = c_obj->radius - w; // horizontal offset
                            int dy = c_obj->radius - h; // vertical offset
                            if ((dx*dx + dy*dy) <= (c_obj->radius * c_obj->radius)) {
                                SDL_RenderDrawPoint(sdl_renderer, c_obj->x + dx, c_obj->y + dy);
                            }
                        }
                    }
                } else {
                    // Render circle outline
                    int x = c_obj->radius - 1;
                    int y = 0;
                    int dx = 1;
                    int dy = 1;
                    int err = dx - ((int)c_obj->radius << 1);
                    while (x >= y) {
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->x + x, c_obj->y + y);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->x + y, c_obj->y + x);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->x - y, c_obj->y + x);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->x - x, c_obj->y + y);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->x - x, c_obj->y - y);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->x - y, c_obj->y - x);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->x + y, c_obj->y - x);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->x + x, c_obj->y - y);

                        if (err <= 0) {
                            y++;
                            err += dy;
                            dy += 2;
                        }
                        if (err > 0) {
                            x--;
                            dx += 2;
                            err += dx - ((int)c_obj->radius << 1);
                        }
                    }
                }


                break;
            case KITTY_OBJECT_RECTANGLE:
                // Render rectangle
                Kitty_ObjRectangle* r_obj = (typeof(Kitty_ObjRectangle)*)obj.data;
                Kitty_Color rect_col = r_obj->color;
                SDL_Rect rect = {r_obj->x, r_obj->y, r_obj->width, r_obj->height};
                SDL_SetRenderDrawColor(sdl_renderer, rect_col.r, rect_col.g, rect_col.b, rect_col.a);
                if (r_obj->filled) {
                    SDL_RenderFillRect(sdl_renderer, &rect);
                } else {
                    SDL_RenderDrawRect(sdl_renderer, &rect);
                }

                break;
            case KITTY_OBJECT_LINE:
                // Render line 
                Kitty_ObjLine* l_obj = (typeof(Kitty_ObjLine)*)obj.data;
                Kitty_Color line_col = l_obj->color;
                SDL_SetRenderDrawColor(sdl_renderer, line_col.r, line_col.g, line_col.b, line_col.a);
                SDL_RenderDrawLine(sdl_renderer, l_obj->x1, l_obj->y1, l_obj->x2, l_obj->y2);

                break;
            default:
                return KITTY_UNKNOWN_ERROR; // Unknown object type
        }
    }
    return KITTY_SUCCESS; // Success
}

void Kitty_Clock(int fps) {
    SDL_Delay(1000 / fps);
}

int Kitty_AddObject(Kitty_Object obj) {
    if (!object_mspace) {
        return KITTY_MEMORYSPACE_NOT_INITIALIZED; // Memory space not initialized
    }
    size_t result = k_ReallocObjectMSpace();
    if (result != KITTY_SUCCESS) {
        return result; // Return error code
    }
    object_mspace->objects[object_mspace->allocation_count] = obj;
    object_mspace->allocation_count++;
    return KITTY_SUCCESS; // Success
}

int Kitty_RemoveObject(size_t index) {
    if (!object_mspace) {
        return KITTY_MEMORYSPACE_NOT_INITIALIZED; // Memory space not initialized
    }
    if (index >= object_mspace->allocation_count) {
        return KITTY_INVALID_OBJECT_INDEX; // Invalid object index
    }
    // Shift objects down to fill the gap
    for (size_t i = index; i < object_mspace->allocation_count - 1; i++) {
        object_mspace->objects[i] = object_mspace->objects[i + 1];
    }
    object_mspace->allocation_count--;
    size_t result = k_ReallocObjectMSpace();
    if (result != KITTY_SUCCESS) {
        return result; // Return error code
    }
    return KITTY_SUCCESS; // Success
}

int Kitty_GetObject(size_t index, Kitty_Object* out_obj) {
    if (!object_mspace) {
        return KITTY_MEMORYSPACE_NOT_INITIALIZED; // Memory space not initialized
    }
    if (index >= object_mspace->allocation_count) {
        return KITTY_INVALID_OBJECT_INDEX; // Invalid object index
    }
    *out_obj = object_mspace->objects[index];
    return KITTY_SUCCESS; // Success
}

Kitty_Object* Kitty_CreateCircle(int x, int y, float radius, bool filled, Kitty_Color color) {
    Kitty_Object* obj = (Kitty_Object*)malloc(sizeof(Kitty_Object));
    if (!obj) {
        return NULL; // Memory allocation failed
    }
    obj->type = KITTY_OBJECT_CIRCLE;
    obj->data = malloc(sizeof(Kitty_ObjCircle));
    if (!obj->data) {
        free(obj);
        return NULL; // Memory allocation failed
    }
    Kitty_ObjCircle* circle_data = (Kitty_ObjCircle*)obj->data;
    circle_data->x = x;
    circle_data->y = y;
    circle_data->radius = radius;
    circle_data->filled = filled;
    circle_data->color = color;
    return obj;
}

Kitty_Object* Kitty_CreateRectangle(int x, int y, int width, int height, bool filled, Kitty_Color color) {
    Kitty_Object* obj = (Kitty_Object*)malloc(sizeof(Kitty_Object));
    if (!obj) {
        return NULL; // Memory allocation failed
    }
    obj->type = KITTY_OBJECT_RECTANGLE;
    obj->data = malloc(sizeof(Kitty_ObjRectangle));
    if (!obj->data) {
        free(obj);
        return NULL; // Memory allocation failed
    }
    Kitty_ObjRectangle* rect_data = (Kitty_ObjRectangle*)obj->data;
    rect_data->x = x;
    rect_data->y = y;
    rect_data->width = width;
    rect_data->height = height;
    rect_data->filled = filled;
    rect_data->color = color;
    return obj;
}

Kitty_Object* Kitty_CreateLine(int x1, int y1, int x2, int y2, Kitty_Color color) {
    Kitty_Object* obj = (Kitty_Object*)malloc(sizeof(Kitty_Object));
    if (!obj) {
        return NULL; // Memory allocation failed
    }
    obj->type = KITTY_OBJECT_LINE;
    obj->data = malloc(sizeof(Kitty_ObjLine));
    if (!obj->data) {
        free(obj);
        return NULL; // Memory allocation failed
    }
    Kitty_ObjLine* line_data = (Kitty_ObjLine*)obj->data;
    line_data->x1 = x1;
    line_data->y1 = y1;
    line_data->x2 = x2;
    line_data->y2 = y2;
    line_data->color = color;
    return obj;
}









// MEMORY STUFF

static int k_CreateObjectMSpace(){
    object_mspace = (k_ObjectMSpace*)malloc(sizeof(k_ObjectMSpace));
    if (!object_mspace){
        return KITTY_MEMORY_ALLOCATION_FAILURE; // Memory allocation failed
    }
    object_mspace->total_allocated = 0;
    object_mspace->allocation_count = 0;
    object_mspace->objects = NULL;
    return KITTY_SUCCESS; // Success
}

static int k_AllocObjectMSpace(){
    if (!object_mspace){
        return KITTY_MEMORYSPACE_NOT_INITIALIZED; // Memory space not initialized
    }
    size_t new_size = object_mspace->total_allocated + K_DEFAULT_OBJECT_MSPACE_SIZE;
    Kitty_Object* new_objects = (Kitty_Object*)realloc(object_mspace->objects, new_size);
    if(!new_objects){
        return KITTY_MEMORY_ALLOCATION_FAILURE; // Memory allocation failed
    }
    object_mspace->objects = new_objects;
    object_mspace->total_allocated = new_size;
    return KITTY_SUCCESS; // Success
}

static int k_UnallocObjectMSpace(){
    if (!object_mspace){
        return KITTY_MEMORYSPACE_NOT_INITIALIZED; // Memory space not initialized
    }
    // subtract allocation count and realloc
    size_t new_size = object_mspace->total_allocated - K_DEFAULT_OBJECT_MSPACE_SIZE;
    Kitty_Object* new_objects = (Kitty_Object*)realloc(object_mspace->objects, new_size);
    if(!new_objects){
        return KITTY_MEMORY_ALLOCATION_FAILURE; // Memory allocation failed
    }
    object_mspace->objects = new_objects;
    object_mspace->total_allocated = new_size;
    return KITTY_SUCCESS; // Success
}

static int k_ReallocObjectMSpace(){
    if (!object_mspace){
        return KITTY_MEMORYSPACE_NOT_INITIALIZED; // Memory space not initialized
    }
    if (object_mspace->allocation_count * sizeof(Kitty_Object) >= object_mspace->total_allocated){
        return k_AllocObjectMSpace(); // Allocate more space
    } else if (object_mspace->allocation_count * sizeof(Kitty_Object) < object_mspace->total_allocated / 4 && object_mspace->total_allocated > K_DEFAULT_OBJECT_MSPACE_SIZE){
        return k_UnallocObjectMSpace(); // Deallocate space
    }
    return KITTY_SUCCESS; // Success
}

static int k_FreeObjectMSpace(){
    if(!object_mspace){
        return KITTY_MEMORYSPACE_NOT_INITIALIZED; // Memory space not initialized
    }

    //loop thru objects and free their data
    for (size_t i = 0; i < object_mspace->allocation_count; i++){
        free(object_mspace->objects[i].data);
        object_mspace->objects[i].data = NULL;
    }

    free(object_mspace->objects);
    object_mspace->objects = NULL;
    object_mspace->total_allocated = 0;
    object_mspace->allocation_count = 0;
    return KITTY_SUCCESS; // Success
}

static int k_DestroyObjectMSpace(){
    if (!object_mspace){
        return KITTY_MEMORYSPACE_NOT_INITIALIZED; // Memory space not initialized
    }
    if (object_mspace->allocation_count > 0 || object_mspace->total_allocated > 0 || object_mspace->objects != NULL){ 
        return KITTY_MEMORYSPACE_DATA_NOT_FREED; // Data not freed
    }
    return KITTY_SUCCESS; // Success
}
