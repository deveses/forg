#include <catch2/catch_test_macros.hpp>

#include "forg/core/ObjectBuffer.h"

#include <cstdint>
#include <utility>

namespace {

struct CounterObject
{
    CounterObject(int* constructedCounter, int* destroyedCounter,
                  int objectValue)
        : constructed(constructedCounter), destroyed(destroyedCounter),
          value(objectValue)
    {
        ++(*constructed);
    }

    ~CounterObject() { ++(*destroyed); }

    int* constructed;
    int* destroyed;
    int value;
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4324) // structure was padded due to alignment specifier
#endif
struct alignas(64) OverAlignedObject
{
    explicit OverAlignedObject(int objectValue) : value(objectValue) {}

    int value;
};
#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace

TEST_CASE("ObjectBuffer starts empty", "[core][objectbuffer]")
{
    forg::core::ObjectBuffer buffer;

    REQUIRE(buffer.Empty());
    REQUIRE(buffer.Size() == 0);
    REQUIRE(buffer.Alignment() == 0);
    REQUIRE(buffer.Data() == nullptr);
    REQUIRE(buffer.Get<int>() == nullptr);
}

TEST_CASE("ObjectBuffer emplaces and exposes stored object",
          "[core][objectbuffer]")
{
    forg::core::ObjectBuffer buffer;

    int* value = buffer.Emplace<int>(42);

    REQUIRE_FALSE(buffer.Empty());
    REQUIRE(value != nullptr);
    REQUIRE(*value == 42);
    REQUIRE(buffer.Get<int>() == value);
    REQUIRE(*buffer.Get<int>() == 42);
    REQUIRE(buffer.Size() == sizeof(int));
    REQUIRE(buffer.Alignment() == alignof(int));
    REQUIRE(buffer.Data() == value);
}

TEST_CASE("ObjectBuffer Reset destroys the stored object once",
          "[core][objectbuffer]")
{
    int constructed = 0;
    int destroyed = 0;
    forg::core::ObjectBuffer buffer;

    buffer.Emplace<CounterObject>(&constructed, &destroyed, 7);

    REQUIRE(constructed == 1);
    REQUIRE(destroyed == 0);
    REQUIRE(buffer.Get<CounterObject>()->value == 7);

    buffer.Reset();

    REQUIRE(buffer.Empty());
    REQUIRE(buffer.Data() == nullptr);
    REQUIRE(buffer.Size() == 0);
    REQUIRE(buffer.Alignment() == 0);
    REQUIRE(destroyed == 1);

    buffer.Reset();

    REQUIRE(destroyed == 1);
}

TEST_CASE("ObjectBuffer destructor destroys stored object",
          "[core][objectbuffer]")
{
    int constructed = 0;
    int destroyed = 0;

    {
        forg::core::ObjectBuffer buffer;
        buffer.Emplace<CounterObject>(&constructed, &destroyed, 11);

        REQUIRE(constructed == 1);
        REQUIRE(destroyed == 0);
    }

    REQUIRE(destroyed == 1);
}

TEST_CASE("ObjectBuffer replacing object destroys previous object first",
          "[core][objectbuffer]")
{
    int firstConstructed = 0;
    int firstDestroyed = 0;
    int secondConstructed = 0;
    int secondDestroyed = 0;
    forg::core::ObjectBuffer buffer;

    buffer.Emplace<CounterObject>(&firstConstructed, &firstDestroyed, 1);
    buffer.Emplace<CounterObject>(&secondConstructed, &secondDestroyed, 2);

    REQUIRE(firstConstructed == 1);
    REQUIRE(firstDestroyed == 1);
    REQUIRE(secondConstructed == 1);
    REQUIRE(secondDestroyed == 0);
    REQUIRE(buffer.Get<CounterObject>()->value == 2);

    buffer.Reset();

    REQUIRE(secondDestroyed == 1);
}

TEST_CASE("ObjectBuffer move construction transfers ownership",
          "[core][objectbuffer]")
{
    int constructed = 0;
    int destroyed = 0;
    forg::core::ObjectBuffer source;
    source.Emplace<CounterObject>(&constructed, &destroyed, 5);

    forg::core::ObjectBuffer target(std::move(source));

    REQUIRE(source.Empty());
    REQUIRE(source.Data() == nullptr);
    REQUIRE(target.Get<CounterObject>()->value == 5);

    target.Reset();

    REQUIRE(constructed == 1);
    REQUIRE(destroyed == 1);
}

TEST_CASE("ObjectBuffer move assignment replaces target ownership",
          "[core][objectbuffer]")
{
    int oldConstructed = 0;
    int oldDestroyed = 0;
    int newConstructed = 0;
    int newDestroyed = 0;
    forg::core::ObjectBuffer source;
    forg::core::ObjectBuffer target;

    source.Emplace<CounterObject>(&newConstructed, &newDestroyed, 22);
    target.Emplace<CounterObject>(&oldConstructed, &oldDestroyed, 9);

    target = std::move(source);

    REQUIRE(source.Empty());
    REQUIRE(oldConstructed == 1);
    REQUIRE(oldDestroyed == 1);
    REQUIRE(newConstructed == 1);
    REQUIRE(newDestroyed == 0);
    REQUIRE(target.Get<CounterObject>()->value == 22);

    target.Reset();

    REQUIRE(newDestroyed == 1);
}

TEST_CASE("ObjectBuffer supports over-aligned objects", "[core][objectbuffer]")
{
    forg::core::ObjectBuffer buffer;

    OverAlignedObject* object = buffer.Emplace<OverAlignedObject>(99);
    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(object);

    REQUIRE(object->value == 99);
    REQUIRE(buffer.Size() == sizeof(OverAlignedObject));
    REQUIRE(buffer.Alignment() == alignof(OverAlignedObject));
    REQUIRE(address % alignof(OverAlignedObject) == 0);
}

TEST_CASE("StaticObjectBuffer stores objects in its inline storage",
          "[core][objectbuffer]")
{
    forg::core::StaticObjectBuffer<sizeof(OverAlignedObject),
                                   alignof(OverAlignedObject)>
        buffer;

    OverAlignedObject* object = buffer.Emplace<OverAlignedObject>(77);
    const auto bufferAddress = reinterpret_cast<std::uintptr_t>(&buffer);
    const auto objectAddress = reinterpret_cast<std::uintptr_t>(object);

    REQUIRE(object->value == 77);
    REQUIRE(objectAddress >= bufferAddress);
    REQUIRE(objectAddress < bufferAddress + sizeof(buffer));
    REQUIRE(objectAddress % alignof(OverAlignedObject) == 0);
    REQUIRE(buffer.Data() == object);
}
