#include "net/CommandQueue.h"

namespace forg::net {

void CommandQueue::Push(const Command& cmd)
{
    QueueItem item;
    item.cmd = cmd;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_items.push_back(item);
}

std::future<std::string> CommandQueue::PushWithReply(const Command& cmd)
{
    QueueItem item;
    item.cmd = cmd;
    item.reply = std::make_shared<std::promise<std::string>>();
    std::future<std::string> result = item.reply->get_future();

    std::lock_guard<std::mutex> lock(m_mutex);
    m_items.push_back(item);
    return result;
}

bool CommandQueue::TryPop(QueueItem& out)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_items.empty())
    {
        return false;
    }

    out = m_items.front();
    m_items.pop_front();
    return true;
}

} // namespace forg::net
