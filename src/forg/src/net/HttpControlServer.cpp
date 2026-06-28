#include "net/HttpControlServer.h"
#include "net/HttpRequest.h"

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

#if defined(FORG_PLATFORM_WINDOWS)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <future>
#include <limits>
#include <memory>
#include <sstream>
#include <thread>

namespace forg::net {
namespace {

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
    virtual int Send(SocketHandle client, const char* data, size_t dataSize) = 0;
    virtual void Close(SocketHandle client) = 0;
};

#if defined(FORG_PLATFORM_WINDOWS)

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

std::unique_ptr<PlatformSocketServer> CreatePlatformSocketServer()
{
    return std::unique_ptr<PlatformSocketServer>(new WinsockServer());
}

#else

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

std::unique_ptr<PlatformSocketServer> CreatePlatformSocketServer()
{
    return std::unique_ptr<PlatformSocketServer>(new PosixSocketServer());
}

#endif

bool SendAll(PlatformSocketServer& sockets, SocketHandle client,
             const std::string& out)
{
    size_t sent = 0;
    while (sent < out.size())
    {
        int n = sockets.Send(client, out.data() + sent, out.size() - sent);
        if (n <= 0)
            return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

} // namespace

struct HttpControlServer::Impl
{
    Impl(const std::string& bindAddr, int port, CommandQueue& queue)
        : addr(bindAddr), port(port), queue(queue),
          sockets(CreatePlatformSocketServer())
    {
    }

    ~Impl() { Stop(); }

    bool Start()
    {
        if (running.load())
            return true;

        if (!sockets->Start(addr, port))
            return false;

        running = true;
        thread = std::thread(&Impl::Run, this);
        return true;
    }

    void Stop()
    {
        running = false;
        sockets->Stop();

        if (thread.joinable())
            thread.join();
    }

    void Run()
    {
        while (running.load())
        {
            SocketHandle client = sockets->Accept();
            if (client == kInvalidSocket)
            {
                if (!running.load())
                    break; // socket closed by Stop()
                continue;
            }

            HandleConnection(client);
            sockets->Close(client);
        }
    }

    void HandleConnection(SocketHandle client)
    {
        char buf[8192];
        int n = sockets->Receive(client, buf, sizeof(buf) - 1);
        if (n <= 0)
            return;

        std::string request(buf, static_cast<size_t>(n));
        size_t eol = request.find("\r\n");
        if (eol == std::string::npos)
            eol = request.find('\n');
        std::string firstLine =
            (eol == std::string::npos) ? request : request.substr(0, eol);

        int status = 200;
        std::string body;

        std::string method, path, query;
        if (ParseRequestLine(firstLine, method, path, query))
        {
            Command cmd = CommandFromRequest(path, query);

            std::future<std::string> result = queue.PushWithReply(cmd);

            // The main thread drains the queue and fulfils the promise each
            // frame. The timeout keeps the socket thread from hanging if the
            // engine has stopped. The queue owns the promise, so timing out
            // here is safe: a later set_value just lands in a value no one
            // reads.
            if (result.wait_for(std::chrono::seconds(1)) ==
                std::future_status::ready)
            {
                body = result.get();
            }
            else
            {
                status = 503;
                body = "{\"ok\":false,\"error\":\"timeout\"}";
            }
        }
        else
        {
            status = 400;
            body = "{\"ok\":false,\"error\":\"badrequest\"}";
        }

        const char* reason = (status == 200)   ? "OK"
                             : (status == 400) ? "Bad Request"
                                               : "Service Unavailable";

        std::ostringstream response;
        response << "HTTP/1.1 " << status << ' ' << reason << "\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Content-Length: " << body.size() << "\r\n"
                 << "Access-Control-Allow-Origin: *\r\n"
                 << "Connection: close\r\n"
                 << "\r\n"
                 << body;

        SendAll(*sockets, client, response.str());
    }

    std::string addr;
    int port;
    CommandQueue& queue;
    std::unique_ptr<PlatformSocketServer> sockets;
    std::atomic<bool> running{false};
    std::thread thread;
};

HttpControlServer::HttpControlServer(const std::string& bindAddr, int port,
                                     CommandQueue& queue)
    : m_impl(new Impl(bindAddr, port, queue))
{
}

HttpControlServer::~HttpControlServer() = default;

bool HttpControlServer::Start() { return m_impl->Start(); }

void HttpControlServer::Stop() { m_impl->Stop(); }

} // namespace forg::net
