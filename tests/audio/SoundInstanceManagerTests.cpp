#include <catch2/catch_test_macros.hpp>

#include "forg/audio/SoundInstanceManager.h"
#include "forg/audio/SoundInstanceProcessorChain.h"

#include <memory>

TEST_CASE("SoundInstanceManager allocates instances from its fixed pool",
          "[audio][sound-instance-manager]")
{
    forg::audio::SoundInstanceManager manager(2);
    auto chain = std::make_shared<forg::audio::SoundInstanceProcessorChain>();

    forg::audio::ProcessedSoundInstance* first = manager.Create(chain);
    forg::audio::ProcessedSoundInstance* second = manager.Create(chain);

    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);
    REQUIRE(first != second);
    REQUIRE(first->Id() != second->Id());
    REQUIRE(manager.Size() == 2);
    REQUIRE(manager.Create(chain) == nullptr);

    const forg::audio::SoundInstanceId firstId = first->Id();
    REQUIRE(manager.Find(firstId) == first);
    REQUIRE(manager.Destroy(first));
    REQUIRE(manager.Find(firstId) == nullptr);

    forg::audio::ProcessedSoundInstance* replacement = manager.Create(chain);
    REQUIRE(replacement == first);
    REQUIRE(replacement->Id() != firstId);
}

TEST_CASE("SoundInstanceManager queues stops and releases them during update",
          "[audio][sound-instance-manager]")
{
    forg::audio::SoundInstanceManager manager(2);
    auto chain = std::make_shared<forg::audio::SoundInstanceProcessorChain>();

    forg::audio::ProcessedSoundInstance* instance = manager.Create(chain);
    REQUIRE(instance != nullptr);
    REQUIRE(instance->Play());

    const forg::audio::SoundInstanceId id = instance->Id();
    REQUIRE(manager.Stop(id));

    REQUIRE(manager.Find(id) == instance);
    REQUIRE(manager.Size() == 1);
    REQUIRE(instance->State() == forg::audio::SoundInstanceState::Playing);

    manager.Update();
    REQUIRE(manager.Find(id) == nullptr);
    REQUIRE(manager.Size() == 0);
}
