#ifndef FORG_CORE_REFPTR_H
#define FORG_CORE_REFPTR_H

#include <utility>

namespace forg::core
{

// Move-only ownership for resources returned with an initial reference count.
template <typename T> class RefPtr
{
  public:
    constexpr RefPtr() noexcept = default;
    explicit RefPtr(T* pointer) noexcept : pointer_(pointer) {}

    RefPtr(const RefPtr&) = delete;
    RefPtr& operator=(const RefPtr&) = delete;

    RefPtr(RefPtr&& other) noexcept
        : pointer_(std::exchange(other.pointer_, nullptr))
    {
    }

    RefPtr& operator=(RefPtr&& other) noexcept
    {
        if (this != &other)
            reset(std::exchange(other.pointer_, nullptr));
        return *this;
    }

    ~RefPtr() { reset(); }

    void reset(T* pointer = nullptr) noexcept
    {
        T* old = std::exchange(pointer_, pointer);
        if (old != nullptr)
            old->Release();
    }

    [[nodiscard]] T* get() const noexcept { return pointer_; }
    [[nodiscard]] explicit operator bool() const noexcept
    {
        return pointer_ != nullptr;
    }
    T& operator*() const noexcept { return *pointer_; }
    T* operator->() const noexcept { return pointer_; }

  private:
    T* pointer_ = nullptr;
};

} // namespace forg::core

#endif // FORG_CORE_REFPTR_H
