/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2005  Slawomir Strumecki

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

#ifndef _FORG_API_H_
#define _FORG_API_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
  #define FXIMPORT __declspec(dllimport)
  #define FXEXPORT __declspec(dllexport)
  #define FXDLLLOCAL
  #define FXDLLPUBLIC
#else
  #ifdef GCC_HASCLASSVISIBILITY
    #define FXIMPORT __attribute__ ((visibility("default")))
    #define FXEXPORT __attribute__ ((visibility("default")))
    #define FXDLLLOCAL __attribute__ ((visibility("hidden")))
    #define FXDLLPUBLIC __attribute__ ((visibility("default")))
  #else
    #define FXIMPORT
    #define FXEXPORT
    #define FXDLLLOCAL
    #define FXDLLPUBLIC
  #endif
#endif

#ifdef FORG_EXPORTS
#   define FORG_API FXEXPORT
#else
#ifdef FORG_STATIC
#   define FORG_API
#else
#   define FORG_API FXIMPORT
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif //_FORG_API_H_
