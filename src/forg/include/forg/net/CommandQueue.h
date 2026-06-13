#ifndef _FORG_NET_COMMANDQUEUE_H_
#define _FORG_NET_COMMANDQUEUE_H_

#include <deque>
#include <mutex>
#include <future>
#include <string>

#include "net/Command.h"

namespace forg { namespace net {

/// A queued command plus an optional channel for its reply.
/**
* The server thread fills `reply` (a promise it owns) so it can wait for the
* main thread to produce a response body. `reply` may be null for callers that
* do not need a response.
*/
struct QueueItem
{
    Command cmd;
    std::promise<std::string>* reply;

    QueueItem() : reply(0) {}
};

/// Thread-safe hand-off of commands from the server thread to the main thread.
/**
* The server thread Push()es; the render thread drains with TryPop() once per
* frame. Only a mutex is needed — the consumer polls, so no condition variable.
*/
class CommandQueue
{
public:
    void Push(const Command& cmd, std::promise<std::string>* reply = 0);

    /// Pops the oldest item into `out`. Returns false (leaving out untouched)
    /// when the queue is empty.
    bool TryPop(QueueItem& out);

private:
    std::mutex m_mutex;
    std::deque<QueueItem> m_items;
};

}}

#endif //_FORG_NET_COMMANDQUEUE_H_
