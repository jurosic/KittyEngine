#include <stddef.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <sys/time.h>
#include <math.h>


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

static size_t frame_num = 0;
static clock_t start_time = 0;
static double frame_time = 0;

static clock_t timer_1 = 0;

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

    //init ttf
    if (TTF_Init() == -1) {
        SDL_Quit();
        return KITTY_SDL_INIT_ERROR; // SDL_ttf initialization failed
    }

    //start time in ms
    start_time = clock();

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

    // Destroy SDL stuff
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
    if (!sdl_renderer) {
        return KITTY_SDL_RENDERER_NOT_INITIALIZED; // SDL renderer not initialized
    }

    clock_t start = clock();
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
                                SDL_RenderDrawPoint(sdl_renderer, c_obj->position.x + dx, c_obj->position.y + dy);
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
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->position.x + x, c_obj->position.y + y);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->position.x + y, c_obj->position.y + x);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->position.x - y, c_obj->position.y + x);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->position.x - x, c_obj->position.y + y);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->position.x - x, c_obj->position.y - y);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->position.x - y, c_obj->position.y - x);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->position.x + y, c_obj->position.y - x);
                        SDL_RenderDrawPoint(sdl_renderer, c_obj->position.x + x, c_obj->position.y - y);

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
                SDL_Rect rect = {r_obj->position.x, r_obj->position.y, r_obj->width, r_obj->height};
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
                SDL_RenderDrawLine(sdl_renderer, l_obj->startPoint.x, l_obj->startPoint.y, l_obj->endPoint.x, l_obj->endPoint.y);

                break;

            case KITTY_OBJECT_TRIANGLE:
                // Render triangle
                Kitty_ObjTriangle* t_obj = (typeof(Kitty_ObjTriangle)*)obj.data;
                Kitty_Color tri_col = t_obj->color;
                SDL_SetRenderDrawColor(sdl_renderer, tri_col.r, tri_col.g, tri_col.b, tri_col.a);
                SDL_RenderDrawLine(sdl_renderer, t_obj->vertex1.x, t_obj->vertex1.y, t_obj->vertex2.x, t_obj->vertex2.y);
                SDL_RenderDrawLine(sdl_renderer, t_obj->vertex2.x, t_obj->vertex2.y, t_obj->vertex3.x, t_obj->vertex3.y);
                SDL_RenderDrawLine(sdl_renderer, t_obj->vertex3.x, t_obj->vertex3.y, t_obj->vertex1.x, t_obj->vertex1.y);

                if (t_obj->filled){
                    // Color in triangle using points, simple scanline fill
                    int minY = t_obj->vertex1.y < t_obj->vertex2.y ? (t_obj->vertex1.y < t_obj->vertex3.y ? t_obj->vertex1.y : t_obj->vertex3.y) : (t_obj->vertex2.y < t_obj->vertex3.y ? t_obj->vertex2.y : t_obj->vertex3.y);
                    int maxY = t_obj->vertex1.y > t_obj->vertex2.y ? (t_obj->vertex1.y > t_obj->vertex3.y ? t_obj->vertex1.y : t_obj->vertex3.y) : (t_obj->vertex2.y > t_obj->vertex3.y ? t_obj->vertex2.y : t_obj->vertex3.y);

                    for (int y = minY; y <= maxY; y++){
                        int nodes = 0;
                        int nodeX[3];
                        Kitty_Point* vertices[3] = {&t_obj->vertex1, &t_obj->vertex2, &t_obj->vertex3};
                        for (int i = 0; i < 3; i++){
                            Kitty_Point* v1 = vertices[i];
                            Kitty_Point* v2 = vertices[(i + 1) % 3];
                            if ((v1->y < y && v2->y >= y) || (v2->y < y && v1->y >= y)){
                                nodeX[nodes++] = v1->x + (y - v1->y) * (v2->x - v1->x) / (v2->y - v1->y);
                            }
                        }
                        for (int i = 0; i < nodes - 1; i += 2){
                            if (nodeX[i] > nodeX[i + 1]){
                                int temp = nodeX[i];
                                nodeX[i] = nodeX[i + 1];
                                nodeX[i + 1] = temp;
                            }
                            SDL_RenderDrawLine(sdl_renderer, nodeX[i], y, nodeX[i + 1], y);
                        }
                    }
                }

                break;

            case KITTY_OBJECT_PIXEL:
                // Render pixel
                Kitty_ObjPixel* p_obj = (typeof(Kitty_ObjPixel)*)obj.data;
                Kitty_Color pixel_col = p_obj->color;
                SDL_SetRenderDrawColor(sdl_renderer, pixel_col.r, pixel_col.g, pixel_col.b, pixel_col.a);
                SDL_RenderDrawPoint(sdl_renderer, p_obj->position.x, p_obj->position.y);

                break;

            case KITTY_OBJECT_TEXT:
                //load font here to add multiple font support
                Kitty_ObjText* text_obj = (typeof(Kitty_ObjText)*)obj.data;
                TTF_Font* font = TTF_OpenFont("arial.ttf", 24); // Load a font
                if (!font) {
                    return KITTY_SDL_TTF_ERROR; // Font loading failed
                }

                SDL_Color sdl_color = {text_obj->color.r, text_obj->color.g, text_obj->color.b, text_obj->color.a};
                SDL_Surface* text_surface = TTF_RenderText_Solid(font, text_obj->text, sdl_color);
                if (!text_surface) {
                    TTF_CloseFont(font);
                    return KITTY_SDL_TTF_ERROR; // Text rendering failed
                }
                SDL_Texture* text_texture = SDL_CreateTextureFromSurface(sdl_renderer, text_surface);
                if (!text_texture) {
                    SDL_FreeSurface(text_surface);
                    TTF_CloseFont(font);
                    return KITTY_SDL_TTF_ERROR; // Texture creation failed
                }
                SDL_Rect text_rect = {text_obj->position.x, text_obj->position.y, text_surface->w, text_surface->h};
                SDL_RenderCopy(sdl_renderer, text_texture, NULL, &text_rect);
                SDL_DestroyTexture(text_texture);
                SDL_FreeSurface(text_surface);
                TTF_CloseFont(font);


                break;

            case KITTY_OBJECT_MESH:
                //turn mesh into triangles (wireframe for now)

                Kitty_ObjMesh* m_obj = (typeof(Kitty_ObjMesh)*)obj.data;
                Kitty_Point3D position = m_obj->position;

                //sort faces by depth bubble sort
                for (size_t j = 0; j < m_obj->face_count - 1; j++){
                    for (size_t k = 0; k < m_obj->face_count - j - 1; k++){
                        Kitty_Face face1 = m_obj->faces[k];
                        Kitty_Face face2 = m_obj->faces[k + 1];

                        float z1 = (m_obj->vertices[face1.a].z + m_obj->vertices[face1.b].z + m_obj->vertices[face1.c].z) / 3.0f;
                        float z2 = (m_obj->vertices[face2.a].z + m_obj->vertices[face2.b].z + m_obj->vertices[face2.c].z) / 3.0f;

                        if (z1 < z2){
                            //swap
                            Kitty_Face temp_face = m_obj->faces[k];
                            Kitty_Color temp_color = m_obj->face_colors[k];
                            m_obj->faces[k] = m_obj->faces[k + 1];
                            m_obj->face_colors[k] = m_obj->face_colors[k + 1];
                            m_obj->faces[k + 1] = temp_face;
                            m_obj->face_colors[k + 1] = temp_color;
                        }
                    }
                }

                // create wireframe of mesh, each vertex offset by position
                for (size_t f = 0; f < m_obj->face_count; f++){
                    Kitty_Face face = m_obj->faces[f];
                    Kitty_Color face_col = m_obj->face_colors[f];
                    int scale = m_obj->scale;
                    Kitty_Vertex3D v1 = m_obj->vertices[face.a];
                    Kitty_Vertex3D v2 = m_obj->vertices[face.b];
                    Kitty_Vertex3D v3 = m_obj->vertices[face.c];
                    Kitty_UV uv1 = m_obj->uvs[face.uv_a];
                    Kitty_UV uv2 = m_obj->uvs[face.uv_b];
                    Kitty_UV uv3 = m_obj->uvs[face.uv_c];

                    //ugly but we gotta copy to modify
                    float v1x = v1.x;
                    float v1y = v1.y;
                    float v1z = v1.z;

                    float v2x = v2.x;
                    float v2y = v2.y;
                    float v2z = v2.z;

                    float v3x = v3.x;
                    float v3y = v3.y;
                    float v3z = v3.z;


                    float uv1u = uv1.u;
                    float uv1v = uv1.v;

                    float uv2u = uv2.u;
                    float uv2v = uv2.v;

                    float uv3u = uv3.u;
                    float uv3v = uv3.v;

                    /*float uv1u = 0;
                    float uv1v = 0;

                    float uv2u = 0;
                    float uv2v = 0;

                    float uv3u = 0;
                    float uv3v = 0;*/

                    //apply perspective
                    float distance = 100.0f; // Distance from the viewer to the projection plane
                    float persp_1 = distance / (distance + v1z - position.z);
                    float persp_2 = distance / (distance + v2z - position.z);
                    float persp_3 = distance / (distance + v3z - position.z);

                    v1x = v1x * persp_1;
                    v1y = v1y * persp_1;
                    v2x = v2x * persp_2;
                    v2y = v2y * persp_2;
                    v3x = v3x * persp_3;
                    v3y = v3y * persp_3;

                    uv1u = uv1u * persp_1;
                    uv1v = uv1v * persp_1;
                    uv2u = uv2u * persp_2;
                    uv2v = uv2v * persp_2;
                    uv3u = uv3u * persp_3;
                    uv3v = uv3v * persp_3;

                    SDL_SetRenderDrawColor(sdl_renderer, face_col.r, face_col.g, face_col.b, face_col.a);

                    SDL_RenderDrawLine(sdl_renderer,
                                       position.x + (v1x * scale),
                                       position.y + (v1y * scale),
                                       position.x + (v2x * scale),
                                       position.y + (v2y * scale));
                        
                    SDL_RenderDrawLine(sdl_renderer,
                                       position.x + (v2x * scale),
                                       position.y + (v2y * scale),
                                       position.x + (v3x * scale),
                                       position.y + (v3y * scale));

                    SDL_RenderDrawLine(sdl_renderer,
                                       position.x + (v3x * scale),
                                       position.y + (v3y * scale),
                                       position.x + (v1x * scale),
                                       position.y + (v1y * scale));


                    if (!m_obj->wire && !m_obj->wrap){
                        //simple scanline fill
                        int minY = (position.y + (v1y * scale)) < (position.y + (v2y * scale)) ? ((position.y + (v1y * scale)) < (position.y + (v3y * scale)) ? (position.y + (v1y * scale)) : (position.y + (v3y * scale))) : ((position.y + (v2y * scale)) < (position.y + (v3y * scale)) ? (position.y + (v2y * scale)) : (position.y + (v3y * scale)));
                        int maxY = (position.y + (v1y * scale)) > (position.y + (v2y * scale)) ? ((position.y + (v1y * scale)) > (position.y + (v3y * scale)) ? (position.y + (v1y * scale)) : (position.y + (v3y * scale))) : ((position.y + (v2y * scale)) > (position.y + (v3y * scale)) ? (position.y + (v2y * scale)) : (position.y + (v3y * scale)));

                        for (int y = minY; y <= maxY; y++){
                            int nodes = 0;
                            int nodeX[3];
                            Kitty_Point3D* vertices[3] = {
                                &(Kitty_Point3D){position.x + (v1x * scale), position.y + (v1y * scale), position.z + (v1.z * scale)},
                                &(Kitty_Point3D){position.x + (v2x * scale), position.y + (v2y * scale), position.z + (v2.z * scale)},
                                &(Kitty_Point3D){position.x + (v3x * scale), position.y + (v3y * scale), position.z + (v3.z * scale)}
                            };
                            for (int i = 0; i < 3; i++){
                                Kitty_Point3D* v1p = vertices[i];
                                Kitty_Point3D* v2p = vertices[(i + 1) % 3];
                                if ((v1p->y < y && v2p->y >= y) || (v2p->y < y && v1p->y >= y)){
                                    nodeX[nodes++] = v1p->x + (y - v1p->y) * (v2p->x - v1p->x) / (v2p->y - v1p->y);
                                }
                            }
                            for (int i = 0; i < nodes - 1; i += 2){
                                if (nodeX[i] > nodeX[i + 1]){
                                    int temp = nodeX[i];
                                    nodeX[i] = nodeX[i + 1];
                                    nodeX[i + 1] = temp;
                                }
                                SDL_RenderDrawLine(sdl_renderer, nodeX[i], y, nodeX[i + 1], y);
                            }
                        }
                    } 
                    else if (m_obj->wrap) {
                        int minY = (position.y + (v1y * scale)) < (position.y + (v2y * scale)) ? ((position.y + (v1y * scale)) < (position.y + (v3y * scale)) ? (position.y + (v1y * scale)) : (position.y + (v3y * scale))) : ((position.y + (v2y * scale)) < (position.y + (v3y * scale)) ? (position.y + (v2y * scale)) : (position.y + (v3y * scale)));
                        int maxY = (position.y + (v1y * scale)) > (position.y + (v2y * scale)) ? ((position.y + (v1y * scale)) > (position.y + (v3y * scale)) ? (position.y + (v1y * scale)) : (position.y + (v3y * scale))) : ((position.y + (v2y * scale)) > (position.y + (v3y * scale)) ? (position.y + (v2y * scale)) : (position.y + (v3y * scale)));

                        // screen-space vertices
                        Kitty_Point3D* vertices[3] = {
                            &(Kitty_Point3D){position.x + (v1x * scale), position.y + (v1y * scale), position.z + (v1.z * scale)},
                            &(Kitty_Point3D){position.x + (v2x * scale), position.y + (v2y * scale), position.z + (v2.z * scale)},
                            &(Kitty_Point3D){position.x + (v3x * scale), position.y + (v3y * scale), position.z + (v3.z * scale)}
                        };

                        // perspective-correct setup:
                        // uv1u/v, uv2u/v, uv3u/v were pre-multiplied by persp_1/2/3 earlier
                        float U_p[3] = { uv1u, uv2u, uv3u }; // u' = u * persp
                        float V_p[3] = { uv1v, uv2v, uv3v }; // v' = v * persp
                        float W_p[3] = { persp_1, persp_2, persp_3 }; // w' = persp

                        for (int y = minY; y <= maxY; y++){
                            int nodes = 0;
                            int   nodeX[3];
                            float nodeU_p[3], nodeV_p[3], nodeW_p[3];

                            // find edge intersections and interpolate u', v', w' at the intersections
                            for (int i = 0; i < 3; i++){
                                int j = (i + 1) % 3;
                                Kitty_Point3D* v1p = vertices[i];
                                Kitty_Point3D* v2p = vertices[j];
                                if ((v1p->y < y && v2p->y >= y) || (v2p->y < y && v1p->y >= y)){
                                    float dy = (float)v2p->y - (float)v1p->y;
                                    if (fabsf(dy) < 1e-6f) continue; // avoid div by zero
                                    float t = ((float)y - (float)v1p->y) / dy;

                                    nodeX[nodes]   = (int)( (float)v1p->x + t * ((float)v2p->x - (float)v1p->x) );
                                    nodeU_p[nodes] = U_p[i] + t * (U_p[j] - U_p[i]);
                                    nodeV_p[nodes] = V_p[i] + t * (V_p[j] - V_p[i]);
                                    nodeW_p[nodes] = W_p[i] + t * (W_p[j] - W_p[i]);
                                    nodes++;
                                }
                            }

                            if (nodes < 2) continue;

                            // ensure left->right ordering; swap accompanying attributes
                            if (nodeX[0] > nodeX[1]){
                                int   tx = nodeX[0];    nodeX[0] = nodeX[1];    nodeX[1] = tx;
                                float tu = nodeU_p[0];  nodeU_p[0] = nodeU_p[1]; nodeU_p[1] = tu;
                                float tv = nodeV_p[0];  nodeV_p[0] = nodeV_p[1]; nodeV_p[1] = tv;
                                float tw = nodeW_p[0];  nodeW_p[0] = nodeW_p[1]; nodeW_p[1] = tw;
                            }

                            int x0 = nodeX[0];
                            int x1 = nodeX[1];
                            if (x1 == x0) continue;

                            for (int x = x0; x <= x1; x++){
                                float tx = (float)(x - x0) / (float)(x1 - x0);
                                // interpolate u', v', w' across the scanline
                                float u_p = nodeU_p[0] + tx * (nodeU_p[1] - nodeU_p[0]);
                                float v_p = nodeV_p[0] + tx * (nodeV_p[1] - nodeV_p[0]);
                                float w_p = nodeW_p[0] + tx * (nodeW_p[1] - nodeW_p[0]);

                                if (fabsf(w_p) < 1e-8f) continue; // avoid div by zero

                                // recover perspective-correct u, v
                                float u = u_p / w_p;
                                float v = v_p / w_p;

                                //if v, u < 0 or > 1 wrap to other side
                                if (u < 0) u = 1.0f + fmodf(u, 1.0f);
                                if (v < 0) v = 1.0f + fmodf(v, 1.0f);
                                u = fmodf(u, 1.0f);
                                v = fmodf(v, 1.0f);


                                int tex_width  = m_obj->texture->sdl_surface->w;
                                int tex_height = m_obj->texture->sdl_surface->h;
                                int tex_pitch  = m_obj->texture->sdl_surface->pitch;

                                int tex_x = (int)(u * tex_width);
                                int tex_y = (int)(v * tex_height);
                                if ((unsigned)tex_x >= (unsigned)tex_width || (unsigned)tex_y >= (unsigned)tex_height) continue;

                                Uint32* pixels = (Uint32*)m_obj->texture->sdl_surface->pixels;
                                Uint32 color = pixels[tex_y * (tex_pitch / 4) + tex_x];

                                Uint8 r, g, b, a;
                                SDL_GetRGBA(color, m_obj->texture->sdl_surface->format, &r, &g, &b, &a);
                                SDL_SetRenderDrawColor(sdl_renderer, r, g, b, 255);
                                SDL_RenderDrawPoint(sdl_renderer, x, y);
                            }
                        }
                    }
                }

                break;

            default:
                return KITTY_UNKNOWN_ERROR; // Unknown object type
        }
    }
    frame_num++;
    frame_time = (clock() - start) * 1000.0 / CLOCKS_PER_SEC; // in milliseconds
    return KITTY_SUCCESS; // Success
}

