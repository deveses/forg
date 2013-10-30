#pragma once

#ifdef EMFC_EXPORTS
#define EMFC_API __declspec(dllexport)
#else
#ifdef EMFC_STATIC
#define EMFC_API
#else 
#define EMFC_API __declspec(dllimport)
#endif
#endif

#define null 0
