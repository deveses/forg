#include "net/HttpControlSocketServer.h"

// base.h (force-included via the PCH) defines IN, OUT and null as macros that
// clash with platform socket headers. Drop them before including system headers.
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif
#ifdef null
#undef null
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <limits>

namespace forg::net {
namespace {

class PosixSocketServer : public PlatformSocketServer
{
  public:
    PosixSocketServer() = default;
    ~PosixSocketServer() override { Stop(); }

    bool Start(const std::string& bindAddr, int port) override
    {
        Stop();

        int listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listenFd < 0)
            return false;

        int yes = 1;
        ::setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<unsigned short>(port));
        if (::inet_pton(AF_INET, bindAddr.c_str(), &addr.sin_addr) != 1)
        {
            ::close(listenFd);
            return false;
        }

        if (::bind(listenFd, reinterpret_cast<sockaddr*>(&addr),
                   sizeof(addr)) < 0 ||
            ::listen(listenFd, 4) < 0)
        {
            ::close(listenFd);
            return false;
        }

        m_listenFd = listenFd;
        return true;
    }

    void Stop() override
    {
        if (m_listenFd >= 0)
        {
            ::shutdown(m_listenFd, SHUT_RDWR);
            ::close(m_listenFd);
            m_listenFd = -1;
        }
    }

    SocketHandle Accept() override
    {
        int client = ::accept(m_listenFd, nullptr, nullptr);
        if (client < 0)
            return kInvalidSocket;
        return static_cast<SocketHandle>(client);
    }

    int Receive(SocketHandle client, char* buffer, size_t bufferSize) override
    {
        return static_cast<int>(
            ::recv(static_cast<int>(client), buffer, bufferSize, 0));
    }

    int Send(SocketHandle client, const char* data, size_t dataSize) override
    {
        const size_t chunk =
            (dataSize > static_cast<size_t>(std::numeric_limits<int>::max()))
                ? static_cast<size_t>(std::numeric_limits<int>::max())
                : dataSize;
        return static_cast<int>(
            ::send(static_cast<int>(client), data, chunk, 0));
    }

    void Close(SocketHandle client) override
    {
        if (client != kInvalidSocket)
            ::close(static_cast<int>(client));
    }

  private:
    int m_listenFd = -1;
};

} // namespace

std::unique_ptr<PlatformSocketServer> CreatePlatformSocketServer()
{
    return std::unique_ptr<PlatformSocketServer>(new PosixSocketServer());
}

} // namespace forg::net