int Kitty_ClearObjects() {
    if (!object_mspace) {
        return KITTY_MEMORYSPACE_NOT_INITIALIZED; // Memory space not initialized
    }
    k_FreeObjectMSpace();
    size_t result = k_ReallocObjectMSpace();
    if (result != KITTY_SUCCESS) {
        return result; // Return error code
    }
    return KITTY_SUCCESS; // Success
}

void Kitty_Clock(int fps) {
    SDL_Delay((1000 / fps) - (Kitty_GetFrameTime()));
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

int Kitty_AddVertexToObjMesh(Kitty_Object* obj, Kitty_Vertex3D vertex) {
    if (!obj || obj->type != KITTY_OBJECT_MESH) {
        return KITTY_INVALID_OBJECT_INDEX; // Invalid object or not a mesh
    }
    Kitty_ObjMesh* mesh = (Kitty_ObjMesh*)obj->data;
    size_t new_size = (mesh->vertex_count + 1) * sizeof(Kitty_Vertex3D);
    Kitty_Vertex3D* new_vertices = (Kitty_Vertex3D*)realloc(mesh->vertices, new_size);
    if (!new_vertices) {
        return KITTY_MEMORY_ALLOCATION_FAILURE; // Memory allocation failed
    }
    mesh->vertices = new_vertices;
    mesh->vertices[mesh->vertex_count] = vertex;
    mesh->vertex_count++;
    return KITTY_SUCCESS; // Success
}

int Kitty_AddFaceToObjMesh(Kitty_Object* obj, Kitty_Face face, Kitty_Color face_color) {
    if (!obj || obj->type != KITTY_OBJECT_MESH) {
        return KITTY_INVALID_OBJECT_INDEX; // Invalid object or not a mesh
    }
    Kitty_ObjMesh* mesh = (Kitty_ObjMesh*)obj->data;
    size_t new_size = (mesh->face_count + 1) * sizeof(Kitty_Face);
    Kitty_Face* new_faces = (Kitty_Face*)realloc(mesh->faces, new_size);
    Kitty_Color* new_face_colors = (Kitty_Color*)realloc(mesh->face_colors, new_size);
    if (!new_faces) {
        return KITTY_MEMORY_ALLOCATION_FAILURE; // Memory allocation failed
    }
    if (!new_face_colors) {
        return KITTY_MEMORY_ALLOCATION_FAILURE; // Memory allocation failed
    }

    mesh->faces = new_faces;
    mesh->face_colors = new_face_colors;
    mesh->faces[mesh->face_count] = face;
    mesh->face_colors[mesh->face_count] = face_color;
    mesh->face_count++;
    return KITTY_SUCCESS; // Success
}

int Kitty_AddUVToObjMesh(Kitty_Object* obj, Kitty_UV uv) {
    if (!obj || obj->type != KITTY_OBJECT_MESH) {
        return KITTY_INVALID_OBJECT_INDEX; // Invalid object or not a mesh
    }
    Kitty_ObjMesh* mesh = (Kitty_ObjMesh*)obj->data;
    size_t new_size = (mesh->uv_count + 1) * sizeof(Kitty_UV);
    Kitty_UV* new_uvs = (Kitty_UV*)realloc(mesh->uvs, new_size);
    if (!new_uvs) {
        return KITTY_MEMORY_ALLOCATION_FAILURE; // Memory allocation failed
    }
    mesh->uvs = new_uvs;
    mesh->uvs[mesh->uv_count] = uv;
    mesh->uv_count++;
    return KITTY_SUCCESS; // Success
}

Kitty_Texture* Kitty_LoadTexture(const char* file_path) {
    if (!sdl_renderer) {
        return NULL; // SDL renderer not initialized
    }
    SDL_Surface* surface = IMG_Load(file_path);
    SDL_Surface* optimized_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!surface) {

        return NULL; // Image loading failed
    }
    Kitty_Texture* texture = (Kitty_Texture*)malloc(sizeof(Kitty_Texture));
    texture->sdl_surface = optimized_surface;
    return texture;
}

