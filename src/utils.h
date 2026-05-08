#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "raylib.h"

std::vector<float> JsonToVector(const std::string& jsonStr, const std::vector<std::string>& keys);
std::string base64_decode(const std::string& encoded);
void TrimSilence(std::vector<short>& buffer, short threshold = 800);
std::string VectorToBytes(const std::vector<short>& buffer);
Camera3D initializeCamer();
