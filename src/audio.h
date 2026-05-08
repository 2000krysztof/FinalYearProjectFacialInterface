#pragma once
#include "miniaudio.h"
#include <vector>
#include <mutex>

extern std::vector<short> recordingBuffer;
extern bool isRecording;
extern std::mutex bufferMutex;

void CaptureCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount);
