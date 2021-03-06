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

#ifndef __OAMLBASE_H__
#define __OAMLBASE_H__


#include "tinyxml2.h"
#ifdef __HAVE_RTAUDIO
#include "RtAudio.h"
#endif


class oamlBase {
private:
	std::string defsFile;
	std::string playingInfo;

	bool verbose;
	bool debugClipping;
	bool writeAudioAtShutdown;
	bool useCompressor;
	bool updateTension;

	std::vector<oamlTrack*> musicTracks;
	std::vector<oamlTrack*> sfxTracks;
	std::vector<oamlLayer*> layers;

	float bpm;
	int beatsPerBar;

	oamlTrack *curTrack;

	ByteBuffer *fullBuffer;

#ifdef __HAVE_RTAUDIO
	RtAudio *rtAudio;
#endif

	int sampleRate;
	int channels;
	int bytesPerSample;
	bool floatBuffer;

	int tension;
	uint64_t tensionMs;
	float volume;
	bool pause;

	oamlFileCallbacks *fcbs;

	uint64_t timeMs;

	oamlCompressor compressor;

	oamlTracksInfo tracksInfo;

	void Clear();

	oamlRC PlayTrackId(int id);
	bool IsTrackPlayingId(int id);

	void ShowPlayingTracks();
	oamlRC ReadAudioDefs(tinyxml2::XMLElement *el, oamlTrack *track);
	oamlRC ReadTrackDefs(tinyxml2::XMLElement *el);
	oamlRC ReadDefs(const char *buf, int size);
	void ReadInternalDefs(const char *filaname);

	int ReadSample(void *buffer, int index);
	void WriteSample(void *buffer, int index, int sample);

	bool IsAudioFormatSupported();

	void AddLayer(std::string layer);
	int GetLayerId(std::string layer);
	oamlLayer *GetLayer(std::string layer);

	void UpdateTension(uint64_t ms);

	oamlTrack* GetTrack(std::string name);
	oamlAudio* GetAudio(std::string trackName, std::string audioName);
	oamlAudioFile* GetAudioFile(std::string trackName, std::string audioName, std::string filename);

public:
	oamlBase();
	~oamlBase();

	const char* GetVersion() { return OAML_VERSION_STRING; }

	oamlRC InitAudioDevice(int sampleRate, int channels);
	oamlRC Init(const char *defsFilename);
	oamlRC ReadDefsFile(const char *defsFilename);
	oamlRC InitString(const char *defs);
	void Shutdown();

	void SetVerbose(bool option) { verbose = option; }
	void SetDebugClipping(bool option) { debugClipping = option; }
	void SetWriteAudioAtShutdown(bool option) { writeAudioAtShutdown = option; }

	void SetAudioFormat(int audioSampleRate, int audioChannels, int audioBytesPerSample, bool audioFloatBuffer);
	void SetVolume(float vol);
	float GetVolume() const { return volume; }

	oamlRC PlayTrack(const char *name);
	oamlRC PlayTrackWithStringRandom(const char *str);
	oamlRC PlayTrackByGroupRandom(const char *group);
	oamlRC PlayTrackByGroupAndSubgroupRandom(const char *group, const char *subgroup);
	oamlRC PlaySfx(const char *name);
	oamlRC PlaySfxEx(const char *name, float vol, float pan);
	oamlRC PlaySfx2d(const char *name, int x, int y, int width, int height);

	oamlRC LoadTrack(const char *name);
	float LoadTrackProgress(const char *name);

	void StopPlaying();
	void Pause();
	void Resume();
	void PauseToggle();

	bool IsPaused() const { return pause; }

	bool IsTrackPlaying(const char *name);
	bool IsPlaying();

	void AddTension(int value);
	void SetTension(int value);

	void SetMainLoopCondition(int value);

	void SetCondition(int id, int value);

	void SetLayerGain(const char *layer, float gain);
	void SetLayerRandomChance(const char *layer, int randomChance);

	void MixToBuffer(void *buffer, int size);

	void Update();

	int SafeAdd(int sample1, int sample2);

	void SetFileCallbacks(oamlFileCallbacks *cbs);

	void EnableDynamicCompressor(bool enable, double thresholdDb, double ratio);

	oamlTracksInfo *GetTracksInfo();

	const char* GetDefsFile();
	const char* GetPlayingInfo();

	void ProjectNew();
	void ProjectSetBPM(float bpm);
	void ProjectSetBeatsPerBar(int beatsPerBar);
	float ProjectGetBPM();
	int ProjectGetBeatsPerBar();

