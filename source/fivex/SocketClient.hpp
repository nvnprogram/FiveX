#pragma once

#include "types.h"
#include "nn/socket.hpp"
#include "nn/nifm.hpp"
#include "math.h"

namespace FiveX{
    class SocketClient {
        public:

            SocketClient();

            bool initCommon(bool fastTcp = 0, bool udp = 0);

            bool bind(u16 port, bool fastTcp = 0, bool udp = 0);
            bool connect(const char * ip, u16 port, bool fastTcp = 0, bool udp = 0);

            bool Accept();

            bool SendMessage(const void *data, size_t size);
            bool ReceiveMessage(void *out, size_t size);

            void shutdown(int method = 2); // SHUT_RDWR 
            void close();

            s32 mSocket;
            bool mIsConnected;
            bool mInitialized;

            s32 mClientSocket;
            bool mIsConnectedClient;

    };
};