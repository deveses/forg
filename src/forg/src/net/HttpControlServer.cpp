#include "net/HttpControlServer.h"
#include "net/HttpRequest.h"
#include "net/HttpControlSocketServer.h"

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <sstream>
#include <thread>

namespace forg::net {
namespace {

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
