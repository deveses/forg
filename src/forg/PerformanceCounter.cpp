#include "forg_pch.h"

#include "PerformanceCounter.h"
#include "debug/dbg.h"

/******************************************************************************/

#ifdef WIN32
#include <Windows.h>
#else
typedef union _LARGE_INTEGER {
  struct {
    uint LowPart;
    uint HighPart;
  };
  struct {
    uint LowPart;
    uint HighPart;
  } u;
  uint64 QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

#define BOOL uint

uint GetLastError() { return 0; }

BOOL QueryPerformanceCounter(LARGE_INTEGER* li)
{
    return 0;
}

BOOL QueryPerformanceFrequency(LARGE_INTEGER* li)
{
    return 0;
}

#endif

/******************************************************************************/

namespace forg {

    PerformanceCounter::PerformanceCounter()
        : m_bStarted(false)
    {
        m_iFrequency = GetFrequency();
    }

    PerformanceCounter::~PerformanceCounter()
    {
    }

    int PerformanceCounter::Start()
    {
        m_iDuration = 0;
        m_iStart = GetTime();
        m_bStarted = true;

        return RESULT_NO_ERRORS;
    }

    int PerformanceCounter::Stop()
    {
        uint64 end = GetTime();
        m_iDuration += (end - m_iStart);
        m_bStarted = false;

        return RESULT_NO_ERRORS;
    }

    int PerformanceCounter::Pause()
    {
        uint64 end = GetTime();
        m_iDuration += (end - m_iStart);
        m_bStarted = false;

        return RESULT_NO_ERRORS;
    }

    int PerformanceCounter::Resume()
    {
        m_iStart = GetTime();
        m_bStarted = true;

        return RESULT_NO_ERRORS;
    }

    int PerformanceCounter::GetDuration(uint64& duration)
    {
        duration = m_iDuration;

        if (m_bStarted)
        {
            duration += GetTime() - m_iStart;
        }

        return RESULT_NO_ERRORS;
    }

    int PerformanceCounter::GetDuration(double& duration)
    {
        duration = (double)m_iDuration;

        if (m_bStarted)
        {
            duration += GetTime() - m_iStart;
        }

        return RESULT_NO_ERRORS;
    }

    int PerformanceCounter::GetDurationInMs(uint64& duration)
    {
        uint64 t = m_iDuration;

        if (m_bStarted)
        {
            t += GetTime() - m_iStart;
        }

        duration = (t * 1000 / m_iFrequency);

        return RESULT_NO_ERRORS;
    }

    int PerformanceCounter::GetDurationInMs(double& duration)
    {
        uint64 t = m_iDuration;

        if (m_bStarted)
        {
            t += GetTime() - m_iStart;
        }

        duration = ((double)t * 1000.0/ m_iFrequency);

        return RESULT_NO_ERRORS;
    }

    //==========================================================================
    // Platform specific
    //==========================================================================

    uint64 PerformanceCounter::GetTime()
    {
        uint64 t;

        QueryPerformanceCounter((LARGE_INTEGER *)&t);

        return t;
    }

    uint64 PerformanceCounter::GetFrequency()
    {
        uint64 freq = 0;

        BOOL bResult = QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

        if (bResult == 0)
        {
            DBG_MSG("WARNING: QueryPerformanceFrequency failed with error code: 0x%x\n", GetLastError());
        } else
        {
            //DBG_MSG("[CPerformanceCounter] Timer resolution: %.8fns\n", 1000000.0 / (double)m_iFrequency);
        }

        return freq;
    }

}

