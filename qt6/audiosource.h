// /^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\
// Copyright (C) 2026 c1audio.com / Claudio Zopfi <c1audio@x21.ch>
// SPDX-License-Identifier: GPL-3.0

#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

//! Additional sound generator mixed into the output of Qt68Wraper
/*!
  Lets other synth engines (e.g. a soundfont player) share the audio
  device, buffer settings and level meter of the mobilesynth output.
*/
class AudioSource
{
public:
    virtual ~AudioSource() = default;

    //! Called from the control thread when the output format changes
    virtual void setSampleRate(int sampleRate) = 0;

    //! Called from the audio thread: fill left/right with frames samples (-1..1)
    /*!
      Must not block. Returns false if nothing was rendered,
      the buffers are ignored in that case.
    */
    virtual bool render(float *left, float *right, int frames) = 0;
};

#endif // AUDIOSOURCE_H
