#include <catch2/catch_test_macros.hpp>

#include "forg/audio/SoundInstanceManager.h"
#include "forg/audio/SoundInstanceProcessorChain.h"

#include <atomic>
#include <memory>
#include <thread>

TEST_CASE("SoundInstanceManager allocates instances from its fixed pool",
          "[audio][sound-instance-manager]")
{
    auto chain = std::make_shared<forg::audio::SoundInstanceProcessorChain>();
    forg::audio::SoundInstanceManager manager(2, chain);

    const forg::audio::SoundInstanceId first = manager.Play();
    const forg::audio::SoundInstanceId second = manager.Play();

    REQUIRE(first != forg::audio::INVALID_SOUND_INSTANCE_ID);
    REQUIRE(second != forg::audio::INVALID_SOUND_INSTANCE_ID);
    REQUIRE(first != second);
    REQUIRE(manager.IsPlaying(first));
    REQUIRE(manager.IsPlaying(second));
    REQUIRE(manager.Size() == 2);
    REQUIRE(manager.Play() == forg::audio::INVALID_SOUND_INSTANCE_ID);

    REQUIRE(manager.Stop(first));
    REQUIRE(manager.IsPlaying(first));
    manager.Update();
    REQUIRE_FALSE(manager.IsPlaying(first));

    const forg::audio::SoundInstanceId replacement = manager.Play();
    REQUIRE(replacement != forg::audio::INVALID_SOUND_INSTANCE_ID);
    REQUIRE(replacement != first);
}

TEST_CASE("SoundInstanceManager queues stops and releases them during update",
          "[audio][sound-instance-manager]")
{
    auto chain = std::make_shared<forg::audio::SoundInstanceProcessorChain>();
    forg::audio::SoundInstanceManager manager(2, chain);

    const forg::audio::SoundInstanceId id = manager.Play();
    REQUIRE(id != forg::audio::INVALID_SOUND_INSTANCE_ID);
    REQUIRE(manager.IsPlaying(id));

    REQUIRE(manager.Stop(id));

    REQUIRE(manager.IsPlaying(id));
    REQUIRE(manager.Size() == 1);

    manager.Update();
    REQUIRE_FALSE(manager.IsPlaying(id));
    REQUIRE(manager.Size() == 0);
}

TEST_CASE("SoundInstanceManager publishes state safely while reclaiming",
          "[audio][sound-instance-manager][threading]")
{
    auto chain = std::make_shared<forg::audio::SoundInstanceProcessorChain>();
    forg::audio::SoundInstanceManager manager(1, chain);
    const forg::audio::SoundInstanceId id = manager.Play();
    REQUIRE(id != forg::audio::INVALID_SOUND_INSTANCE_ID);

    std::atomic<bool> started = false;
    std::atomic<bool> running = true;
    std::atomic<std::size_t> queryCount = 0;
    std::thread observer(
        [&]
        {
            started.store(true, std::memory_order_release);
            while (running.load(std::memory_order_acquire))
            {
                static_cast<void>(manager.IsPlaying(id));
                queryCount.fetch_add(1, std::memory_order_relaxed);
            }
        });

    while (!started.load(std::memory_order_acquire) ||
           queryCount.load(std::memory_order_relaxed) == 0)
    {
    }

    REQUIRE(manager.Stop(id));
    manager.Update();
    running.store(false, std::memory_order_release);
    observer.join();

    REQUIRE(queryCount.load(std::memory_order_relaxed) > 0);
    REQUIRE_FALSE(manager.IsPlaying(id));
    REQUIRE(manager.Size() == 0);
}
