#pragma once

//mp3½âÂë¿â
#include <mpg123.h>
#include "Decodec.h"
#include <queue>

class MP3Decodec : public Decodec
{
public:
	MP3Decodec(void);
	~MP3Decodec(void);

private:
	mpg123_handle*	m_phMPG123;
	long		    m_lFrequency;
	ALuint			m_uiFormat;
	int				m_iChannels;
	ALuint			m_uiBufferSize;
	size_t			m_uiBytesWritten;

	ALuint			m_uiBuffers[4];
	ALuint			m_uiSource;
	ALuint			m_uiBuffer;
	ALint			m_iBuffersProcessed;
	ALint			m_iQueuedBuffers;

	ALubyte*		m_pDecodeBuffer;
	int 			m_iEncoding;
	bool			m_bEnd;

	BSTR			m_strFileName;
	float			m_fPlayingProcess;

	off_t			m_iEndPos;
	off_t			m_iCurPos;

	std::queue<STREAMINFO> m_queueStreams;
	std::queue<STREAMINFO> m_queueForUpdate;

public:
	HRESULT LoadMusicFromFile(const TCHAR* lpszFileName);
	HRESULT Pause();
	HRESULT Play();
	HRESULT Replay();
	HRESULT Stop();
	HRESULT Clean();
	HRESULT IsEnd( VARIANT_BOOL* pbEnd );
	HRESULT GetPlayingProcess(FLOAT* pVal);
	HRESULT SetPlayingProcess(FLOAT newVal);
	HRESULT GetVolume(FLOAT* pVal);
	HRESULT SetVolume(FLOAT newVal);

};

