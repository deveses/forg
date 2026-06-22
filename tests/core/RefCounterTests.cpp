#include "forg/core/RefCounter.h"
#include "forg/core/RefPtr.h"

#include <catch2/catch_test_macros.hpp>

namespace {

class TestRefCounter : public forg::core::RefCounter
{
  public:
    explicit TestRefCounter(bool* destroyed = nullptr) : destroyed_(destroyed)
    {
    }
    ~TestRefCounter() override
    {
        if (destroyed_ != nullptr)
            *destroyed_ = true;
    }

  private:
    bool* destroyed_;
};

} // namespace

TEST_CASE("RefCounter reports the updated reference count", "[core][refcount]")
{
    auto* counter = new TestRefCounter;

    REQUIRE(counter->AddRef() == 2);
    REQUIRE(counter->Release() == 1);
    REQUIRE(counter->Release() == 0);
}

TEST_CASE("RefPtr releases adopted resources", "[core][refcount]")
{
    bool destroyed = false;
    {
        forg::core::RefPtr<TestRefCounter> pointer(
            new TestRefCounter(&destroyed));
        REQUIRE(pointer);
    }
    REQUIRE(destroyed);
}
