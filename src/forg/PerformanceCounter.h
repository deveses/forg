// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once

#include "base.h"

namespace forg {

// Brief description
/**
 * Detailed description
 * @author eses
 * @version 1.0
 * @date 04-2006
 * @todo
 * @bug
 * @warning
 */
class FORG_API PerformanceCounter
{
    // Nested
  public:
    //'structors
  public:
    PerformanceCounter();
    ~PerformanceCounter();

    // Attributes
  protected:
    uint64 m_iFrequency; // need to be quadword aligned

    uint64 m_iDuration;
    uint64 m_iStart;

    bool m_bStarted;

    // Public Operators
  public:
    // Public Methods
  public:
    int Start();
    int Stop();
    int Pause();
    int Resume();

    int GetDuration(uint64& duration);
    int GetDuration(double& duration);
    int GetDurationInMs(uint64& duration);
    int GetDurationInMs(double& duration);
    int GetDurationInUs(uint64& duration);
    int GetDurationInUs(double& duration);

    // Helpers
  private:
    uint64 GetTime();
    uint64 GetFrequency();
};

} // namespace forg

/*
class CDuration
{
protected:
LARGE_INTEGER m_liStart;
LARGE_INTEGER m_liStop;

LONGLONG m_llFrequency;
LONGLONG m_llCorrection;

public:
CDuration(void);

void Start(void);
void Stop(void);
double GetDuration(void) const;
};

inline CDuration::CDuration(void)
{
LARGE_INTEGER liFrequency;

QueryPerformanceFrequency(&liFrequency);
m_llFrequency = liFrequency.QuadPart;

// Calibration
Start();
Stop();

m_llCorrection = m_liStop.QuadPart-m_liStart.QuadPart;
}

inline void CDuration::Start(void)
{
// Ensure we will not be interrupted by any other thread for a while
Sleep(0);
QueryPerformanceCounter(&m_liStart);
}

inline void CDuration::Stop(void)
{
QueryPerformanceCounter(&m_liStop);
}

inline double CDuration::GetDuration(void) const
{
return (double)(m_liStop.QuadPart-m_liStart.QuadPart-m_llCorrection)*1000000.0 /
m_llFrequency;
}
*/
