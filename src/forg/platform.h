// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#if defined(_WIN32) | defined(_WIN64)
#define PLATFORM_WINDOWS
#endif

#ifdef _WIN32
#define PLATFORM_WIN32
#endif

#ifdef _WIN64
#define PLATFORM_WIN64
#endif

#ifdef __linux__
#define PLATFORM_LINUX
#endif
