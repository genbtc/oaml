//-----------------------------------------------------------------------------
// Copyright (c) 2015-2016 Marcelo Fernandez
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "oamlCommon.h"


oamlMusicTrack::oamlMusicTrack() {
	name = "Track";

	playCondSamples = 0;
	playCondAudio = NULL;

	fadeIn = 0;
	fadeOut = 0;
	xfadeIn = 0;
	xfadeOut = 0;

	tailPos = 0;

	introAudio = NULL;
	endAudio = NULL;

	curAudio = NULL;
	tailAudio = NULL;
	fadeAudio = NULL;
}

oamlMusicTrack::~oamlMusicTrack() {
	ClearAudios(&loopAudios);
	ClearAudios(&randAudios);
	ClearAudios(&condAudios);
}

void oamlMusicTrack::AddAudio(oamlAudio *audio) {
	ASSERT(audio != NULL);

	if (audio->GetType() == 1) {
		introAudio = audio;
	} else if (audio->GetType() == 3) {
		endAudio = audio;
	} else if (audio->GetType() == 4) {
		condAudios.push_back(audio);
	} else if (audio->GetRandomChance() > 0) {
		randAudios.push_back(audio);
	} else {
		loopAudios.push_back(audio);
	}
}

void oamlMusicTrack::SetCondition(int id, int value) {
	bool stopCond = false;
	bool playCond = false;

	if (playCondSamples > 0)
		return;

	if (id == CONDITION_MAIN_LOOP) {
		for (size_t i=0; i<loopAudios.size(); i++) {
			oamlAudio *audio = loopAudios[i];
			if (audio->HasCondition(id)) {
				audio->SetPickable(audio->TestCondition(id, value));
			}
		}

		if (curAudio == NULL) {
			PlayNext();
		}
		return;
	}

	for (size_t i=0; i<condAudios.size(); i++) {
		oamlAudio *audio = condAudios[i];
		if (audio->GetCondId() != id)
			continue;

		if (curAudio != audio) {
			// Audio isn't being played right now
			if (audio->TestCondition(id, value) == true) {
				// Condition is true, so let's play the audio
				if (curAudio == NULL || curAudio->GetMinMovementBars() == 0) {
					PlayCond(audio);
				} else {
					PlayCondWithMovement(audio);
				}

				playCond = true;
			}
		} else {
			// Audio is being played right now
			if (audio->TestCondition(id, value) == false) {
				stopCond = true;
			}
		}
	}

	if (stopCond == true && playCond == false) {
		// No condition is being played now, let's go back to the main loop
		if (curAudio == NULL || curAudio->GetMinMovementBars() == 0) {
			PlayCond(NULL);
		} else {
			PlayCondWithMovement(NULL);
		}
	}
}

void oamlMusicTrack::PlayCondWithMovement(oamlAudio *audio) {
	playCondAudio = audio;
	playCondSamples = curAudio->GetBarsSamples(curAudio->GetMinMovementBars());
	if (playCondSamples == 0)
		return;

	playCondSamples = (playCondSamples + curAudio->GetBarsSamples(curAudio->GetSamplesCount() / playCondSamples) * curAudio->GetMinMovementBars()) - curAudio->GetSamplesCount();
//	printf("%s %d\n", __FUNCTION__, playCondSamples);
}

void oamlMusicTrack::PlayCond(oamlAudio *audio) {
	fadeAudio = curAudio;
	curAudio = audio;
	if (curAudio == NULL) {
		PlayNext();
	} else {
		curAudio->Open();
		XFadePlay();
	}
}

int oamlMusicTrack::Play() {
	int doFade = 0;

	if (lock > 0) {
		return -1;
	}

//	__Log("%s %s\n", __FUNCTION__, GetNameStr());
	fadeAudio = NULL;

	if (curAudio == NULL) {
		doFade = 1;
	}

	SetCondition(CONDITION_MAIN_LOOP, 0);

	if (introAudio) {
		curAudio = introAudio;
		curAudio->Open();
	} else {
		PlayNext();
	}

	if (doFade && curAudio) {
		// First check the fade in property for the audio and then the track fade in property
		if (curAudio->GetFadeIn()) {
			curAudio->DoFadeIn(curAudio->GetFadeIn());
		} else if (fadeIn) {
			curAudio->DoFadeIn(fadeIn);
		}
	}

	return 0;
}

void oamlMusicTrack::ShowInfo() {
	printf("%s %lu %lu %lu\n", GetNameStr(), loopAudios.size(), randAudios.size(), condAudios.size());
}

