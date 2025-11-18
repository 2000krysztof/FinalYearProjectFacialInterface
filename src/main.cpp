#include "raylib.h"
#include <iostream>
int main(){
	InitWindow(800, 600, "Facial Interface");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);


        EndDrawing();
    }

    CloseWindow();
    return 0;
}
