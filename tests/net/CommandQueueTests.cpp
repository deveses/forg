#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <thread>

#include "forg/net/CommandQueue.h"

using forg::net::Command;
using forg::net::CommandQueue;
using forg::net::QueueItem;

static Command makeCommand(const std::string& verb)
{
    Command cmd;
    cmd.verb = verb;
    return cmd;
}

TEST_CASE("CommandQueue pops items in FIFO order", "[net][queue]")
{
    CommandQueue queue;
    queue.Push(makeCommand("a"));
    queue.Push(makeCommand("b"));
    queue.Push(makeCommand("c"));

    QueueItem item;

    REQUIRE(queue.TryPop(item));
    REQUIRE(item.cmd.verb == "a");
    REQUIRE(queue.TryPop(item));
    REQUIRE(item.cmd.verb == "b");
    REQUIRE(queue.TryPop(item));
    REQUIRE(item.cmd.verb == "c");
}

TEST_CASE("CommandQueue reports empty after draining", "[net][queue]")
{
    CommandQueue queue;
    queue.Push(makeCommand("only"));

    QueueItem item;
    REQUIRE(queue.TryPop(item));

    QueueItem untouched;
    untouched.cmd.verb = "sentinel";
    REQUIRE_FALSE(queue.TryPop(untouched));
    REQUIRE(untouched.cmd.verb == "sentinel"); // left untouched on empty
}

TEST_CASE("CommandQueue round-trips a reply through the future", "[net][queue]")
{
    CommandQueue queue;
    std::future<std::string> result = queue.PushWithReply(makeCommand("state"));

    QueueItem item;
    REQUIRE(queue.TryPop(item));
    REQUIRE(item.reply != nullptr);

    item.reply->set_value("{\"ok\":true}");
    REQUIRE(result.get() == "{\"ok\":true}");
}

TEST_CASE("CommandQueue reply survives the requester giving up", "[net][queue]")
{
    // Mirrors the HTTP timeout path: the requester pushes, then abandons its
    // future and returns. The queue must still own a live promise so the later
    // set_value on the consumer side is not a use-after-free.
    CommandQueue queue;
    {
        std::future<std::string> result = queue.PushWithReply(makeCommand("state"));
        (void)result;
    } // future (and the requester's scope) gone here; the queue owns the promise

    QueueItem item;
    REQUIRE(queue.TryPop(item));
    REQUIRE(item.reply != nullptr);
    REQUIRE_NOTHROW(item.reply->set_value("late"));
}

TEST_CASE("CommandQueue is safe across a producer thread", "[net][queue]")
{
    CommandQueue queue;
    const int total = 1000;

    std::thread producer([&queue]() {
        for (int i = 0; i < total; ++i)
        {
            queue.Push(makeCommand("x"));
        }
    });

    int drained = 0;
    QueueItem item;
    while (drained < total)
    {
        if (queue.TryPop(item))
        {
            ++drained;
        }
    }
    producer.join();

    REQUIRE(drained == total);
    REQUIRE_FALSE(queue.TryPop(item));
}