int Kitty_LoadDotObj(FILE* file, Kitty_Object* mesh) {
    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            Kitty_Vertex3D vertex;
            sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);


            Kitty_AddVertexToObjMesh(mesh, vertex);
        } else if (strncmp(line, "f ", 2) == 0) {
            Kitty_Face face;

            //random color for each face
            Kitty_Color color = {
                .r = rand() % 256,
                .g = rand() % 256,
                .b = rand() % 256,
                .a = 255
            };

            int a, b, c;
            int uv_a, uv_b, uv_c;

            //f 6/18/13 10/19/13 20/20/13
            sscanf(line, "f %d/%d/%*d %d/%d/%*d %d/%d/%*d", &a, &uv_a, &b, &uv_b, &c, &uv_c);


            face.a = a - 1;
            face.b = b - 1;
            face.c = c - 1;
            face.uv_a = uv_a - 1;
            face.uv_b = uv_b - 1;
            face.uv_c = uv_c - 1;

            Kitty_AddFaceToObjMesh(mesh, face, color);
        } else if (strncmp(line, "vt ", 2) == 0) {
            Kitty_UV uv;
            sscanf(line, "vt %f %f", &uv.u, &uv.v);

            //flip UV
            uv.v = 1.0f - uv.v;

            Kitty_AddUVToObjMesh(mesh, uv);
        }
    }
    return 0;
}

