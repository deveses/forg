#include "forg_pch.h"

#include "core/RefCounter.h"
#include "cpu/atomic.h"

namespace forg { namespace core {

    int RefCounter::AddRef()
    {
        return cpu::AtomicIncrement(&m_refCount);
    }

    int RefCounter::Release()
    {
        int c = cpu::AtomicDecrement(&m_refCount);

        if (c == 0)
        {
            delete this;
        }

	    return c;
    }

}}