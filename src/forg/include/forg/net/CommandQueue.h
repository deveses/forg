#ifndef _FORG_NET_COMMANDQUEUE_H_
#define _FORG_NET_COMMANDQUEUE_H_

#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <string>

#include "net/Command.h"

namespace forg
{
namespace net
{

/// A queued command plus an optional channel for its reply.
/**
 * When a caller needs a response it uses PushWithReply(), which creates a
 * promise owned by the queue (held via shared_ptr) and hands back the matching
 * future. Queue-owned lifetime is the point: the promise stays alive even if
 * the requester stops waiting (e.g. times out and returns) before the main
 * thread produces the body. `reply` is null for fire-and-forget callers.
 */
struct QueueItem
{
    Command cmd;
    std::shared_ptr<std::promise<std::string>> reply;

    QueueItem() : reply() {}
};

/// Thread-safe hand-off of commands from the server thread to the main thread.
/**
 * The server thread Push()es; the render thread drains with TryPop() once per
 * frame. Only a mutex is needed — the consumer polls, so no condition variable.
 */
class CommandQueue
{
  public:
    /// Queues a fire-and-forget command (no reply channel).
    void Push(const Command& cmd);

    /// Queues a command and returns a future for its reply. The queue owns the
    /// promise, so the future stays valid even if the caller stops waiting.
    std::future<std::string> PushWithReply(const Command& cmd);

    /// Pops the oldest item into `out`. Returns false (leaving out untouched)
    /// when the queue is empty.
    bool TryPop(QueueItem& out);

  private:
    std::mutex m_mutex;
    std::deque<QueueItem> m_items;
};

} // namespace net
} // namespace forg

#endif //_FORG_NET_COMMANDQUEUE_H_
