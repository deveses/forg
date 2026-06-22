#include "net/HttpControlServer.h"
#include "net/HttpRequest.h"

#if !defined(FORG_PLATFORM_WINDOWS)

// base.h (force-included via the PCH) defines IN, OUT and null as macros that
// clash with the BSD socket headers. Drop them before including system headers.
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

#include <chrono>
#include <cstring>
#include <future>
#include <sstream>

namespace forg::net {

HttpControlServer::HttpControlServer(const std::string& bindAddr, int port,
                                     CommandQueue& queue)
    : m_addr(bindAddr), m_port(port), m_queue(queue), m_listenFd(-1),
      m_running(false)
{
}

HttpControlServer::~HttpControlServer() { Stop(); }

bool HttpControlServer::Start()
{
    m_listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0)
    {
        return false;
    }

    int yes = 1;
    ::setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<unsigned short>(m_port));
    addr.sin_addr.s_addr = inet_addr(m_addr.c_str());

    if (::bind(m_listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) <
            0 ||
        ::listen(m_listenFd, 4) < 0)
    {
        ::close(m_listenFd);
        m_listenFd = -1;
        return false;
    }

    m_running = true;
    m_thread = std::thread(&HttpControlServer::Run, this);
    return true;
}

void HttpControlServer::Stop()
{
    m_running = false;

    if (m_listenFd >= 0)
    {
        // shutdown + close unblocks a thread parked in accept().
        ::shutdown(m_listenFd, SHUT_RDWR);
        ::close(m_listenFd);
        m_listenFd = -1;
    }

    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void HttpControlServer::Run()
{
    while (m_running)
    {
        int client = ::accept(m_listenFd, 0, 0);
        if (client < 0)
        {
            if (!m_running)
            {
                break; // socket closed by Stop()
            }
            continue;
        }

        HandleConnection(client);
        ::close(client);
    }
}

void HttpControlServer::HandleConnection(int clientFd)
{
    char buf[8192];
    ssize_t n = ::recv(clientFd, buf, sizeof(buf) - 1, 0);
    if (n <= 0)
    {
        return;
    }

    std::string request(buf, static_cast<size_t>(n));
    size_t eol = request.find("\r\n");
    if (eol == std::string::npos)
    {
        eol = request.find('\n');
    }
    std::string firstLine =
        (eol == std::string::npos) ? request : request.substr(0, eol);

    int status = 200;
    std::string body;

    std::string method, path, query;
    if (ParseRequestLine(firstLine, method, path, query))
    {
        Command cmd = CommandFromRequest(path, query);

        std::future<std::string> result = m_queue.PushWithReply(cmd);

        // The main thread drains the queue and fulfils the promise each frame.
        // The timeout keeps the socket thread from hanging if it has stopped.
        // The queue owns the promise, so timing out here is safe: the main
        // thread's later set_value just lands in a value no one reads.
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

    std::string out = response.str();
    ::send(clientFd, out.data(), out.size(), 0);
}

} // namespace forg::net

#endif // !FORG_PLATFORM_WINDOWS