size_t Kitty_GetFrameNumber() {
    return frame_num;
}

clock_t Kitty_GetDeltaTime() {
    return clock() - start_time;
}

double Kitty_GetFrameTime() {
    return frame_time;
}

void Kitty_SetTimer1() {
    timer_1 = clock();
}

bool Kitty_Timer1Trip(long milliseconds) {
    clock_t current_time = clock();
    long elapsed_time = (current_time - timer_1) * 1000 / CLOCKS_PER_SEC;
    return elapsed_time >= milliseconds;
}

Kitty_Object* Kitty_CreateCircle(Kitty_Point position, float radius, bool filled, Kitty_Color color) {
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
    circle_data->position = position;
    circle_data->radius = radius;
    circle_data->filled = filled;
    circle_data->color = color;
    return obj;
}

Kitty_Object* Kitty_CreateRectangle(Kitty_Point position, int width, int height, bool filled, Kitty_Color color) {
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
    rect_data->position = position;
    rect_data->width = width;
    rect_data->height = height;
    rect_data->filled = filled;
    rect_data->color = color;
    return obj;
}

Kitty_Object* Kitty_CreateLine(Kitty_Point startPosition, Kitty_Point endPosition, Kitty_Color color) {
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
    line_data->startPoint = startPosition;
    line_data->endPoint = endPosition;
    line_data->color = color;
    return obj;
}

