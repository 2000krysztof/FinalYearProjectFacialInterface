#include "server.h"
#include <iostream>
#include <libwebsockets.h>

struct PerSocketData {
    EventQueue* queue;
};

static int onMessage(struct lws *wsi, enum lws_callback_reasons reason,
                     void *user, void *in, size_t len)
{
    PerSocketData* data = static_cast<PerSocketData*>(user);

    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED:
            data->queue = (EventQueue*)lws_context_user(lws_get_context(wsi));

            break;

        case LWS_CALLBACK_RECEIVE: {
            std::string message((const char*)in, len);
            data->queue->push(message);
            break;
        }

        default:
            break;
    }

    return 0;
}

static struct lws_protocols protocols[] = {
    {
        "face-protocol",
        onMessage,
        sizeof(PerSocketData),
        1024,
    },
    { NULL, NULL, 0, 0 }
};

Server::Server(int port, EventQueue* eventQueue)
    : port(port), eventQueue(eventQueue) {}

Server::~Server(){
	stop();
}

void Server::run() {
	if (isRunning) return;
	isRunning = true;
	thread = std::thread(&Server::mainLoop, this);
}

void Server::stop(){
	isRunning = false;
	if(thread.joinable()){
		thread.join();
	}
}

void Server::mainLoop() {
    struct lws_context_creation_info info = {0};
    info.port = port;
    info.protocols = protocols;
    info.user = eventQueue;

    struct lws_context *context = lws_create_context(&info);

    while (isRunning) {
        lws_service(context, 0);
    }

    lws_context_destroy(context);
}
