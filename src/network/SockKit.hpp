/*
 * @Descripttion:
 * @version: 1.0
 * @Author: YangHouQi
 * @Date: 2024-10-22 14:36:25
 * @LastEditors: YangHouQi
 * @LastEditTime: 2024-10-24 17:31:31
 */
#include <iostream>
#include <string>
#include <memory>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
enum UdpModel
{
    um_unicast,
    um_multicast,
    um_broadcast
};
enum TcpModel
{
    tm_server,
    tm_client
};
struct AddrInfo
{
    std::string ip = 0;
    int port = 0;
};
struct TcpSocketInfo
{
    int socketFd;
    int socketAcceptFd;
    std::string connectedIp;
    uint16_t connectedPort;
    bool isConnected;

    TcpSocketInfo(int fd) : socketFd(fd), isConnected(false) {}
};
using RecvCallback = std::function<void(const std::string &buffer, int length, const AddrInfo &addrInfo)>;

class SocketBase
{
public:
    virtual bool bind(const std::string &ip, int port) = 0;
    virtual bool send(const std::string &message, const std::string &destIp, uint16_t destPort) = 0;
    virtual bool recv(RecvCallback recvCallback) = 0;
    virtual ~SocketBase() = default;
};

class SocketStrategyBase
{
public:
    virtual int send(std::shared_ptr<int> socketFd, const std::string &message, const std::string &destIp, const uint16_t &destPort) = 0;
    virtual int recv(std::shared_ptr<int> socketFd, RecvCallback recvCallback, int bufferSize) = 0;
    virtual bool recvSwitch(bool rSwitch) = 0;
    virtual ~SocketStrategyBase() = default;
};
class UDPUnicastStrategy : public SocketStrategyBase
{
public:
    int send(std::shared_ptr<int> socketFd, const std::string &message, const std::string &destIp, const uint16_t &destPort) override;
    int recv(std::shared_ptr<int> socketFd, RecvCallback recvCallback, int bufferSize) override;
    bool recvSwitch(bool rSwitch) override;

    std::string buffer;
    std::unique_ptr<std::thread> recvThread;
    std::atomic<bool> recvFlag;
};
class UDPMulticastStrategy : public UDPUnicastStrategy
{
};
class UDPBroadcastStrategy : public UDPUnicastStrategy
{
};

class UdpSocket : public SocketBase
{
public:
    UdpSocket();
    UdpSocket(const std::string &ip, int port);
    UdpSocket(UdpModel udpModel, const std::string &ip, int port);
    bool send(const std::string &message, const std::string &destIp, uint16_t destPort) override;
    bool recv(RecvCallback recvCallback);
    bool recvSwitch(bool rSwitch);
    bool sockClose();
    ~UdpSocket();

private:
    std::shared_ptr<int> socketFd;
    sockaddr_in serverAddr, clientAddr;
    ip_mreq mreq;
    std::unique_ptr<SocketStrategyBase> socketStrategy;
    bool bind(const std::string &ip, int port) override;
    bool bind(UdpModel udpModel, const std::string &ip, int port);
};
class UdpFactory
{
public:
    static inline std::unique_ptr<UdpSocket> createUdpSocket()
    {
        return std::make_unique<UdpSocket>();
    }

    static inline std::unique_ptr<UdpSocket> createUdpSocket(const std::string &ip, int port)
    {
        return std::make_unique<UdpSocket>(ip, port);
    }

    static inline std::unique_ptr<UdpSocket> createUdpSocket(UdpModel udpModel, const std::string &ip, int port)
    {
        return std::make_unique<UdpSocket>(udpModel, ip, port);
    }
};

class TCPStrategyBase
{
public:
    virtual ~TCPStrategyBase() = default;
    virtual bool connect(TcpSocketInfo &socketInfo) = 0;
    virtual int send(TcpSocketInfo &socketInfo, const std::string &message) = 0;
    virtual int recv(TcpSocketInfo &socketInfo, RecvCallback recvCallback, const int bufferSize) = 0;
    virtual bool close(TcpSocketInfo &socketInfo) = 0;
    virtual bool bind(TcpSocketInfo &socketInfo) = 0;
    virtual bool recvSwitch(bool rSwitch) = 0;
};

class TCPServerStrategy : public TCPStrategyBase
{
public:
    bool connect(TcpSocketInfo &socketInfo) override;
    int send(TcpSocketInfo &socketInfo, const std::string &message) override;
    int recv(TcpSocketInfo &socketInfo, RecvCallback recvCallback, const int bufferSize) override;
    bool close(TcpSocketInfo &socketInfo) override;
    bool bind(TcpSocketInfo &socketInfo) override;
    bool recvSwitch(bool rSwitch) override;
    ~TCPServerStrategy();

private:
    std::string buffer;
    std::unique_ptr<std::thread> recvThread;
    std::atomic<bool> recvFlag;
};

class TCPClientStrategy : public TCPStrategyBase
{
public:
    bool connect(TcpSocketInfo &socketInfo) override;
    int send(TcpSocketInfo &socketInfo, const std::string &message) override;
    int recv(TcpSocketInfo &socketInfo, RecvCallback recvCallback, const int bufferSize) override;
    bool close(TcpSocketInfo &socketInfo) override;
    bool bind(TcpSocketInfo &socketInfo) override;
    bool recvSwitch(bool rSwitch) override;
    ~TCPClientStrategy();

private:
    std::string buffer;
    std::unique_ptr<std::thread> recvThread;
};
class TcpSocket
{
private:
    TcpSocketInfo socketInfo;
    std::unique_ptr<TCPStrategyBase> strategy;

public:
    TcpSocket(TcpModel tcpModel, const std::string &ip, int port);
    bool connect(const std::string &ip, uint16_t port);
    int send(const std::string &message);
    int recv(RecvCallback recvCallback);
    bool recvSwitch(bool rSwitch);
    bool close();
    ~TcpSocket();
};

class TcpFactory
{
public:
    static inline std::unique_ptr<TcpSocket> createTcpServer(const std::string &ip, int port)
    {
        return std::make_unique<TcpSocket>(TcpModel::tm_server, ip, port);
    }
    static inline std::unique_ptr<TcpSocket> createTcpClient(const std::string &ip, int port)
    {
        return std::make_unique<TcpSocket>(TcpModel::tm_client, ip, port);
    }
};
