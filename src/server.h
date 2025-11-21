#pragma once
#include <atomic>
#include <thread>
#include "eventQueue.h"

class Server {
public:
    Server(int port, EventQueue* eventQueue);
	~Server();
    void run();
	void stop();

private:
	void mainLoop();
    EventQueue* eventQueue;
	std::thread thread;
	std::atomic<bool> isRunning {false};
	int port;
};
