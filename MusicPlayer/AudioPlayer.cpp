#include "stdafx.h"
#include "AudioPlayer.h"
#include "MusicPlayer.h"
#include "MP3Decodec.h"
#include "WaveDecodec.h"
#include "OggDecodec.h"

#pragma comment(lib, "OpenAL32.lib")

AudioPlayer::AudioPlayer()
{
	m_pDecodec = NULL;
	InitializeALDevice();
}


AudioPlayer::~AudioPlayer()
{
	if (m_pDecodec)
	{
		delete m_pDecodec;
		m_pDecodec = NULL;
	}
	UninitializeALDevice();
}

STDMETHODIMP AudioPlayer::LoadMusicFromFile(const TCHAR* lpszFileName)
{
	// TODO: 在此添加实现代码
	if (m_pDecodec)
	{
		delete m_pDecodec;
		m_pDecodec = NULL;
	}
	CString strFileName = lpszFileName;
	strFileName = strFileName.MakeLower();
	int length = strFileName.GetLength();
	if (length < 4)
		return S_FALSE;
	int i = strFileName.Find(_T(".mp3"), length - 4);
	if (i != -1)
	{
		m_pDecodec = new MP3Decodec();
	}
	else if (strFileName.Find(_T(".wav"), length - 4) != -1)
	{
		m_pDecodec = new WaveDecodec();
	}
	else if (strFileName.Find(_T(".ogg"), length - 4) != -1)
	{
		m_pDecodec = new OggDecodec();
	}

	if (m_pDecodec)
	{
		if (m_pDecodec->LoadMusicFromFile(lpszFileName) == S_FALSE)
		{
			delete m_pDecodec;
			m_pDecodec = NULL;
			return S_FALSE;
		}
		return S_OK;
	}

	return S_FALSE;
}


STDMETHODIMP AudioPlayer::Play(void)
{
	// TODO: 在此添加实现代码
	if (m_pDecodec)
		m_pDecodec->Play();

	return S_OK;
}


STDMETHODIMP AudioPlayer::Stop(void)
{
	// TODO: 在此添加实现代码
	if (m_pDecodec)
		m_pDecodec->Stop();

	return S_OK;
}

void AudioPlayer::InitializeALDevice()
{
	//char *pDefaultDeviceName =
	//	(char*)alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	ALCdevice  *pDevice = alcOpenDevice(NULL);
	ALCcontext *pContext = alcCreateContext(pDevice, NULL);
	alcMakeContextCurrent(pContext);
}

void AudioPlayer::UninitializeALDevice()
{
	ALCcontext *pContext = alcGetCurrentContext();
	ALCdevice *pDevice = alcGetContextsDevice(pContext);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(pContext);
	alcCloseDevice(pDevice);
}

STDMETHODIMP AudioPlayer::get_End(VARIANT_BOOL* pVal)
{
	// TODO: 在此添加实现代码
	if (m_pDecodec)
		m_pDecodec->IsEnd(pVal);
	else
		*pVal = VARIANT_TRUE;

	return S_OK;
}


STDMETHODIMP AudioPlayer::Replay(void)
{
	// TODO: 在此添加实现代码
	if (m_pDecodec)
		m_pDecodec->Replay();

	return S_OK;
}


STDMETHODIMP AudioPlayer::get_PlayingProcess(FLOAT* pVal)
{
	// TODO: 在此添加实现代码
	if (m_pDecodec)
		m_pDecodec->GetPlayingProcess(pVal);

	return S_OK;
}


STDMETHODIMP AudioPlayer::put_PlayingProcess(FLOAT newVal)
{
	// TODO: 在此添加实现代码
	if (m_pDecodec)
		m_pDecodec->SetPlayingProcess(newVal);

	return S_OK;
}


STDMETHODIMP AudioPlayer::get_Volume(FLOAT* pVal)
{
	// TODO: 在此添加实现代码
	if (m_pDecodec)
		m_pDecodec->GetVolume(pVal);

	return S_OK;
}


STDMETHODIMP AudioPlayer::put_Volume(FLOAT newVal)
{
	// TODO: 在此添加实现代码
	if (m_pDecodec)
		m_pDecodec->SetVolume(newVal);

	return S_OK;
}


STDMETHODIMP AudioPlayer::Pause(void)
{
	// TODO: 在此添加实现代码
	if (m_pDecodec)
		m_pDecodec->Pause();

	return S_OK;
}