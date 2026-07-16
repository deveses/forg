#include <catch2/catch_test_macros.hpp>

#include "forg/core/ObjectPool.h"

#include <cstdint>
#include <utility>

namespace {

struct TrackedObject
{
    TrackedObject(int* constructionCounter, int* destructionCounter,
                  int objectValue)
        : constructions(constructionCounter), destructions(destructionCounter),
          value(objectValue)
    {
        ++(*constructions);
    }

    ~TrackedObject() { ++(*destructions); }

    int* constructions;
    int* destructions;
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

TEST_CASE("ObjectPool reports its initial state", "[core][objectpool]")
{
    forg::core::ObjectPool<int> pool(3);

    REQUIRE(pool.Capacity() == 3);
    REQUIRE(pool.Size() == 0);
    REQUIRE(pool.Available() == 3);
    REQUIRE(pool.Empty());
    REQUIRE_FALSE(pool.Full());
}

TEST_CASE("ObjectPool emplaces objects until full", "[core][objectpool]")
{
    forg::core::ObjectPool<int> pool(2);

    int* first = pool.Emplace(11);
    int* second = pool.Emplace(22);

    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);
    REQUIRE(first != second);
    REQUIRE(*first == 11);
    REQUIRE(*second == 22);
    REQUIRE(pool.Size() == 2);
    REQUIRE(pool.Available() == 0);
    REQUIRE(pool.Full());
    REQUIRE(pool.Emplace(33) == nullptr);
}

TEST_CASE("ObjectPool releases and reuses slots", "[core][objectpool]")
{
    forg::core::ObjectPool<int> pool(2);
    int* first = pool.Emplace(1);
    int* second = pool.Emplace(2);

    REQUIRE(pool.Release(first));
    REQUIRE(pool.Size() == 1);

    int* replacement = pool.Emplace(3);

    REQUIRE(replacement == first);
    REQUIRE(*replacement == 3);
    REQUIRE(*second == 2);
    REQUIRE(pool.Full());
}

TEST_CASE("ObjectPool rejects invalid releases", "[core][objectpool]")
{
    forg::core::ObjectPool<int> pool(1);
    int outside = 7;
    int* object = pool.Emplace(4);

    REQUIRE_FALSE(pool.Release(nullptr));
    REQUIRE_FALSE(pool.Release(&outside));
    REQUIRE(pool.Release(object));
    REQUIRE_FALSE(pool.Release(object));
    REQUIRE(pool.Empty());
}

TEST_CASE("ObjectPool destroys live objects", "[core][objectpool]")
{
    int constructions = 0;
    int destructions = 0;

    {
        forg::core::ObjectPool<TrackedObject> pool(3);
        TrackedObject* first = pool.Emplace(&constructions, &destructions, 10);
        pool.Emplace(&constructions, &destructions, 20);

        REQUIRE(constructions == 2);
        REQUIRE(destructions == 0);

        REQUIRE(pool.Release(first));
        REQUIRE(destructions == 1);

        pool.Clear();
        REQUIRE(destructions == 2);
        REQUIRE(pool.Empty());

        pool.Emplace(&constructions, &destructions, 30);
    }

    REQUIRE(constructions == 3);
    REQUIRE(destructions == 3);
}

TEST_CASE("ObjectPool supports zero capacity", "[core][objectpool]")
{
    forg::core::ObjectPool<int> pool(0);

    REQUIRE(pool.Capacity() == 0);
    REQUIRE(pool.Empty());
    REQUIRE(pool.Full());
    REQUIRE(pool.Emplace(1) == nullptr);
}

TEST_CASE("ObjectPool supports over-aligned objects", "[core][objectpool]")
{
    forg::core::ObjectPool<OverAlignedObject> pool(1);

    OverAlignedObject* object = pool.Emplace(99);
    const auto address = reinterpret_cast<std::uintptr_t>(object);

    REQUIRE(object != nullptr);
    REQUIRE(object->value == 99);
    REQUIRE(address % alignof(OverAlignedObject) == 0);
}

TEST_CASE("ObjectPool move transfers live object ownership",
          "[core][objectpool]")
{
    int constructions = 0;
    int destructions = 0;
    forg::core::ObjectPool<TrackedObject> source(1);
    TrackedObject* object = source.Emplace(&constructions, &destructions, 42);

    forg::core::ObjectPool<TrackedObject> target(std::move(source));

    REQUIRE(source.Capacity() == 0);
    REQUIRE(source.Empty());
    REQUIRE(target.Size() == 1);
    REQUIRE(object->value == 42);
    REQUIRE(target.Release(object));
    REQUIRE(destructions == 1);
}
