// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
namespace forg::audio {

enum
{
    MAX_NUM_CHANNELS = 8,
};

struct SAudioFormat
{
    int freq; // samples per second
    int bps;  // bytes per sample
    int chan; // channels
};

struct SAudioBuffer
{
    char* ptr;
    unsigned int size;

    SAudioFormat format;
};

struct SAudioStream
{
    enum EAudioStreamState
    {
        STATE_OFF = 0,
        STATE_ON
    };

    SAudioBuffer buffer;
    SAudioFormat format;

    unsigned int offset;
    unsigned int bytes_left;

    unsigned int state;
};

class IAudioOutput
{
  public:
    virtual bool CanWrite() = 0;
    virtual void Write(char* data, unsigned int size) = 0;

    virtual bool Init() = 0;
    virtual void Release() = 0;
};

} // namespace forg::audio
