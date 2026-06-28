#include "net/HttpControlSocketServer.h"

// base.h (force-included via the PCH) defines IN, OUT and null as macros that
// clash with Winsock headers. Drop them before including system headers.
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif
#ifdef null
#undef null
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstring>
#include <limits>

namespace forg::net {
namespace {

class WinsockServer : public PlatformSocketServer
{
  public:
    WinsockServer() = default;
    ~WinsockServer() override { Stop(); }

    bool Start(const std::string& bindAddr, int port) override
    {
        Stop();

        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
            return false;
        m_wsaStarted = true;

        SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET)
        {
            Stop();
            return false;
        }

        int yes = 1;
        ::setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char*>(&yes), sizeof(yes));

        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<unsigned short>(port));
        if (::inet_pton(AF_INET, bindAddr.c_str(), &addr.sin_addr) != 1)
        {
            ::closesocket(listenSocket);
            Stop();
            return false;
        }

        if (::bind(listenSocket, reinterpret_cast<sockaddr*>(&addr),
                   sizeof(addr)) == SOCKET_ERROR ||
            ::listen(listenSocket, 4) == SOCKET_ERROR)
        {
            ::closesocket(listenSocket);
            Stop();
            return false;
        }

        m_listenSocket = listenSocket;
        return true;
    }

    void Stop() override
    {
        if (m_listenSocket != INVALID_SOCKET)
        {
            ::shutdown(m_listenSocket, SD_BOTH);
            ::closesocket(m_listenSocket);
            m_listenSocket = INVALID_SOCKET;
        }

        if (m_wsaStarted)
        {
            WSACleanup();
            m_wsaStarted = false;
        }
    }

    SocketHandle Accept() override
    {
        SOCKET client = ::accept(m_listenSocket, nullptr, nullptr);
        if (client == INVALID_SOCKET)
            return kInvalidSocket;
        return static_cast<SocketHandle>(client);
    }

    int Receive(SocketHandle client, char* buffer, size_t bufferSize) override
    {
        return ::recv(static_cast<SOCKET>(client), buffer,
                      static_cast<int>(bufferSize), 0);
    }

    int Send(SocketHandle client, const char* data, size_t dataSize) override
    {
        const size_t chunk =
            (dataSize > static_cast<size_t>(std::numeric_limits<int>::max()))
                ? static_cast<size_t>(std::numeric_limits<int>::max())
                : dataSize;
        return ::send(static_cast<SOCKET>(client), data,
                      static_cast<int>(chunk), 0);
    }

    void Close(SocketHandle client) override
    {
        if (client != kInvalidSocket)
            ::closesocket(static_cast<SOCKET>(client));
    }

  private:
    SOCKET m_listenSocket = INVALID_SOCKET;
    bool m_wsaStarted = false;
};

} // namespace

std::unique_ptr<PlatformSocketServer> CreatePlatformSocketServer()
{
    return std::unique_ptr<PlatformSocketServer>(new WinsockServer());
}

} // namespace forg::net
