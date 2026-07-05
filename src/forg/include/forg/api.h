// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#define FXIMPORT __declspec(dllimport)
#define FXEXPORT __declspec(dllexport)
#define FXDLLLOCAL
#define FXDLLPUBLIC
#else
#ifdef GCC_HASCLASSVISIBILITY
#define FXIMPORT __attribute__((visibility("default")))
#define FXEXPORT __attribute__((visibility("default")))
#define FXDLLLOCAL __attribute__((visibility("hidden")))
#define FXDLLPUBLIC __attribute__((visibility("default")))
#else
#define FXIMPORT
#define FXEXPORT
#define FXDLLLOCAL
#define FXDLLPUBLIC
#endif
#endif

#ifdef FORG_EXPORTS
#define FORG_API FXEXPORT
#else
#ifdef FORG_STATIC
#define FORG_API
#else
#define FORG_API FXIMPORT
#endif
#endif

#ifdef __cplusplus
}
#endif
