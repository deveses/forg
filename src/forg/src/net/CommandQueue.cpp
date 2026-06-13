#include "net/CommandQueue.h"

namespace forg { namespace net {

void CommandQueue::Push(const Command& cmd, std::promise<std::string>* reply)
{
    QueueItem item;
    item.cmd = cmd;
    item.reply = reply;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_items.push_back(item);
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

}}
