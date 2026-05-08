#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include "facialInterface.h"

void SendToBun(const std::vector<short> audioBuffer, 
               std::queue<std::vector<EmotionSegment>>& timelineQueue, std::mutex& timelineMutex,
               std::queue<std::vector<unsigned char>>& audioQueue, std::mutex& audioMutex);
