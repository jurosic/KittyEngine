#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


#include "kittyengine.h"

int test_init(){
    int result = Kitty_Init("Kitty Engine Test", 800, 600);
    if (result != KITTY_SUCCESS){
        printf("Kitty_Init failed with error code: %d\n", result);
        return 1;
    }
    printf("Kitty_Init succeeded.\n");
    Kitty_Quit();
    return 0;
}

int test_circle_creation(){
    Kitty_Object* circle = Kitty_CreateCircle(400, 300, 50.0f, true, (Kitty_Color){255, 0, 0, 255});
    if (!circle){
        printf("Kitty_CreateCircle failed.\n");
        return 1;
    }
    printf("Kitty_CreateCircle succeeded.\n");
    free(circle->data);
    free(circle);
    return 0;
}

int test_rectangle_creation(){
    Kitty_Object* rectangle = Kitty_CreateRectangle(100, 100, 200, 150, false, (Kitty_Color){0, 255, 0, 255});
    if (!rectangle){
        printf("Kitty_CreateRectangle failed.\n");
        return 1;
    }
    printf("Kitty_CreateRectangle succeeded.\n");
    free(rectangle->data);
    free(rectangle);
    return 0;
}

int test_line_creation(){
    Kitty_Object* line = Kitty_CreateLine(50, 50, 300, 300, (Kitty_Color){0, 0, 255, 255});
    if (!line){
        printf("Kitty_CreateLine failed.\n");
        return 1;
    }
    printf("Kitty_CreateLine succeeded.\n");
    free(line->data);
    free(line);
    return 0;
}

int test_rendering(){
    int result = Kitty_Init("Kitty Engine Rendering Test", 800, 600);
    if (result != KITTY_SUCCESS){
        printf("Kitty_Init failed with error code: %d\n", result);
        return 1;
    }

    Kitty_Color clearColor = {0, 0, 0, 255};
    Kitty_ClearScreen(clearColor);

    Kitty_Object* circle = Kitty_CreateCircle(400, 300, 50.0f, true, (Kitty_Color){255, 0, 0, 255});
    Kitty_Object* rectangle = Kitty_CreateRectangle(100, 100, 200, 150, false, (Kitty_Color){0, 255, 0, 255});
    Kitty_Object* line = Kitty_CreateLine(50, 50, 300, 300, (Kitty_Color){0, 0, 255, 255});

    if ((result = Kitty_AddObject(*circle))) {
        printf("Kitty_AddObject (circle) failed with error code: %d\n", result);
        Kitty_Quit();
        return 1;
    }

    if ((result = Kitty_AddObject(*rectangle))) {
        printf("Kitty_AddObject (rectangle) failed with error code: %d\n", result);
        Kitty_Quit();
        return 1;
    }

    if ((result = Kitty_AddObject(*line))) {
        printf("Kitty_AddObject (line) failed with error code: %d\n", result);
        Kitty_Quit();
        return 1;
    }

    if ((result = Kitty_RenderObjects())){
        printf("Kitty_RenderObjects failed with error code: %d\n", result);
        Kitty_Quit();
        return 1;
    }
    if ((result = Kitty_FlipBuffers())){
        printf("Kitty_FlipBuffers failed with error code: %d\n", result);
        Kitty_Quit();
        return 1;
    }

    SDL_Delay(1000);

    if ((result = Kitty_Quit())) {
        printf("Kitty_Quit failed with error code: %d\n", result);
        return 1;
    }
    return 0;
}

int test_render_multiple() {
        int result = Kitty_Init("Kitty Engine Rendering Test", 800, 600);
    if (result != KITTY_SUCCESS){
        printf("Kitty_Init failed with error code: %d\n", result);
        return 1;
    }

    Kitty_Color clearColor = {0, 0, 0, 255};

    for (int i = 0; i < 10; i++){
        for (int j = 0; j < 5; j++){
            Kitty_ClearScreen(clearColor);

            //three objects at random places with rand();
            Kitty_Object* circle = Kitty_CreateCircle(rand() % 800, rand() % 600, 50.0f, true, (Kitty_Color){255, 0, 0, 255});
            Kitty_Object* rectangle = Kitty_CreateRectangle(rand() % 600, rand() % 450, 200, 150, false, (Kitty_Color){0, 255, 0, 255});
            Kitty_Object* line = Kitty_CreateLine(rand() % 800, rand() % 600, rand() % 800, rand() % 600, (Kitty_Color){0, 0, 255, 255});

            if ((result = Kitty_AddObject(*circle))) {
                printf("Kitty_AddObject (circle) failed with error code: %d\n", result);
                Kitty_Quit();
                return 1;
            }

            if ((result = Kitty_AddObject(*rectangle))) {
                printf("Kitty_AddObject (rectangle) failed with error code: %d\n", result);
                Kitty_Quit();
                return 1;
            }

            if ((result = Kitty_AddObject(*line))) {
                printf("Kitty_AddObject (line) failed with error code: %d\n", result);
                Kitty_Quit();
                return 1;
            }

            if ((result = Kitty_RenderObjects())){
                printf("Kitty_RenderObjects failed with error code: %d\n", result);
                Kitty_Quit();
                return 1;
            }
            if ((result = Kitty_FlipBuffers())){
                printf("Kitty_FlipBuffers failed with error code: %d\n", result);
                Kitty_Quit();
                return 1;
            }

            Kitty_Clock(5);
        }

        if ((result = Kitty_ClearObjects())){
            printf("Kitty_ClearObjects failed with error code: %d\n", result);
            Kitty_Quit();
            return 1;
        }
    }

    if ((result = Kitty_Quit())) {
        printf("Kitty_Quit failed with error code: %d\n", result);
        return 1;
    }
    return 0;

}

