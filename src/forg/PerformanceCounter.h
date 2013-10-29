/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef _FORG_PERFORMANCE_COUNTER_H_
#define _FORG_PERFORMANCE_COUNTER_H_

#if _MSC_VER > 1000
#pragma once
#endif

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
    //Nested
    public:

    //'structors
    public:
        PerformanceCounter();
        ~PerformanceCounter();

    //Attributes
    protected:
        uint64 m_iFrequency;    //need to be quadword aligned

        uint64 m_iDuration;
        uint64 m_iStart;

        bool   m_bStarted;

    //Public Operators
    public:

    //Public Methods
    public:
        int Start();
        int Stop();
        int Pause();
        int Resume();

        int GetDuration(uint64& duration);
        int GetDuration(double& duration);
        int GetDurationInMs(uint64& duration);
        int GetDurationInMs(double& duration);

    //Helpers
    private:
        uint64 GetTime();
        uint64 GetFrequency();
    };



}

#endif  //_FORG_PERFORMANCE_COUNTER_H_

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
return (double)(m_liStop.QuadPart-m_liStart.QuadPart-m_llCorrection)*1000000.0 / m_llFrequency;
}
*/

