#include "raylib.h"
#include "miniaudio.h" 
#include "facialInterface.h"
#include "audio.h"
#include "network.h"
#include "utils.h"
#include <iostream>
#include <thread>
#include <json.hpp>
#include <httplib.h>
#include <queue>
#include <mutex>

using json = nlohmann::json;

std::queue<std::vector<unsigned char>> audioQueue;
std::mutex audioMutex;
std::queue<std::vector<EmotionSegment>> timelineQueue;
std::mutex timelineMutex;

int main(){

	InitWindow(800, 600, "Facial Interface");
    SetTargetFPS(60);
	SetTraceLogLevel(LOG_NONE);
	InitAudioDevice();

	ma_device_config config = ma_device_config_init(ma_device_type_capture);
	config.capture.format   = ma_format_s16;
	config.capture.channels = 1;
	config.sampleRate       = 44100;
	config.dataCallback     = CaptureCallback;

	ma_device micDevice;
	if (ma_device_init(NULL, &config, &micDevice) != MA_SUCCESS) {
		std::cerr << "Failed to init mic" << std::endl;
		return -1;
	}
	ma_device_start(&micDevice);

	Shader shader = LoadShader("res/shader.vert", "res/shader.frag");
	FacialInterface facialInterface = FacialInterface("res/FaceModel.glb", shader, 5);

	Camera3D cam = initializeCamer();

	std::string out;
	std::vector<std::string> keys = {"happy", "angry", "sad", "surprised"};

    while (!WindowShouldClose()) {
		{
			std::lock_guard<std::mutex> lock(audioMutex);
			while (!audioQueue.empty()) {
				auto audioData = audioQueue.front();
				audioQueue.pop();
				Wave wave = LoadWaveFromMemory(".mp3", audioData.data(), (int)audioData.size());
				Sound sound = LoadSoundFromWave(wave);
				UnloadWave(wave);
				PlaySound(sound);
				std::cout << "Playing sound!" << std::endl;
			}
		}
		{
			std::lock_guard<std::mutex> lock(timelineMutex);
			if (!timelineQueue.empty() && !facialInterface.IsPlaying()) {
				facialInterface.SetTimeline(timelineQueue.front());
				timelineQueue.pop();
			}
		}
		if (IsKeyPressed(KEY_R)) {
			isRecording = !isRecording;

			if (isRecording) {
				recordingBuffer.clear();
				std::cout << "Recording started..." << std::endl;
			} else {
				std::vector<short> clonedBuffer;
				{
					std::lock_guard<std::mutex> lock(bufferMutex);
					clonedBuffer = std::move(recordingBuffer);
					recordingBuffer.clear();
				}

				if (!clonedBuffer.empty()) {
					std::cout << "Sending clone: " << clonedBuffer.size() << " samples." << std::endl;
					std::thread t(SendToBun, std::move(clonedBuffer), std::ref(timelineQueue), std::ref(timelineMutex), std::ref(audioQueue), std::ref(audioMutex));
					t.detach();
				}
			}
		}

		facialInterface.Update();
		facialInterface.Draw(cam);
    }
	ma_device_stop(&micDevice);
	ma_device_uninit(&micDevice);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
