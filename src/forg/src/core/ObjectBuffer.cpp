// SPDX-License-Identifier: GPL-3.0-or-later

#include "forg_pch.h"

#include "forg/core/ObjectBuffer.h"

namespace forg::core {

ObjectBuffer::~ObjectBuffer() { Reset(); }

ObjectBuffer::ObjectBuffer(ObjectBuffer&& other) noexcept
    : m_data(other.m_data), m_object(other.m_object), m_size(other.m_size),
      m_alignment(other.m_alignment), m_destructor(other.m_destructor)
{
    other.m_data = nullptr;
    other.m_object = nullptr;
    other.m_size = 0;
    other.m_alignment = 0;
    other.m_destructor = nullptr;
}

ObjectBuffer& ObjectBuffer::operator=(ObjectBuffer&& other) noexcept
{
    if (this == &other)
        return *this;

    Reset();

    m_data = other.m_data;
    m_object = other.m_object;
    m_size = other.m_size;
    m_alignment = other.m_alignment;
    m_destructor = other.m_destructor;

    other.m_data = nullptr;
    other.m_object = nullptr;
    other.m_size = 0;
    other.m_alignment = 0;
    other.m_destructor = nullptr;

    return *this;
}

bool ObjectBuffer::Empty() const noexcept { return m_object == nullptr; }

std::size_t ObjectBuffer::Size() const noexcept { return m_size; }

std::size_t ObjectBuffer::Alignment() const noexcept { return m_alignment; }

void* ObjectBuffer::Data() noexcept { return m_data; }

const void* ObjectBuffer::Data() const noexcept { return m_data; }

void ObjectBuffer::Reset() noexcept
{
    if (m_destructor != nullptr && m_object != nullptr)
        m_destructor(m_object);

    Deallocate(m_data, m_alignment);

    m_data = nullptr;
    m_object = nullptr;
    m_size = 0;
    m_alignment = 0;
    m_destructor = nullptr;
}

void* ObjectBuffer::Allocate(std::size_t size, std::size_t alignment)
{
    return ::operator new(size, std::align_val_t(alignment));
}

void ObjectBuffer::Deallocate(void* data, std::size_t alignment) noexcept
{
    if (data == nullptr)
        return;

    ::operator delete(data, std::align_val_t(alignment));
}

} // namespace forg::core
