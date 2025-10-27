#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

#include "kittyengine.h"

int main(void){
    int result = Kitty_Init("Kitty Engine Test Window", 800, 600);
    if (result != KITTY_SUCCESS){
        return result; // Return error code
    }

    Kitty_Object *rect = Kitty_CreateRectangle(100, 100, 200, 150, true, (Kitty_Color){255, 0, 0, 255});
    Kitty_Object *line = Kitty_CreateLine(50, 50, 300, 300, (Kitty_Color){0, 255, 0, 255});
    Kitty_Object *circle = Kitty_CreateCircle(400, 300, 75.0f, false, (Kitty_Color){0, 0, 255, 255});

    Kitty_AddObject(*rect);
    Kitty_AddObject(*line);
    Kitty_AddObject(*circle);


    bool running = true;
    SDL_Event event;
    while (running) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        Kitty_RenderObjects();

        Kitty_Clock(60); // Cap at 60 FPS
        Kitty_FlipBuffers(); // Present the rendered frame
    }

    Kitty_Quit();
    return 0;
}