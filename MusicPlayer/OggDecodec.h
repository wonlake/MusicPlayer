#pragma once

//ogg½âÂë¿â
#include <Vorbis/vorbisfile.h>
#include "decodec.h"
#include <queue>

class OggDecodec :	public Decodec
{
public:
	OggDecodec(void);
	~OggDecodec(void);

private:
	ov_callbacks	m_Callbacks;
	OggVorbis_File	m_OggVorbisFile;
	vorbis_info*	m_pVorbisInfo;

	long		    m_lFrequency;
	ALuint			m_uiFormat;
	int				m_iChannels;
	ALuint			m_uiBufferSize;
	ALuint			m_uiBytesWritten;

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

private:

	void Swap(short &s1, short &s2);

	unsigned long DecodeOggVorbis( OggVorbis_File* pOggVorbisFile, 
		char* pDecodeBuffer, unsigned long ulBufferSize,
		int iChannels );

	static size_t ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
	static int    ov_seek_func(void *datasource, ogg_int64_t offset, int whence);
	static int    ov_close_func(void *datasource);
	static long   ov_tell_func(void *datasource);
};

