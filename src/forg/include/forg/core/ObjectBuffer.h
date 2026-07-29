// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "forg/base.h"

#include <cstddef>
#include <new>
#include <utility>

namespace forg::core {

class FORG_API ObjectBuffer
{
  public:
    ObjectBuffer() = default;
    ~ObjectBuffer();

    ObjectBuffer(const ObjectBuffer&) = delete;
    ObjectBuffer& operator=(const ObjectBuffer&) = delete;

    ObjectBuffer(ObjectBuffer&& other) noexcept;
    ObjectBuffer& operator=(ObjectBuffer&& other) noexcept;

    [[nodiscard]] bool Empty() const noexcept;
    [[nodiscard]] std::size_t Size() const noexcept;
    [[nodiscard]] std::size_t Alignment() const noexcept;
    [[nodiscard]] void* Data() noexcept;
    [[nodiscard]] const void* Data() const noexcept;

    void Reset() noexcept;

    template <typename T, typename... Args> T* Emplace(Args&&... args)
    {
        Reset();

        void* storage = Allocate(sizeof(T), alignof(T));
        T* object = new (storage) T(std::forward<Args>(args)...);

        m_data = storage;
        m_object = object;
        m_size = sizeof(T);
        m_alignment = alignof(T);
        m_destructor = &Destroy<T>;

        return object;
    }

    template <typename T> T* Get() noexcept
    {
        return reinterpret_cast<T*>(m_object);
    }

    template <typename T> const T* Get() const noexcept
    {
        return reinterpret_cast<const T*>(m_object);
    }

  private:
    using Destructor = void (*)(void*);

    static void* Allocate(std::size_t size, std::size_t alignment);
    static void Deallocate(void* data, std::size_t alignment) noexcept;

    template <typename T> static void Destroy(void* object)
    {
        static_cast<T*>(object)->~T();
    }

    void* m_data = nullptr;
    void* m_object = nullptr;
    std::size_t m_size = 0;
    std::size_t m_alignment = 0;
    Destructor m_destructor = nullptr;
};

template <std::size_t Capacity,
          std::size_t StorageAlignment = alignof(std::max_align_t)>
class StaticObjectBuffer
{
    static_assert(Capacity > 0);
    static_assert(StorageAlignment > 0 &&
                  (StorageAlignment & (StorageAlignment - 1)) == 0);

  public:
    StaticObjectBuffer() = default;
    ~StaticObjectBuffer() { Reset(); }

    StaticObjectBuffer(const StaticObjectBuffer&) = delete;
    StaticObjectBuffer& operator=(const StaticObjectBuffer&) = delete;
    StaticObjectBuffer(StaticObjectBuffer&&) = delete;
    StaticObjectBuffer& operator=(StaticObjectBuffer&&) = delete;

    [[nodiscard]] bool Empty() const noexcept { return m_object == nullptr; }
    [[nodiscard]] std::size_t Size() const noexcept { return m_size; }
    [[nodiscard]] std::size_t Alignment() const noexcept
    {
        return m_objectAlignment;
    }
    [[nodiscard]] void* Data() noexcept { return m_object; }
    [[nodiscard]] const void* Data() const noexcept { return m_object; }

    void Reset() noexcept
    {
        if (m_destructor != nullptr && m_object != nullptr)
            m_destructor(m_object);

        m_object = nullptr;
        m_size = 0;
        m_objectAlignment = 0;
        m_destructor = nullptr;
    }

    template <typename T, typename... Args> T* Emplace(Args&&... args)
    {
        static_assert(sizeof(T) <= Capacity,
                      "Object exceeds StaticObjectBuffer capacity");
        static_assert(alignof(T) <= StorageAlignment,
                      "Object exceeds StaticObjectBuffer alignment");

        Reset();
        T* object = ::new (static_cast<void*>(m_storage))
            T(std::forward<Args>(args)...);

        m_object = object;
        m_size = sizeof(T);
        m_objectAlignment = alignof(T);
        m_destructor = &Destroy<T>;
        return object;
    }

    template <typename T> T* Get() noexcept
    {
        return reinterpret_cast<T*>(m_object);
    }

    template <typename T> const T* Get() const noexcept
    {
        return reinterpret_cast<const T*>(m_object);
    }

  private:
    using Destructor = void (*)(void*);

    template <typename T> static void Destroy(void* object)
    {
        static_cast<T*>(object)->~T();
    }

    alignas(StorageAlignment) std::byte m_storage[Capacity];
    void* m_object = nullptr;
    std::size_t m_size = 0;
    std::size_t m_objectAlignment = 0;
    Destructor m_destructor = nullptr;
};

} // namespace forg::core
