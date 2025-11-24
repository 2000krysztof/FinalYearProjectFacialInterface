#include "eventQueue.h"
#include "raylib.h"
#include "facialInterface.h"
#include "server.h"
#include <iostream>
#include <libwebsockets.h>
#include <threads.h>
#include <json.hpp>

using json = nlohmann::json;

Camera3D initializeCamer();

std::vector<float> JsonToVector(const std::string& jsonStr, const std::vector<std::string>& keys);

int main(){
	EventQueue eventQueue = EventQueue();
	Server server = Server(8080, &eventQueue);
	server.run();

	InitWindow(800, 600, "Facial Interface");
    SetTargetFPS(60);
	SetTraceLogLevel(LOG_NONE);

	Shader shader = LoadShader("res/shader.vert", "res/shader.frag");
	FacialInterface facialInterface = FacialInterface("res/FaceModel.glb", shader, 5);

	Camera3D cam = initializeCamer();

	std::string out;
	std::vector<std::string> keys = {"happy", "angry", "sad", "surprised"};

    while (!WindowShouldClose()) {
		if(eventQueue.pop(out)){
			std::vector<float> weights = JsonToVector(out, keys);
			facialInterface.UpdateWeights(weights);
		}
		facialInterface.Update();
		facialInterface.Draw(cam);
    }

	server.stop();
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

std::vector<float> JsonToVector(const std::string& jsonStr, const std::vector<std::string>& keys)
{
    std::vector<float> v;
    try {
        auto j = json::parse(jsonStr);

        for (const auto& key : keys) {
            if (j.contains(key)) {
                v.push_back(j[key].get<float>());
            } else {
                v.push_back(0.0f);
            }
        }
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }

    return v;
}
