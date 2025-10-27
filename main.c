#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

#include "kittyengine.h"

int main(void){
    int result = Kitty_Init("Kitty Engine Test Window", 800, 600);
    if (result != KITTY_SUCCESS){
        return result; // Return error code
    }

    bool flip = true;

    // Main loop
    bool running = true;
    SDL_Event event;
    while (running) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        if (flip) {
            Kitty_ClearScreen((Kitty_Color){255, 0, 0, 255}); // Clear screen to red
        } else {
            Kitty_ClearScreen((Kitty_Color){0, 0, 255, 255}); // Clear screen to blue
        }

        flip = !flip;

        // Here you would add rendering of objects

        Kitty_Clock(2); // Cap at 60 FPS
        Kitty_FlipBuffers(); // Present the rendered frame
        printf("Flip!\n");
    }

    Kitty_Quit();
    return 0;
}