#ifndef _FORG_NET_HTTPCONTROLSERVER_H_
#define _FORG_NET_HTTPCONTROLSERVER_H_

#include <atomic>
#include <string>
#include <thread>

#include "net/CommandQueue.h"

namespace forg::net {

/// A tiny HTTP/1.1 control server backed by a CommandQueue.
/**
 * Runs a blocking accept loop on a background thread. Each request is parsed
 * into a Command, handed to the queue with a reply promise, and the thread
 * waits (with a timeout) for the main thread to produce the response body.
 * The server never touches application state directly — it only speaks to the
 * queue, so it knows nothing about cameras, meshes, or rendering.
 *
 * POSIX sockets only (macOS/Linux); the source is excluded from Windows builds.
 */
class HttpControlServer
{
  public:
    HttpControlServer(const std::string& bindAddr, int port,
                      CommandQueue& queue);
    ~HttpControlServer();

    /// Binds and listens, then spawns the accept thread. Returns false on
    /// failure.
    bool Start();

    /// Stops the accept thread and closes the socket. Safe to call more than
    /// once.
    void Stop();

  private:
    void Run();
    void HandleConnection(int clientFd);

    std::string m_addr;
    int m_port;
    CommandQueue& m_queue;
    int m_listenFd;
    std::atomic<bool> m_running;
    std::thread m_thread;
};

} // namespace forg::net

#endif //_FORG_NET_HTTPCONTROLSERVER_H_
