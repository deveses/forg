#include <catch2/catch_test_macros.hpp>

#include "forg/audio/AudioEngine.h"

TEST_CASE("AudioEngine owns manager and tolerates repeated lifecycle calls",
          "[audio]")
{
    forg::audio::AudioEngine audio;

    REQUIRE_FALSE(audio.IsInitialized());
    REQUIRE(&audio.Manager() == &audio.Manager());
    REQUIRE(&audio.Manager().Mixer() == &audio.Manager().Mixer());

    audio.Update();
    audio.Shutdown();
    audio.Shutdown();
    REQUIRE_FALSE(audio.IsInitialized());

    const bool initialized = audio.Init();
    REQUIRE(audio.IsInitialized() == initialized);

    audio.Update();
    audio.Shutdown();
    REQUIRE_FALSE(audio.IsInitialized());
}

TEST_CASE("AudioManager exposes a stable mixer", "[audio]")
{
    forg::audio::AudioManager manager;

    REQUIRE_FALSE(manager.IsInitialized());
    REQUIRE(&manager.Mixer() == &manager.Mixer());

    manager.Update();
    manager.Shutdown();
    REQUIRE_FALSE(manager.IsInitialized());
}
