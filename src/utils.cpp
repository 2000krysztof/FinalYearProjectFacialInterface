#include "utils.h"
#include <iostream>
#include <cmath>

using json = nlohmann::json;

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_decode(const std::string& encoded) {
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

void TrimSilence(std::vector<short>& buffer, short threshold) {
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
