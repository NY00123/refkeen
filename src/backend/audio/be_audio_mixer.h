/* Copyright (C) 2014-2020 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef BE_AUDIO_MIXER
#define BE_AUDIO_MIXER

#include "be_audio_resampling.h"

typedef struct BE_ST_AudioMixerSource
{
	void (*genSamples)(BE_ST_SndSample_T *stream, int length);
	struct
	{
		BE_ST_SndSample_T *buffer;
		int size, num;
	} in, out;
	BESDLResamplingContext resamplingContext;
	int numScaledSamplesToGenNextTime;
	int freq;
	bool skip;
} BE_ST_AudioMixerSource;

void BEL_ST_AudioMixerInit(int freq);
void BEL_ST_AudioMixerShutdown(void);
void BEL_ST_AudioMixerCallback(BE_ST_SndSample_T *stream, int len);
void BEL_ST_AudioMixerUpdateFromPITRateWord(int_fast32_t rateVal);

// Returns a pointer to a new source, which is invalidated after mixer shutdown
BE_ST_AudioMixerSource *BEL_ST_AudioMixerAddSource(
	int freq, int maxNumOfOutSamples,
	void (*genSamples)(BE_ST_SndSample_T *stream, int len));

void BE_ST_AudioMixerSetSourceFreq(BE_ST_AudioMixerSource *src, int freq);

#endif