Kitty_Object* Kitty_CreateTriangle(Kitty_Point vertex1, Kitty_Point vertex2, Kitty_Point vertex3, bool filled, Kitty_Color color) {
    Kitty_Object* obj = (Kitty_Object*)malloc(sizeof(Kitty_Object));
    if (!obj) {
        return NULL; // Memory allocation failed
    }
    obj->type = KITTY_OBJECT_TRIANGLE;
    obj->data = malloc(sizeof(Kitty_ObjTriangle));
    if (!obj->data) {
        free(obj);
        return NULL; // Memory allocation failed
    }
    Kitty_ObjTriangle* tri_data = (Kitty_ObjTriangle*)obj->data;
    tri_data->vertex1 = vertex1;
    tri_data->vertex2 = vertex2;
    tri_data->vertex3 = vertex3;
    tri_data->color = color;
    tri_data->filled = filled;
    return obj;
}

Kitty_Object* Kitty_CreatePixel(Kitty_Point position, Kitty_Color color) {
    Kitty_Object* obj = (Kitty_Object*)malloc(sizeof(Kitty_Object));
    if (!obj) {
        return NULL; // Memory allocation failed
    }
    obj->type = KITTY_OBJECT_PIXEL;
    obj->data = malloc(sizeof(Kitty_ObjPixel));
    if (!obj->data) {
        free(obj);
        return NULL; // Memory allocation failed
    }
    Kitty_ObjPixel* pixel_data = (Kitty_ObjPixel*)obj->data;
    pixel_data->position = position;
    pixel_data->color = color;
    return obj;
}

