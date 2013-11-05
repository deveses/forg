#include "forg_pch.h"
#include "cpuid.h"
#include "debug/dbg.h"

namespace forg { namespace cpu {

uint32 invoke_cpuid(int param, uint32& _eax, uint32& _ebx, uint32& _ecx, uint32& _edx)
{
	int cpui[4];
	__cpuid(cpui, param);

    _eax = cpui[0];
	_ebx = cpui[1];
	_ecx = cpui[2];
	_edx = cpui[3];

	return cpui[0];
}

cpuid::cpuid()
{
    uint32 eax, ebx, ecx, edx;
 
    // vendor id
    m_vendorid[0] = 0;
    uint32* t = (uint32*)m_vendorid;
    uint32 num_fun = invoke_cpuid(0, eax, t[0], t[2], t[1]);
    m_vendorid[12] = 0;
    DBG_MSG("[CPUID] vendorid: %s, num_fun: %d\n", m_vendorid, num_fun);

    // signature
    eax = invoke_cpuid(1, eax, ebx, ecx, edx);
    
    m_proc_sig.value = eax;
    DBG_MSG("[CPUID] signature: %x:%x:%x:%x:%x:%x\n", m_proc_sig.sig.SteppingID, m_proc_sig.sig.ModelNumber, m_proc_sig.sig.FamilyCode, m_proc_sig.sig.Type,
        m_proc_sig.sig.ExtendedModel, m_proc_sig.sig.ExtendedFamily);

    // feature flags
    m_feat_flags1.value = ecx;

    m_feat_flags2.value = edx;

    // extended functions
    eax = invoke_cpuid(0x80000000, eax, ebx, ecx, edx);
    eax ^= 0x80000000;  

    m_brand[0] = 0;
    if (eax >= 4)
    {

        uint32* regb = (uint32*)m_brand;

        invoke_cpuid(0x80000002, regb[0], regb[1], regb[2], regb[3]);
        invoke_cpuid(0x80000003, regb[4], regb[5], regb[6], regb[7]);
        invoke_cpuid(0x80000004, regb[8], regb[9], regb[10], regb[11]);

        DBG_MSG("[CPUID] processor brand string: %s\n", m_brand);
    }
}

}}
