#include "fivex/SocketClient.hpp"

namespace FiveX{
    
SocketClient::SocketClient(){
    mInitialized = 0;
    mIsConnected = 0;
    mIsConnectedClient = 0;
}
bool SocketClient::initCommon(bool fastTcp, bool udp){

    if(mInitialized) close();

    nn::nifm::Initialize();
    nn::nifm::SubmitNetworkRequestAndWait();

    mSocket = nn::socket::Socket(2, (udp ? SOCK_DGRAM : SOCK_STREAM), 6);

    mInitialized = (mSocket >= 0);

    if(mInitialized && fastTcp){
        int isNoDelay = 1;
        nn::socket::SetSockOpt(mSocket, 6, TCP_NODELAY, &isNoDelay, sizeof(isNoDelay));
    }
    
    return mInitialized;

}
bool SocketClient::bind(u16 port, bool fastTcp, bool udp){

    if(!initCommon(fastTcp, udp)) return 0;

    sockaddr_in serverAddr;
    serverAddr.sin_addr.s_addr = 0;
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_port        = nn::socket::InetHtons(port);

    mIsConnected = (nn::socket::Bind(mSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == 0);
    
    if(!mIsConnected) close();
    
    return mIsConnected;


}
bool SocketClient::Accept(){

    if(!mIsConnected) return 0;

    nn::socket::Listen(mSocket, 1);

    sockaddr clientAddr;
    u32 clientAddrSize = sizeof(clientAddr);

    mClientSocket = nn::socket::Accept(mSocket, &clientAddr, &clientAddrSize);

    mIsConnectedClient = (mClientSocket >= 0);

    if(!mIsConnectedClient) close();

    return mIsConnectedClient;

}
bool SocketClient::connect(const char *ip, u16 port, bool fastTcp, bool udp){

    if(!initCommon(fastTcp, udp)) return 0;

    sockaddr_in serverAddr;
    nn::socket::InetAton(ip, &serverAddr.sin_addr);
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_port        = nn::socket::InetHtons(port);

    mIsConnected = (nn::socket::Connect(mSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == 0);
    
    if(!mIsConnected) close();
    
    return mIsConnected;

}

void SocketClient::shutdown(int method){

    if(mIsConnectedClient) nn::socket::Shutdown(mClientSocket, method);
    
    if(mInitialized) nn::socket::Shutdown(mSocket, method);

}

void SocketClient::close(){

    shutdown();

    if(mIsConnectedClient){
        nn::socket::Close(mClientSocket);
        mIsConnectedClient = 0;
    }

    if(mInitialized) nn::socket::Close(mSocket);

    mIsConnected = 0;
    mInitialized = 0;
    
}

bool SocketClient::SendMessage(const void *data, size_t size){
    
    if(!mIsConnected) return 0;

    s32 curSock = (mIsConnectedClient ? mClientSocket : mSocket);

    size_t sentSize = 0;
    while(sentSize < size){

        size_t newSize = nn::socket::Send(curSock, reinterpret_cast<const char*>(data) + sentSize, std::min(size_t(1024), size - sentSize), 0);
        if(newSize == 0) break;

        sentSize+=newSize;
        if(sentSize == size) return 1;

    }

    close();

    return 0;
}

bool SocketClient::ReceiveMessage(void *out, size_t size){

    if(!mIsConnected) return 0;

    s32 curSock = (mIsConnectedClient ? mClientSocket : mSocket);

    size_t recvSize = 0;
    while(recvSize < size){

        size_t newSize = nn::socket::Recv(curSock, reinterpret_cast<char*>(out) + recvSize, std::min(size_t(1024), size - recvSize), 0);
        if(newSize == 0) break;

        recvSize+=newSize;
        if(recvSize == size) return 1;
    }

    close();

    return 0;
}

};