Kitty_Object* Kitty_CreateMesh(){
    Kitty_Object* obj = (Kitty_Object*)malloc(sizeof(Kitty_Object));
    if (!obj) {
        return NULL; // Memory allocation failed
    }
    obj->type = KITTY_OBJECT_MESH;
    obj->data = malloc(sizeof(Kitty_ObjMesh));
    if (!obj->data) {
        free(obj);
        return NULL; // Memory allocation failed
    }
    Kitty_ObjMesh* mesh_data = (Kitty_ObjMesh*)obj->data;
    mesh_data->vertices = NULL;
    mesh_data->faces = NULL;
    mesh_data->face_colors = NULL;
    mesh_data->uvs = NULL;
    mesh_data->scale = 1;
    mesh_data->position = (Kitty_Point3D){0, 0, 0};
    mesh_data->vertex_count = 0;
    mesh_data->face_count = 0;
    mesh_data->uv_count = 0;
    mesh_data->wire = false;
    mesh_data->wrap = true;
    return obj;
}

Kitty_Object* Kitty_CreateText(Kitty_Point position, float size, float rotation, Kitty_Color color, const char* text) {
    Kitty_Object* obj = (Kitty_Object*)malloc(sizeof(Kitty_Object));
    if (!obj) {
        return NULL; // Memory allocation failed
    }
    obj->type = KITTY_OBJECT_TEXT;
    obj->data = malloc(sizeof(Kitty_ObjText));
    if (!obj->data) {
        free(obj);
        return NULL; // Memory allocation failed
    }
    Kitty_ObjText* text_data = (Kitty_ObjText*)obj->data;
    text_data->position = position;
    text_data->size = size;
    text_data->rotation = rotation;
    text_data->color = color;
    text_data->text = strdup(text); // Duplicate the string
    if (!text_data->text) {
        free(text_data);
        free(obj);
        return NULL; // Memory allocation failed
    }
    return obj;
}

