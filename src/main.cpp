#include "raylib.h"
#include "miniaudio.h" 
#include "facialInterface.h"
#include <iostream>
#include <threads.h>
#include <json.hpp>
#include <httplib.h>
#include <queue>

using json = nlohmann::json;

Camera3D initializeCamer();

std::vector<float> JsonToVector(const std::string& jsonStr, const std::vector<std::string>& keys);

std::vector<short> recordingBuffer;
bool isRecording = false;
std::mutex bufferMutex;

std::queue<std::vector<unsigned char>> audioQueue;
std::mutex audioMutex;

void CaptureCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
    if (!isRecording) return;
    const short* samples = (const short*)input;
    std::lock_guard<std::mutex> lock(bufferMutex);
    for (ma_uint32 i = 0; i < frameCount; i++) {
        recordingBuffer.push_back(samples[i]);
    }
}

void TrimSilence(std::vector<short>& buffer, short threshold = 800) {
    auto start = buffer.begin();
    while (start != buffer.end() && std::abs(*start) < threshold) {
        start++;
    }

    auto end = buffer.end();
    while (end != start && std::abs(*(end - 1)) < threshold) {
        end--;
    }

    if (start != buffer.begin()) buffer.erase(buffer.begin(), start);
    if (end != buffer.end()) buffer.erase(end, buffer.end());
}

std::string VectorToBytes(const std::vector<short>& buffer) {
    return std::string(reinterpret_cast<const char*>(buffer.data()), 
                       buffer.size() * sizeof(short));
}
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline std::string base64_decode(const std::string& encoded) {
    std::string decoded;
    int i = 0;
    unsigned char char_array_4[4], char_array_3[3];
    int in_len = encoded.size();
    int idx = 0;

    while (in_len-- && encoded[idx] != '=' && 
           (isalnum(encoded[idx]) || encoded[idx] == '+' || encoded[idx] == '/')) {
        char_array_4[i++] = encoded[idx++];
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            for (i = 0; i < 3; i++) decoded += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 4; j++) char_array_4[j] = 0;
        for (int j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        for (int j = 0; j < i - 1; j++) decoded += char_array_3[j];
    }

    return decoded;
}

void SendToBun(const std::vector<short> audioBuffer) {
    std::string rawData(reinterpret_cast<const char*>(audioBuffer.data()), 
                        audioBuffer.size() * sizeof(short));

    httplib::Client cli("http://178.104.153.198:30080");
	cli.set_connection_timeout(30);
	cli.set_read_timeout(60);
	cli.set_write_timeout(60);

    httplib::UploadFormData item;
    item.name = "audio";
    item.content = rawData;
    item.filename = "input.pcm";
    item.content_type = "application/octet-stream";

	std::vector<httplib::UploadFormData> items;
    items.push_back(item);
	std::cout << "Sending " << rawData.size() << " bytes to server..." << std::endl;
	if (auto res = cli.Post("/process-voice", items)) {
		auto data = nlohmann::json::parse(res->body);

		std::string base64Audio = data["audio"];
		std::string audioBytes = base64_decode(base64Audio);
		std::vector<unsigned char> audioData(audioBytes.begin(), audioBytes.end());
		std::cout << "Pushing audio bytes: " << audioData.size() << std::endl;
		{
			std::lock_guard<std::mutex> lock(audioMutex);
			audioQueue.push(audioData);
			std::cout << "Queue size after push: " << audioQueue.size() << std::endl;
		}
	} else {
		std::cerr << "Connection error: " << httplib::to_string(res.error()) << std::endl;
	}
}

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
		if (IsKeyPressed(KEY_R)) {
			isRecording = !isRecording;

			if (isRecording) {
				recordingBuffer.clear();
				std::cout << "Recording started..." << std::endl;
			} else {
				if (!recordingBuffer.empty()) {
					std::vector<short> clonedBuffer = recordingBuffer;

					std::cout << "Sending clone: " << clonedBuffer.size() << " samples." << std::endl;

					std::thread t(SendToBun, std::move(clonedBuffer));
					t.detach();

					recordingBuffer.clear();
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
