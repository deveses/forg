// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "base.h"

namespace forg::cpu {

union UProcessorSignature
{
    struct SProcessorSignature
    {
        uint32 SteppingID : 4;
        uint32 ModelNumber : 4;
        uint32 FamilyCode : 4;
        uint32 Type : 2;
        uint32 Reserved1 : 2;
        uint32 ExtendedModel : 4;
        uint32 ExtendedFamily : 8;
        uint32 Reserved2 : 4;
    } sig;

    uint32 value;
};

union UFeatureFlagsECX
{
    struct SFeatureFlags
    {
        uint32 SSE3 : 1;
        uint32 PCLMULDQ : 1;
        uint32 DTES64 : 1;
        uint32 MONITOR : 1;
        uint32 DS_CPL : 1;
        uint32 VMX : 1;
        uint32 SMX : 1;
        uint32 EIST : 1;
        uint32 TM2 : 1;
        uint32 SSSE3 : 1;
        uint32 CNXT_ID : 1;
        uint32 Reserved1 : 1;
        uint32 FMA : 1;
        uint32 CX16 : 1;
        uint32 xTPR : 1;
        uint32 PDCM : 1;
        uint32 Reserved2 : 1;
        uint32 PCID : 1;
        uint32 DCA : 1;
        uint32 SSE41 : 1;
        uint32 SSE42 : 1;
        uint32 x2APIC : 1;
        uint32 MOVBE : 1;
        uint32 POPCNT : 1;
        uint32 TCS_DEADLINE : 1;
        uint32 AES : 1;
        uint32 XSAVE : 1;
        uint32 OSXSAVE : 1;
        uint32 AVX : 1;
        uint32 Reserved3 : 3;
    } flags;

    uint32 value;
};

union UFeatureFlagsEDX
{
    struct SFeatureFlags
    {
        uint32 FPU : 1;
        uint32 VME : 1;
        uint32 DE : 1;
        uint32 PSE : 1;
        uint32 TSC : 1;
        uint32 MSR : 1;
        uint32 PAE : 1;
        uint32 MCE : 1;
        uint32 CX8 : 1;
        uint32 APIC : 1;
        uint32 Reserved1 : 1;
        uint32 SEP : 1;
        uint32 MTRR : 1;
        uint32 PGE : 1;
        uint32 MCA : 1;
        uint32 CMOV : 1;
        uint32 PAT : 1;
        uint32 PSE36 : 1;
        uint32 PSN : 1;
        uint32 CFLSH : 1;
        uint32 Reserved2 : 1;
        uint32 DS : 1;
        uint32 ACPI : 1;
        uint32 MMX : 1;
        uint32 FXSR : 1;
        uint32 SSE : 1;
        uint32 SSE2 : 1;
        uint32 SS : 1;
        uint32 HTT : 1;
        uint32 TM : 1;
        uint32 Reserved3 : 1;
        uint32 PBE : 1;
    } flags;

    uint32 value;
};

class cpuid
{
  private:
    char m_vendorid[13];
    char m_brand[48];
    UProcessorSignature m_proc_sig;
    UFeatureFlagsECX m_feat_flags1;
    UFeatureFlagsEDX m_feat_flags2;

  public:
    cpuid();
};

static cpuid g_cpuid;

} // namespace forg::cpu