int KittyD_DrawMeshUVMap(Kitty_Point position, int scale, Kitty_ObjMesh* mesh){
    if (!sdl_renderer){
        return KITTY_SDL_RENDERER_NOT_INITIALIZED; // SDL renderer not initialized
    }

    // create wireframe of mesh, each vertex offset by position
    for (size_t f = 0; f < mesh->face_count; f++){
        Kitty_Face face = mesh->faces[f];
        Kitty_UV uv1 = mesh->uvs[face.uv_a];
        Kitty_UV uv2 = mesh->uvs[face.uv_b];
        Kitty_UV uv3 = mesh->uvs[face.uv_c];

        int uv1x = position.x + (int)(uv1.u * mesh->texture->sdl_surface->w) * scale;
        int uv1y = position.y + (int)(uv1.v * mesh->texture->sdl_surface->h) * scale;
        int uv2x = position.x + (int)(uv2.u * mesh->texture->sdl_surface->w) * scale;
        int uv2y = position.y + (int)(uv2.v * mesh->texture->sdl_surface->h) * scale;
        int uv3x = position.x + (int)(uv3.u * mesh->texture->sdl_surface->w) * scale;
        int uv3y = position.y + (int)(uv3.v * mesh->texture->sdl_surface->h) * scale;

        //set red color
        SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(sdl_renderer, uv1x, uv1y, uv2x, uv2y);
        SDL_RenderDrawLine(sdl_renderer, uv2x, uv2y, uv3x, uv3y);
        SDL_RenderDrawLine(sdl_renderer, uv3x, uv3y, uv1x, uv1y);
    }

    return KITTY_SUCCESS;
}

int KittyD_DrawTexture(Kitty_Point position, int scale, Kitty_Texture* texture){
    if (!sdl_renderer){
        return KITTY_SDL_RENDERER_NOT_INITIALIZED; // SDL renderer not initialized
    }

    Uint32* pixels = (Uint32*)texture->sdl_surface->pixels;
    for (int y = 0; y < texture->sdl_surface->h; y++){
        for (int x = 0; x < texture->sdl_surface->w; x++){
            //enlarge 4x
            Uint32 color = pixels[y * (texture->sdl_surface->pitch / 4) + x];
            Uint8 r, g, b, a;
            SDL_GetRGBA(color, texture->sdl_surface->format, &r, &g, &b, &a);
            for (int py = 0; py < scale; py++){
                for (int px = 0; px < scale; px++){
                    SDL_SetRenderDrawColor(sdl_renderer, r, g, b, 255);
                    SDL_RenderDrawPoint(sdl_renderer, position.x + x * scale + px, position.y + y * scale + py);
                }
            }
        }
    }

    return KITTY_SUCCESS; // Success
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

    // Loop thru objects and free their data
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