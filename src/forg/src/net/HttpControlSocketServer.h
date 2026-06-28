#ifndef FORG_NET_HTTPCONTROLSOCKETSERVER_H
#define FORG_NET_HTTPCONTROLSOCKETSERVER_H

#include <cstdint>
#include <limits>
#include <memory>
#include <string>

namespace forg::net {

using SocketHandle = std::uintptr_t;
constexpr SocketHandle kInvalidSocket =
    std::numeric_limits<SocketHandle>::max();

class PlatformSocketServer
{
  public:
    virtual ~PlatformSocketServer() = default;

    virtual bool Start(const std::string& bindAddr, int port) = 0;
    virtual void Stop() = 0;
    virtual SocketHandle Accept() = 0;
    virtual int Receive(SocketHandle client, char* buffer,
                        size_t bufferSize) = 0;
    virtual int Send(SocketHandle client, const char* data,
                     size_t dataSize) = 0;
    virtual void Close(SocketHandle client) = 0;
};

std::unique_ptr<PlatformSocketServer> CreatePlatformSocketServer();

} // namespace forg::net

#endif // FORG_NET_HTTPCONTROLSOCKETSERVER_H
