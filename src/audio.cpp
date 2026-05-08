#include "audio.h"

std::vector<short> recordingBuffer;
bool isRecording = false;
std::mutex bufferMutex;

void CaptureCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
    if (!isRecording) return;
    const short* samples = (const short*)input;
    std::lock_guard<std::mutex> lock(bufferMutex);
    for (ma_uint32 i = 0; i < frameCount; i++) {
        recordingBuffer.push_back(samples[i]);
    }
}
