#include "raylib.h"
#include <iostream>

Camera3D initializeCamer(){
    Camera3D cam = { 0 };
    cam.position = { 0.0f, 0.1f, 1.0f };
    cam.target   = { 0.0f, 0.1f, 0.0f };
    cam.up       = { 0.0f, 1.0f, 0.0f };
    cam.fovy     = 45.0f;
    cam.projection = CAMERA_PERSPECTIVE;
	return cam;
}

int main(){
	InitWindow(800, 600, "Facial Interface");

    SetTargetFPS(60);
	Model faceModel = LoadModel("res/FaceModel.glb");
	int animCount = 5;
	ModelAnimation* anims = LoadModelAnimations("res/FaceModel.glb", &animCount);

	Camera3D cam = initializeCamer();

	Shader shader = LoadShader("res/shader.vert", "res/shader.frag");
	faceModel.materials[0].shader = shader;
	int animIndex = 0;

	bool hasJustClicked = false;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
		BeginMode3D(cam);

		if(IsMouseButtonDown(0)){
			if(!hasJustClicked){
				std::cout<< "clicked" << std::endl;
				hasJustClicked = true;
				animIndex ++;
			}
		}else{
			hasJustClicked = false;
		}
        DrawModel(faceModel, {0,0,0}, 0.1f, WHITE);
		UpdateModelAnimation(faceModel, anims[0], animIndex);
        EndMode3D();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
