#include "raylib.h"
int main(){
	InitWindow(800, 600, "Facial Interface");

    SetTargetFPS(60);
	Model faceModel = LoadModel("res/FaceModel.glb");
	int animCount = 0;
	ModelAnimation* anims = LoadModelAnimations("res/FaceModel.glb", &animCount);

    Camera3D cam = { 0 };
    cam.position = { 0.0f, 0.1f, 1.0f };
    cam.target   = { 0.0f, 0.1f, 0.0f };
    cam.up       = { 0.0f, 1.0f, 0.0f };
    cam.fovy     = 45.0f;
    cam.projection = CAMERA_PERSPECTIVE;

	Shader shader = LoadShader("res/shader.vert", "res/shader.frag");

	faceModel.materials[0].shader = shader;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
		BeginMode3D(cam);

        DrawModel(faceModel, {0,0,0}, 0.1f, WHITE);

        EndMode3D();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
