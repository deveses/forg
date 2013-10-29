#ifndef DBG_NEW_H_INCLUDED
#define DBG_NEW_H_INCLUDED

#include "dbg.h"

namespace forg { namespace debug {

inline void* __cdecl operator new(size_t size, char const * file, int line)
{
    void *p = malloc(size);
    RMSG("%s: %d", file, line);
//      if (Tracer::Ready)
//        NewTrace.Add (p, file, line);
    return p;
}

inline void __cdecl operator delete(void * p, char const * file, int line)
{
//    if (Tracer::Ready)
//        NewTrace.Remove (p);
    free (p);
}

inline void* __cdecl operator new(size_t size)
{
    void * p = malloc (size);
    //if (Tracer::Ready)
    //    NewTrace.Add (p, "?", 0);
    return p;
}

inline void __cdecl operator delete(void * p)
{
//    if (Tracer::Ready)
//        NewTrace.Remove (p);
    free (p);
}

}}

#endif // DBG_NEW_H_INCLUDED
