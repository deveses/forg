// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace forg::core {

template <typename T> class ObjectPool
{
    static_assert(std::is_object_v<T>);
    static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>);
    static_assert(std::is_nothrow_destructible_v<T>);

  public:
    explicit ObjectPool(std::size_t capacity)
        : m_slots(capacity == 0 ? nullptr : std::make_unique<Slot[]>(capacity)),
          m_capacity(capacity)
    {
        InitializeFreeList();
    }

    ~ObjectPool() { Clear(); }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    ObjectPool(ObjectPool&& other) noexcept
        : m_slots(std::move(other.m_slots)),
          m_capacity(std::exchange(other.m_capacity, 0)),
          m_size(std::exchange(other.m_size, 0)),
          m_freeHead(std::exchange(other.m_freeHead, InvalidIndex))
    {
    }

    ObjectPool& operator=(ObjectPool&& other) noexcept
    {
        if (this == &other)
            return *this;

        Clear();

        m_slots = std::move(other.m_slots);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_size = std::exchange(other.m_size, 0);
        m_freeHead = std::exchange(other.m_freeHead, InvalidIndex);

        return *this;
    }

    [[nodiscard]] std::size_t Capacity() const noexcept { return m_capacity; }

    [[nodiscard]] std::size_t Size() const noexcept { return m_size; }

    [[nodiscard]] std::size_t Available() const noexcept
    {
        return m_capacity - m_size;
    }

    [[nodiscard]] bool Empty() const noexcept { return m_size == 0; }

    [[nodiscard]] bool Full() const noexcept { return m_size == m_capacity; }

    template <typename... Args> T* Emplace(Args&&... args)
    {
        if (m_freeHead == InvalidIndex)
            return nullptr;

        Slot& slot = m_slots[m_freeHead];
        T* object =
            std::construct_at(StorageAt(slot), std::forward<Args>(args)...);

        m_freeHead = slot.next;
        slot.next = InvalidIndex;
        slot.occupied = true;
        ++m_size;

        return object;
    }

    bool Release(T* object) noexcept
    {
        const std::size_t index = IndexOf(object);
        if (index == InvalidIndex || !m_slots[index].occupied)
            return false;

        Slot& slot = m_slots[index];
        std::destroy_at(ObjectAt(slot));
        slot.occupied = false;
        slot.next = m_freeHead;
        m_freeHead = index;
        --m_size;

        return true;
    }

    void Clear() noexcept
    {
        for (std::size_t index = 0; index < m_capacity; ++index)
        {
            Slot& slot = m_slots[index];
            if (slot.occupied)
            {
                std::destroy_at(ObjectAt(slot));
                slot.occupied = false;
            }
        }

        m_size = 0;
        InitializeFreeList();
    }

  private:
    static constexpr std::size_t InvalidIndex = static_cast<std::size_t>(-1);

    struct Slot
    {
        Slot() noexcept {}

        alignas(T) std::byte storage[sizeof(T)];
        std::size_t next = InvalidIndex;
        bool occupied = false;
    };

    static T* StorageAt(Slot& slot) noexcept
    {
        return reinterpret_cast<T*>(slot.storage);
    }

    static T* ObjectAt(Slot& slot) noexcept
    {
        return std::launder(StorageAt(slot));
    }

    void InitializeFreeList() noexcept
    {
        for (std::size_t index = 0; index < m_capacity; ++index)
        {
            m_slots[index].next =
                index + 1 < m_capacity ? index + 1 : InvalidIndex;
        }

        m_freeHead = m_capacity == 0 ? InvalidIndex : 0;
    }

    [[nodiscard]] std::size_t IndexOf(const T* object) const noexcept
    {
        if (object == nullptr || m_capacity == 0)
            return InvalidIndex;

        const auto address = reinterpret_cast<std::uintptr_t>(object);
        const auto first = reinterpret_cast<std::uintptr_t>(m_slots.get());

        if (address < first)
            return InvalidIndex;

        const std::uintptr_t offset = address - first;
        if (offset % sizeof(Slot) != 0)
            return InvalidIndex;

        const std::size_t index = offset / sizeof(Slot);
        return index < m_capacity ? index : InvalidIndex;
    }

    std::unique_ptr<Slot[]> m_slots;
    std::size_t m_capacity = 0;
    std::size_t m_size = 0;
    std::size_t m_freeHead = InvalidIndex;
};

} // namespace forg::core
