/*
 * @Descripttion:
 * @version: 1.0
 * @Author: YangHouQi
 * @Date: 2024-10-17 09:03:35
 * @LastEditors: YangHouQi
 * @LastEditTime: 2024-10-25 10:53:11
 */
#include "SockKit.hpp"

UdpSocket::UdpSocket()
{
    this->socketStrategy = std::make_unique<UDPUnicastStrategy>(); // 默认单播策略
    bind("", 0);
};

UdpSocket::UdpSocket(const std::string &ip, int port)
{
    this->socketStrategy = std::make_unique<UDPUnicastStrategy>(); // 默认单播策略
    bind(ip, port);
};

bool UdpSocket::bind(const std::string &ip, int port)
{
    this->socketFd = std::make_shared<int>(socket(AF_INET, SOCK_DGRAM, 0));
    if (*this->socketFd < 0)
    {
        std::cerr << "创建套接字失败，errno: " << errno << std::endl; // 输出错误号
        return false;
    }
    memset(&this->serverAddr, 0, sizeof(serverAddr));
    this->serverAddr.sin_family = AF_INET;
    this->serverAddr.sin_port = port < 0 || port > 65535 ? htons(0) : htons(port);
    this->serverAddr.sin_addr.s_addr = ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());
    this->clientAddr = {};
    if (::bind(*socketFd, (struct sockaddr *)&this->serverAddr, sizeof(this->serverAddr)) < 0)
    {
        std::cerr << "绑定套接字失败，errno: " << errno << " - " << strerror(errno) << std::endl;
        return false;
    }
    return true; // 返回成功
};

bool UdpSocket::bind(UdpModel udpModel, const std::string &ip, int port)
{
    this->socketFd = std::make_shared<int>(socket(AF_INET, SOCK_DGRAM, 0));
    if (*this->socketFd < 0)
    {
        std::cerr << "创建套接字失败，errno: " << errno << std::endl; // 输出错误号
        close(*this->socketFd);
        return false;
    }
    memset(&this->serverAddr, 0, sizeof(serverAddr));
    this->serverAddr.sin_family = AF_INET;
    this->serverAddr.sin_port = port < 0 || port > 65535 ? htons(0) : htons(port);
    this->serverAddr.sin_addr.s_addr = INADDR_ANY;
    this->clientAddr = {};
    if (::bind(*this->socketFd, (struct sockaddr *)&this->serverAddr, sizeof(this->serverAddr)) < 0)
    {
        std::cerr << "绑定套接字失败，errno: " << errno << " - " << strerror(errno) << std::endl;
        close(*this->socketFd);
        return false;
    }
    // 组播逻辑
    if (udpModel == UdpModel::um_multicast)
    {
        this->mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str()); // 设置组播ip
        this->mreq.imr_interface.s_addr = INADDR_ANY;
        if (setsockopt(*this->socketFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) // 加入组播组
        {
            std::cerr << "加入组播组失败，errno: " << errno << " - " << strerror(errno) << std::endl;
            close(*this->socketFd);
            return false;
        }

        unsigned char ttl = 1;
        if (setsockopt(*this->socketFd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
        {
            std::cerr << "设置TTL失败，errno: " << errno << " - " << strerror(errno) << std::endl;
            close(*this->socketFd);
            return false;
        }
    }
    else if (udpModel == UdpModel::um_broadcast)
    {
        int opt = 1;
        setsockopt(*this->socketFd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)); // 启用SO_BROADCAST允许广播
    }
    return true; // 返回成功
};

UdpSocket::UdpSocket(UdpModel udpModel, const std::string &ip, int port)
{
    switch (udpModel)
    {
    case UdpModel::um_unicast:
        this->socketStrategy = std::make_unique<UDPUnicastStrategy>(); // 组播策略
        bind(ip, port);
        break;
    case UdpModel::um_multicast:
        this->socketStrategy = std::make_unique<UDPMulticastStrategy>(); // 组播策略
        bind(UdpModel::um_multicast, ip, port);
        break;
    case UdpModel::um_broadcast:
        this->socketStrategy = std::make_unique<UDPBroadcastStrategy>(); // 组播策略
        bind(ip, port);
        break;
    }
};

bool UdpSocket::send(const std::string &message, const std::string &destIp, uint16_t destPort)
{
    if (socketStrategy == nullptr)
        return false;
    else if (message.length() <= 0 || message.length() > 4096)
        return false;
    else if (destIp.length() <= 0 || destIp.length() > 16)
        return false;
    else if (destPort <= 0 || destPort > 65535)
        return false;
    auto sendRes = this->socketStrategy->send(this->socketFd, message, destIp, destPort);
    return sendRes <= 0 ? false : true;
};

bool UdpSocket::recv(RecvCallback recvCallback)
{
    return this->socketStrategy->recv(this->socketFd, recvCallback, 4096);
};

bool UdpSocket::recvSwitch(bool rSwitch)
{
    return this->socketStrategy->recvSwitch(rSwitch);
};

bool UdpSocket::sockClose()
{
    close(*this->socketFd);
};

UdpSocket::~UdpSocket()
{
    close(*this->socketFd);
};

int UDPUnicastStrategy::send(std::shared_ptr<int> socketFd, const std::string &message, const std::string &destIp, const uint16_t &destPort)
{
    struct sockaddr_in clientAddr = {};
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(destPort);
    clientAddr.sin_addr.s_addr = inet_addr(destIp.c_str());
    auto sendByte = sendto(*socketFd, message.c_str(), message.length(), 0, (const struct sockaddr *)&clientAddr, sizeof(clientAddr));
    std::cout << sendByte << std::endl;
    return sendByte;
};

int UDPUnicastStrategy::recv(std::shared_ptr<int> socketFd, RecvCallback recvCallback, int bufferSize)
{
    this->recvFlag = true;
    socklen_t len = sizeof(sockaddr_in);
    sockaddr_in clientAddr;
    this->buffer.resize(bufferSize);
    std::cout << "准备进行 recvfrom 操作，socketFd: " << *socketFd << std::endl;
    recvThread = std::make_unique<std::thread>([socketFd, this, recvCallback, &bufferSize]()
                                               {
        while (this->recvFlag)
        {   
            if (*socketFd < 0)
            {
                std::cerr << "socketFd 无效: " << *socketFd << std::endl;
                return; 
            }
            sockaddr_in clientAddr; // 转为string ip int port
            socklen_t len = sizeof(clientAddr);
            ssize_t recv_len = recvfrom(*socketFd, (void *)this->buffer.data(), bufferSize, 0, (struct sockaddr *)&clientAddr, &len);
            if (recv_len < 0) {
                std::cerr << "接收错误, errno: " << errno << " - " << strerror(errno) << std::endl;
                continue;
            }
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, sizeof(ipStr));
            int port = ntohs(clientAddr.sin_port);
            recvCallback(this->buffer, recv_len, {ipStr,port});
        } });

    recvThread->detach();
    return 0;
};
bool UDPUnicastStrategy::recvSwitch(bool rSwitch)
{
    this->recvFlag = rSwitch;
    if (rSwitch && this->recvThread && this->recvThread->joinable())
    {
        this->recvThread->join(); // 等待线程退出
    }
};

