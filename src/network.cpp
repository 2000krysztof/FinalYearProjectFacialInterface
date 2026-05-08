#include "network.h"
#include "utils.h"
#include <httplib.h>
#include <json.hpp>
#include <iostream>

void SendToBun(const std::vector<short> audioBuffer, 
               std::queue<std::vector<EmotionSegment>>& timelineQueue, std::mutex& timelineMutex,
               std::queue<std::vector<unsigned char>>& audioQueue, std::mutex& audioMutex) {
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

        std::vector<EmotionSegment> timeline;
        for (auto& ts : data["timestamps"]) {
            EmotionSegment seg;
            seg.emotion = ts["tag"].get<std::string>();
            seg.start   = ts["start"].get<float>();
            seg.end     = ts["end"].get<float>();
            timeline.push_back(seg);
        }
        {
            std::lock_guard<std::mutex> lock(timelineMutex);
            timelineQueue.push(timeline);
        }

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