oamlAudio* oamlMusicTrack::PickNextAudio() {
	if (randAudios.size() > 0 && (curAudio == NULL || curAudio->GetRandomChance() == 0)) {
		for (size_t i=0; i<randAudios.size(); i++) {
			int chance = randAudios[i]->GetRandomChance();
			if (Random(0, 100) > chance) {
				continue;
			} else {
				return randAudios[i];
			}
		}
	}

	if (loopAudios.size() == 1) {
		return loopAudios[0];
	} else if (loopAudios.size() >= 2) {
		oamlAudio *list[256];
		int count = 0;

		for (size_t i=0; i<loopAudios.size(); i++) {
			oamlAudio *audio = loopAudios[i];
			if (audio->IsPickable())
				list[count++] = audio;
		}

		if (count == 0) {
			return NULL;
		} else if (count == 1) {
			return list[0];
		} else {
			int r = Random(0, count-1);
			while (curAudio == list[r]) {
				r = Random(0, count-1);
			}

			return list[r];
		}
	}

	return NULL;
}

void oamlMusicTrack::PlayNext() {
	if (curAudio) {
		if (curAudio->GetType() == 4) {
			tailAudio = curAudio;
			tailPos = curAudio->GetSamplesCount();

			curAudio->Open();
			return;
		}
	}

	if (fadeAudio == NULL)
		fadeAudio = curAudio;

	curAudio = PickNextAudio();
	if (curAudio)
		curAudio->Open();

	if (fadeAudio != curAudio) {
		XFadePlay();
	} else {
		fadeAudio = NULL;
	}
}

void oamlMusicTrack::XFadePlay() {
	if (curAudio) {
		// First check the fade in property for the audio and then the track fade in property
		if (curAudio->GetXFadeIn()) {
			curAudio->DoFadeIn(curAudio->GetXFadeIn());
		} else if (fadeAudio && fadeAudio->GetXFadeIn()) {
			curAudio->DoFadeIn(fadeAudio->GetXFadeIn());
		} else if (xfadeIn) {
			curAudio->DoFadeIn(xfadeIn);
		}
	}

	if (fadeAudio) {
		if (curAudio && curAudio->GetXFadeOut()) {
			fadeAudio->DoFadeOut(curAudio->GetXFadeOut());
		} else if (fadeAudio && fadeAudio->GetXFadeOut()) {
			fadeAudio->DoFadeOut(fadeAudio->GetXFadeOut());
		} else if (xfadeOut) {
			fadeAudio->DoFadeOut(xfadeOut);
		} else {
			fadeAudio = NULL;
		}
	}
}

void oamlMusicTrack::Mix(float *samples, int channels, bool debugClipping) {
	lock++;

	if (curAudio) {
		MixAudio(curAudio, samples, channels, debugClipping);
	}

	if (tailAudio) {
		tailPos = MixAudio(tailAudio, samples, channels, debugClipping, tailPos);
		if (tailAudio->HasFinishedTail(tailPos))
			tailAudio = NULL;
	}

	if (fadeAudio) {
		MixAudio(fadeAudio, samples, channels, debugClipping);
	}

	if (curAudio && curAudio->HasFinished()) {
		tailAudio = curAudio;
		tailPos = curAudio->GetSamplesCount();

		PlayNext();
	}

	if (fadeAudio && fadeAudio->HasFinished()) {
		fadeAudio = NULL;
	}

	if (playCondSamples > 0) {
		playCondSamples--;
		if (playCondSamples == 0) {
			PlayCond(playCondAudio);
		}
	}

	lock--;
}

bool oamlMusicTrack::IsPlaying() {
	return (curAudio != NULL || tailAudio != NULL);
}

std::string oamlMusicTrack::GetPlayingInfo() {
	char str[1024];
	std::string info = "";

	if (curAudio == NULL && tailAudio == NULL && fadeAudio == NULL)
		return info;

	info+= GetName() + ":";

	if (curAudio) {
		snprintf(str, 1024, " curAudio = %s (pos=%d)", curAudio->GetFilenameStr(), curAudio->GetSamplesCount());
		info+= str;
	}

	if (tailAudio) {
		snprintf(str, 1024, " tailAudio = %s (pos=%d)", tailAudio->GetFilenameStr(), tailAudio->GetSamplesCount());
		info+= str;
	}

	if (fadeAudio) {
		snprintf(str, 1024, " fadeAudio = %s (pos=%d)", fadeAudio->GetFilenameStr(), fadeAudio->GetSamplesCount());
		info+= str;
	}

	return info;
}

void oamlMusicTrack::Stop() {
	if (curAudio) {
		if (fadeOut) {
			fadeAudio = curAudio;
			fadeAudio->DoFadeOut(fadeOut);
		}
		curAudio = NULL;
	}

	tailAudio = NULL;
}