bool TCPServerStrategy::connect(TcpSocketInfo &socketInfo)
{
    std::cerr << "Server does not need to connect." << std::endl;
    return false;
}

int TCPServerStrategy::send(TcpSocketInfo &socketInfo, const std::string &message)
{
    return socketInfo.socketAcceptFd > 0 ? ::send(socketInfo.socketAcceptFd, message.c_str(), message.size(), 0) : false;
}

int TCPServerStrategy::recv(TcpSocketInfo &socketInfo, RecvCallback recvCallback, const int bufferSize)
{
    this->buffer.resize(bufferSize);
    this->recvFlag = true;
    this->recvThread = std::make_unique<std::thread>([this, &socketInfo, recvCallback, bufferSize]()
                                                     {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        socketInfo.socketAcceptFd = accept(socketInfo.socketFd, (sockaddr *)&clientAddr, &clientLen);
        if (socketInfo.socketAcceptFd < 0)
        {
            std::cerr << "Accept 失败!" << std::endl;
            return;
        }
        while (recvFlag)
        {
            int bytesReceived = ::recv(socketInfo.socketAcceptFd , &this->buffer[0], bufferSize, 0);
            if (bytesReceived > 0)
            {
                recvCallback(this->buffer, bytesReceived, {"", 0});
            }
            else if (bytesReceived == 0 || (bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
            {
                break;
            }
        } });
    this->recvThread->detach();
    return 0;
}

bool TCPServerStrategy::close(TcpSocketInfo &socketInfo)
{
    this->recvFlag = false;
    return ::shutdown(socketInfo.socketFd, SHUT_RDWR) == 0;
}

bool TCPServerStrategy::bind(TcpSocketInfo &socketInfo)
{
    socketInfo.socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketInfo.socketFd < 0)
    {
        std::cerr << "TCP 套接字创建失败, errno: " << errno << " - " << strerror(errno) << std::endl;
        return false;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(socketInfo.connectedPort);
    serverAddr.sin_addr.s_addr = socketInfo.connectedIp.empty() ? INADDR_ANY : inet_addr(socketInfo.connectedIp.c_str());

    if (::bind(socketInfo.socketFd, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Bind failed!" << std::endl;
        return false;
    }

    if (listen(socketInfo.socketFd, 20) < 0)
    {
        std::cerr << "Listen failed!" << std::endl;
        return false;
    }

    std::cout << "Server started and listening on " << socketInfo.connectedIp << ":" << socketInfo.connectedPort << std::endl;
    return true;
}

bool TCPServerStrategy::recvSwitch(bool rSwitch)
{
    return this->recvFlag = rSwitch;
}

TCPServerStrategy::~TCPServerStrategy()
{
    if (this->recvThread && this->recvThread->joinable())
    {
        this->recvThread->join();
    }
}
TcpSocket::TcpSocket(TcpModel tcpModel, const std::string &ip, int port) : socketInfo(-1)
{
    socketInfo.connectedIp = ip;
    socketInfo.connectedPort = port;
    switch (tcpModel)
    {
    case TcpModel::tm_server:
        this->strategy = std::make_unique<TCPServerStrategy>();
        this->strategy->bind(socketInfo);
        break;
    case TcpModel::tm_client:
        this->strategy = std::make_unique<TCPClientStrategy>();
        this->strategy->connect(socketInfo);
        break;
    }
}

bool TcpSocket::connect(const std::string &ip, uint16_t port)
{
    socketInfo.connectedIp = ip;
    socketInfo.connectedPort = port;
    return this->strategy->connect(socketInfo);
}

int TcpSocket::send(const std::string &message)
{
    return strategy ? strategy->send(socketInfo, message) : -1;
}

int TcpSocket::recv(RecvCallback recvCallback)
{
    return strategy ? strategy->recv(this->socketInfo, recvCallback, 4096) : false;
}

bool TcpSocket::close()
{
    return strategy ? strategy->close(socketInfo) : false;
}

bool TcpSocket::recvSwitch(bool rSwitch)
{
    return strategy ? strategy->recvSwitch(rSwitch) : false;
}

TcpSocket::~TcpSocket()
{
    ::close(socketInfo.socketFd);
}

bool TCPClientStrategy::connect(TcpSocketInfo &socketInfo)
{
    if (socketInfo.socketFd > 0)
        ::close(socketInfo.socketFd);
    socketInfo.socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketInfo.socketFd == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(socketInfo.connectedPort);
    serverAddr.sin_addr.s_addr = socketInfo.connectedIp.empty() ? INADDR_ANY : inet_addr(socketInfo.connectedIp.c_str());
    if (::connect(socketInfo.socketFd, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "Failed to connect to server" << std::endl;
        ::close(socketInfo.socketFd);
        return false;
    }
    return true;
}

int TCPClientStrategy::send(TcpSocketInfo &socketInfo, const std::string &message)
{
    return ::send(socketInfo.socketFd, message.c_str(), message.size(), 0);
}

int TCPClientStrategy::recv(TcpSocketInfo &socketInfo, RecvCallback recvCallback, const int bufferSize)
{
    this->buffer.resize(bufferSize);
    this->recvThread = std::make_unique<std::thread>([this, &socketInfo, recvCallback, &bufferSize]()
                                                     {
        auto recvBytes = -1;                      
        recvBytes = ::recv(socketInfo.socketFd,&this->buffer[0], bufferSize, 0);
        if (recvBytes < 0)
        {
            std::cerr << "接收失败" << std::endl;
        }
        recvCallback(buffer,recvBytes,{socketInfo.connectedIp,socketInfo.connectedPort} );
        this->buffer.clear(); });
    this->recvThread->detach();
    return 0;
}

bool TCPClientStrategy::close(TcpSocketInfo &socketInfo)
{
    ::shutdown(socketInfo.socketFd, SHUT_RDWR);
    ::close(socketInfo.socketFd);
    return true;
}

bool TCPClientStrategy::bind(TcpSocketInfo &socketInfo)
{
    std::cout << "客户端无需监听与绑定" << std::endl;
    return false;
}

bool TCPClientStrategy::recvSwitch(bool rSwitch)
{
    return false;
}

TCPClientStrategy::~TCPClientStrategy()
{
    if (this->recvThread->joinable())
    {
        this->recvThread->join();
    }
}