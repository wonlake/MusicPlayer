#pragma once

#include "Decodec.h"

class AudioPlayer
{
public:
	AudioPlayer();
	~AudioPlayer();
private:
	static void InitializeALDevice();
	static void UninitializeALDevice();
	void Clean();

private:
	static ALuint m_uiReference;
	Decodec*	  m_pDecodec;

public:
	STDMETHOD(LoadMusicFromFile)(const TCHAR* lpszFileName);
	STDMETHOD(Play)(void);
	STDMETHOD(Stop)(void);
	STDMETHOD(get_End)(VARIANT_BOOL* pVal);
	STDMETHOD(Replay)(void);
	STDMETHOD(get_PlayingProcess)(FLOAT* pVal);
	STDMETHOD(put_PlayingProcess)(FLOAT newVal);
	STDMETHOD(get_Volume)(FLOAT* pVal);
	STDMETHOD(put_Volume)(FLOAT newVal);
	STDMETHOD(Pause)(void);
};

