#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

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
    KITTY_SDL_LOCK_TEXTURE_ERROR = 4,
    KITTY_FILE_NOT_FOUND = 5,

    KITTY_MEMORY_ALLOCATION_FAILURE = 100,
    KITTY_MEMORYSPACE_NOT_INITIALIZED = 101,
    KITTY_MEMORYSPACE_DATA_NOT_FREED = 102,
    KITTY_INVALID_OBJECT_INDEX = 103,

    KITTY_SDL_INIT_ERROR = 1000,
    KITTY_SDL_WINDOW_CREATION_ERROR = 1001,
    KITTY_SDL_RENDERER_CREATION_ERROR = 1002,
    KITTY_SDL_TTF_ERROR = 1003,

    KITTY_UNKNOWN_ERROR = 9999
};

enum Kitty_ObjType {
    KITTY_OBJECT_CIRCLE,
    KITTY_OBJECT_RECTANGLE,
    KITTY_OBJECT_LINE,
    KITTY_OBJECT_TRIANGLE,
    KITTY_OBJECT_PIXEL,
    KITTY_OBJECT_MESH,
    KITTY_OBJECT_TEXT
};

typedef struct {
    int x;
    int y;
} Kitty_Point;

typedef struct {
    int x;
    int y;
    int z;
} Kitty_Point3D;

typedef struct {
    float x;
    float y;
    float z;
} Kitty_Vertex3D;

typedef struct{
    float u;
    float v;
} Kitty_UV;

typedef struct {
    int a;
    int b;
    int c;
    int uv_a;
    int uv_b;
    int uv_c;
} Kitty_Face;

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
    SDL_Surface* sdl_surface;
} Kitty_Texture;

typedef struct {
    Kitty_Point position;
    float radius;
    bool filled;
    Kitty_Color color;
} Kitty_ObjCircle;

typedef struct {
    Kitty_Point position;
    int width;
    int height;
    bool filled;
    Kitty_Color color;
} Kitty_ObjRectangle;

typedef struct {
    Kitty_Point startPoint;
    Kitty_Point endPoint;
    Kitty_Color color;
} Kitty_ObjLine;

typedef struct {
    Kitty_Point vertex1;
    Kitty_Point vertex2;
    Kitty_Point vertex3;
    Kitty_Color color;
    bool filled;
} Kitty_ObjTriangle;

typedef struct {
    Kitty_Point position;
    Kitty_Color color;
} Kitty_ObjPixel;

typedef struct {
    Kitty_Point3D position;
    Kitty_Vertex3D origin;
    float scale;
    bool wrap;
    bool wire;
    Kitty_Vertex3D* vertices;
    Kitty_Face* faces;
    Kitty_Color* face_colors;
    Kitty_UV* uvs;
    Kitty_Texture* texture;
    size_t uv_count;
    size_t vertex_count;
    size_t face_count;
} Kitty_ObjMesh;

typedef struct {
    Kitty_Point position;
    float size;
    float rotation;
    Kitty_Color color;
    char* text;
} Kitty_ObjText;

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
int Kitty_ClearObjects();

void Kitty_Clock(int fps);

int Kitty_AddObject(Kitty_Object obj);

int Kitty_RemoveObject(size_t index);

int Kitty_GetObject(size_t index, Kitty_Object* out_obj);

int Kitty_AddVertexToObjMesh(Kitty_Object* obj, Kitty_Vertex3D vertex);
int Kitty_AddFaceToObjMesh(Kitty_Object* obj, Kitty_Face face, Kitty_Color face_color);
int Kitty_AddUVToObjMesh(Kitty_Object* obj, Kitty_UV uv);

Kitty_Texture* Kitty_LoadTexture(const char* file_path);
int Kitty_LoadDotObj(FILE* file, Kitty_Object* mesh);

size_t Kitty_GetFrameNumber();
clock_t Kitty_GetDeltaTime();
double Kitty_GetFrameTime();

void Kitty_SetTimer1();
bool Kitty_Timer1Trip(long miliseconds);

void Kitty_RotateCamera(float angle_x, float angle_y, float angle_z);
Kitty_Vertex3D Kitty_GetCameraPosition();
void Kitty_SetCameraPosition(Kitty_Vertex3D position);
void Kitty_SetCameraOrigin(Kitty_Point3D origin);

Kitty_Object* Kitty_CreateCircle(Kitty_Point position, float radius, bool filled, Kitty_Color color);
Kitty_Object* Kitty_CreateRectangle(Kitty_Point position, int width, int height, bool filled, Kitty_Color color);
Kitty_Object* Kitty_CreateLine(Kitty_Point startPosition, Kitty_Point endPosition, Kitty_Color color);
Kitty_Object* Kitty_CreateTriangle(Kitty_Point vertex1, Kitty_Point vertex2, Kitty_Point vertex3, bool filled, Kitty_Color color);
Kitty_Object* Kitty_CreatePixel(Kitty_Point position, Kitty_Color color);
Kitty_Object* Kitty_CreateMesh();
Kitty_Object* Kitty_CreateText(Kitty_Point position, float rotation, float size, Kitty_Color color, const char* text);

int Kitty_Transform(Kitty_Object* obj, Kitty_Point3D translation, Kitty_Vertex3D rotation);

Kitty_Vertex3D KittyM_CalculateMeshCenter(Kitty_ObjMesh* mesh);
float KittyM_DotProduct3(Kitty_Vertex3D v1, Kitty_Vertex3D v2);
float KittyM_VectorLength3(Kitty_Vertex3D v);
Kitty_Vertex3D KittyM_Point2PointV3(Kitty_Vertex3D from, Kitty_Vertex3D to);
Kitty_Vertex3D KittyM_CrossProduct3(Kitty_Vertex3D v1, Kitty_Vertex3D v2);
Kitty_Vertex3D KittyM_VectorNormalize3(Kitty_Vertex3D v);
Kitty_Vertex3D KittyM_RotateVertex3D_X(Kitty_Vertex3D v, float angle);
Kitty_Vertex3D KittyM_RotateVertex3D_Y(Kitty_Vertex3D v, float angle);
Kitty_Vertex3D KittyM_RotateVertex3D_Z(Kitty_Vertex3D v, float angle);

int KittyD_DrawMeshUVMap(Kitty_Point position, int scale, Kitty_ObjMesh* mesh);
int KittyD_DrawTexture(Kitty_Point position, int scale, Kitty_Texture* texture);

#endif // KITTYENGINE_H