int test_memory_free(){
    int result = Kitty_Init("Kitty Engine Memory Test", 800, 600);
    if (result != KITTY_SUCCESS){
        printf("Kitty_Init failed with error code: %d\n", result);
        return 1;
    }

    for (int i = 0; i < 100; i++){
        Kitty_Object* circle = Kitty_CreateCircle(400, 300, 50.0f, true, (Kitty_Color){255, 0, 0, 255});
        if (!circle){
            printf("Kitty_CreateCircle failed during memory test.\n");
            Kitty_Quit();
            return 1;
        }
        if ((result = Kitty_AddObject(*circle))) {
            printf("Kitty_AddObject failed with error code: %d\n", result);
            free(circle->data);
            free(circle);
            Kitty_Quit();
            return 1;
        }
        free(circle->data);
        free(circle);
    }

    for (int i = 0; i < 100; i++){
        if ((result = Kitty_RemoveObject(0))) {
            printf("Kitty_RemoveObject failed with error code: %d\n", result);
            Kitty_Quit();
            return 1;
        }
    }

    result = Kitty_Quit();
    if (result != KITTY_SUCCESS){
        printf("Kitty_Quit failed with error code: %d\n", result);
        return 1;
    }

    printf("Memory management test passed successfully.\n");
    return 0;
}

int test_memory_stress_1000(){
    int result = Kitty_Init("Kitty Engine Memory Stress Test", 800, 600);
    if (result != KITTY_SUCCESS){
        printf("Kitty_Init failed with error code: %d\n", result);
        return 1;
    }

    for (int i = 0; i < 1000; i++){
        Kitty_Object* circle = Kitty_CreateCircle(400, 300, 50.0f, true, (Kitty_Color){255, 0, 0, 255});
        if (!circle){
            printf("Kitty_CreateCircle failed during memory stress test.\n");
            Kitty_Quit();
            return 1;
        }
        if ((result = Kitty_AddObject(*circle))) {
            printf("Kitty_AddObject failed with error code: %d\n", result);
            free(circle->data);
            free(circle);
            Kitty_Quit();
            return 1;
        }
        free(circle->data);
        free(circle);
    }

    for (int i = 0; i < 1000; i++){
        if ((result = Kitty_RemoveObject(0))) {
            printf("Kitty_RemoveObject failed with error code: %d\n", result);
            Kitty_Quit();
            return 1;
        }
    }

    result = Kitty_Quit();
    if (result != KITTY_SUCCESS){
        printf("Kitty_Quit failed with error code: %d\n", result);
        return 1;
    }

    printf("Memory stress test passed successfully.\n");
    return 0;
}

int test_memory_stress_100000(){ 
    int result = Kitty_Init("Kitty Engine Memory Stress Test", 800, 600);
    if (result != KITTY_SUCCESS){
        printf("Kitty_Init failed with error code: %d\n", result);
        return 1;
    }

    for (int i = 0; i < 100000; i++){
        Kitty_Object* circle = Kitty_CreateCircle(400, 300, 50.0f, true, (Kitty_Color){255, 0, 0, 255});
        if (!circle){
            printf("Kitty_CreateCircle failed during memory stress test.\n");
            Kitty_Quit();
            return 1;
        }
        if ((result = Kitty_AddObject(*circle))) {
            printf("Kitty_AddObject failed with error code: %d\n", result);
            free(circle->data);
            free(circle);
            Kitty_Quit();
            return 1;
        }
        free(circle->data);
        free(circle);
    }

    for (int i = 0; i < 100000; i++){
        if ((result = Kitty_RemoveObject(0))) {
            printf("Kitty_RemoveObject failed with error code: %d\n", result);
            Kitty_Quit();
            return 1;
        }
    }

    result = Kitty_Quit();
    if (result != KITTY_SUCCESS){
        printf("Kitty_Quit failed with error code: %d\n", result);
        return 1;
    }

    printf("Memory stress test passed successfully.\n");
    return 0;
}

int main(void){
    unsigned int failed = 0;

    failed += test_init();
    failed += test_circle_creation();
    failed += test_rectangle_creation();
    failed += test_line_creation();
    failed += test_rendering();
    failed += test_render_multiple();
    failed += test_memory_free();
    failed += test_memory_stress_1000();
    failed += test_memory_stress_100000();

    if (failed){
        printf("%u tests failed.\n", failed);
    } else {
        printf("All tests passed successfully.\n");
    }

    return 0;
}