	oamlRC TrackNew(std::string name, bool sfxTrack = false);
	oamlRC TrackRemove(std::string name);
	void TrackRename(std::string name, std::string newName);
	void TrackSetVolume(std::string name, float volume);
	void TrackSetFadeIn(std::string name, int fadeIn);
	void TrackSetFadeOut(std::string name, int fadeOut);
	void TrackSetXFadeIn(std::string name, int xFadeIn);
	void TrackSetXFadeOut(std::string name, int xFadeOut);

	bool TrackExists(std::string name);
	bool TrackIsSfxTrack(std::string name);
	bool TrackIsMusicTrack(std::string name);
	void TrackGetAudioList(std::string name, std::vector<std::string>& list);
	float TrackGetVolume(std::string name);
	int TrackGetFadeIn(std::string name);
	int TrackGetFadeOut(std::string name);
	int TrackGetXFadeIn(std::string name);
	int TrackGetXFadeOut(std::string name);

	oamlRC AudioNew(std::string trackName, std::string audioName, int type);
	oamlRC AudioRemove(std::string trackName, std::string audioName);
	void AudioAddAudioFile(std::string trackName, std::string audioName, std::string filename);
	void AudioSetName(std::string trackName, std::string audioName, std::string name);
	void AudioSetVolume(std::string trackName, std::string audioName, float volume);
	void AudioSetBPM(std::string trackName, std::string audioName, float bpm);
	void AudioSetBeatsPerBar(std::string trackName, std::string audioName, int beatsPerBar);
	void AudioSetBars(std::string trackName, std::string audioName, int bars);
	void AudioSetMinMovementBars(std::string trackName, std::string audioName, int minMovementBars);
	void AudioSetRandomChance(std::string trackName, std::string audioName, int randomChance);
	void AudioSetFadeIn(std::string trackName, std::string audioName, int fadeIn);
	void AudioSetFadeOut(std::string trackName, std::string audioName, int fadeOut);
	void AudioSetXFadeIn(std::string trackName, std::string audioName, int xFadeIn);
	void AudioSetXFadeOut(std::string trackName, std::string audioName, int xFadeOut);
	void AudioSetCondId(std::string trackName, std::string audioName, int condId);
	void AudioSetCondType(std::string trackName, std::string audioName, int condType);
	void AudioSetCondValue(std::string trackName, std::string audioName, int condValue);
	void AudioSetCondValue2(std::string trackName, std::string audioName, int condValue2);

	bool AudioExists(std::string trackName, std::string audioName);
	void AudioGetAudioFileList(std::string trackName, std::string audioName, std::vector<std::string>& list);
	int AudioGetType(std::string trackName, std::string audioName);
	float AudioGetVolume(std::string trackName, std::string audioName);
	float AudioGetBPM(std::string trackName, std::string audioName);
	int AudioGetBeatsPerBar(std::string trackName, std::string audioName);
	int AudioGetBars(std::string trackName, std::string audioName);
	int AudioGetMinMovementBars(std::string trackName, std::string audioName);
	int AudioGetRandomChance(std::string trackName, std::string audioName);
	int AudioGetFadeIn(std::string trackName, std::string audioName);
	int AudioGetFadeOut(std::string trackName, std::string audioName);
	int AudioGetXFadeIn(std::string trackName, std::string audioName);
	int AudioGetXFadeOut(std::string trackName, std::string audioName);
	int AudioGetCondId(std::string trackName, std::string audioName);
	int AudioGetCondType(std::string trackName, std::string audioName);
	int AudioGetCondValue(std::string trackName, std::string audioName);
	int AudioGetCondValue2(std::string trackName, std::string audioName);

	void AudioFileRemove(std::string trackName, std::string audioName, std::string filename);
	void AudioFileSetLayer(std::string trackName, std::string audioName, std::string filename, std::string layer);
	void AudioFileSetRandomChance(std::string trackName, std::string audioName, std::string filename, int randomChance);

	std::string AudioFileGetLayer(std::string trackName, std::string audioName, std::string filename);
	int AudioFileGetRandomChance(std::string trackName, std::string audioName, std::string filename);

	oamlRC LayerNew(std::string name);
	void LayerList(std::vector<std::string>& list);
	void LayerRename(std::string layerName, std::string name);
	int LayerGetId(std::string layerName);
	int LayerGetRandomChance(std::string layerName);
	float LayerGetGain(std::string layerName);

	int GetSampleRate() const { return sampleRate; }
};

#endif /* __OAMLBASE_H__ */
