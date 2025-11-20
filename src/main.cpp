#include "raylib.h"
#include "facialInterface.h"

Camera3D initializeCamer();


int main(){

	InitWindow(800, 600, "Facial Interface");
    SetTargetFPS(60);


	Shader shader = LoadShader("res/shader.vert", "res/shader.frag");
	FacialInterface facialInterface = FacialInterface("res/FaceModel.glb", shader, 5);

	Camera3D cam = initializeCamer();


    while (!WindowShouldClose()) {
		facialInterface.Draw(cam);
    }

    CloseWindow();
    return 0;
}

Camera3D initializeCamer(){
    Camera3D cam = { 0 };
    cam.position = { 0.0f, 0.1f, 1.0f };
    cam.target   = { 0.0f, 0.1f, 0.0f };
    cam.up       = { 0.0f, 1.0f, 0.0f };
    cam.fovy     = 45.0f;
    cam.projection = CAMERA_PERSPECTIVE;
	return cam;
}
