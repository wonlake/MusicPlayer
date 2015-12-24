#include "StdAfx.h"
#include "OggDecodec.h"

#ifdef _DEBUG
#pragma comment(lib, "libogg_d")
#pragma comment(lib, "libvorbisfile_d")
#pragma comment(lib, "libvorbis_d")
#else
#pragma comment(lib, "libogg")
#pragma comment(lib, "libvorbisfile")
#pragma comment(lib, "libvorbis")
#endif

OggDecodec::OggDecodec(void)
{
	m_bEnd = false;

	m_fPlayingProcess = 0.0f;
	m_iEndPos	= 1;
	m_iCurPos	= 0;

	m_uiSource = 0;
	for( int i = 0; i < 4; ++i )
		m_uiBuffers[i] = 0;
	m_pDecodeBuffer = NULL;	

	m_Callbacks.read_func  = ov_read_func;
	m_Callbacks.seek_func  = ov_seek_func;
	m_Callbacks.close_func = ov_close_func;
	m_Callbacks.tell_func  = ov_tell_func;

	memset( &m_OggVorbisFile, 0, sizeof(m_OggVorbisFile) );

	m_strFileName = NULL;
}


OggDecodec::~OggDecodec(void)
{
	Clean();

	m_Callbacks.read_func  = NULL;
	m_Callbacks.seek_func  = NULL;
	m_Callbacks.close_func = NULL;
	m_Callbacks.tell_func  = NULL;
}


HRESULT OggDecodec::LoadMusicFromFile(const TCHAR* lpszFileName)
{
	CString strFileName = lpszFileName;

	Clean();
	
	int len = WideCharToMultiByte( CP_ACP, 0, (LPCTSTR)strFileName, -1, NULL, 0, NULL, NULL );
	char* pFileName = new char[len];
	WideCharToMultiByte( CP_ACP, 0, (LPCTSTR)strFileName, -1, pFileName, len, NULL, NULL );
	m_strFileName = SysAllocString( (LPCTSTR)strFileName );

	FILE *pOggVorbisFile = fopen( pFileName, "rb" );
	if( pOggVorbisFile == NULL )
	{
		delete[] pFileName;	
		return S_FALSE;
	}
	delete[] pFileName;	
	
	if( ov_open_callbacks( pOggVorbisFile,
		&m_OggVorbisFile, NULL, 0, m_Callbacks ) == 0 )
	{
		m_pVorbisInfo = ov_info( &m_OggVorbisFile, -1 );
		if( m_pVorbisInfo )
		{
			m_lFrequency = m_pVorbisInfo->rate;
			m_iChannels  = m_pVorbisInfo->channels;
			
			//每通道2字节,缓冲区大小应是单位频率内所有通道字节数和的整数倍!			
			switch( m_iChannels )
			{
			case 1:
				m_uiFormat = AL_FORMAT_MONO16;
				m_uiBufferSize = m_lFrequency >> 1;
				m_uiBufferSize -= (m_uiBufferSize % 2);
				break;
			case 2:
				m_uiFormat = AL_FORMAT_STEREO16;
				m_uiBufferSize = m_lFrequency;
				m_uiBufferSize -= (m_uiBufferSize % 4);
				break;
			case 4:
				m_uiFormat = alGetEnumValue("AL_FORMAT_QUAD16");
				m_uiBufferSize = m_lFrequency * 2;
				m_uiBufferSize -= (m_uiBufferSize % 8);
				break;
			case 6:
				m_uiFormat = alGetEnumValue("AL_FORMAT_51CHN16");
				m_uiBufferSize = m_lFrequency * 3;
				m_uiBufferSize -= (m_uiBufferSize % 12);
				break;
			default:
				break;
			}
		}

		if( m_uiFormat )
		{
			m_pDecodeBuffer = new ALubyte[m_uiBufferSize];

			alGenBuffers( 4, m_uiBuffers );
			alGenSources( 1, &m_uiSource );

			STREAMINFO streaminfo;
			m_iEndPos = ov_pcm_total( &m_OggVorbisFile, 0 );

			for( ALint iLoop = 0; iLoop < 4; iLoop++ )
			{
				streaminfo.iPrimDataSize = ov_pcm_tell( &m_OggVorbisFile );
				int ulBytesWritten = DecodeOggVorbis( &m_OggVorbisFile,
					(char*)m_pDecodeBuffer, m_uiBufferSize, m_iChannels );
				streaminfo.iPrimDataSize = ov_pcm_tell( &m_OggVorbisFile ) -
					streaminfo.iPrimDataSize;
				streaminfo.iRawDataSize = ulBytesWritten;

				if( ulBytesWritten )
				{
					alBufferData( m_uiBuffers[iLoop], m_uiFormat,
						m_pDecodeBuffer, ulBytesWritten, m_lFrequency );
					alSourceQueueBuffers( m_uiSource, 1, &m_uiBuffers[iLoop] );
					m_queueStreams.push( streaminfo );
				}
			}
		}
	}

	return S_OK;
}

HRESULT OggDecodec::Pause()
{
	ALint iState = 0;
	alGetSourcei( m_uiSource, AL_SOURCE_STATE, &iState );
	if( iState != AL_PAUSED )
		alSourcePause( m_uiSource );

	return S_OK;
}

HRESULT OggDecodec::Play()
{
	m_iBuffersProcessed = 0;
	alGetSourcei( m_uiSource, AL_BUFFERS_PROCESSED, &m_iBuffersProcessed );

	STREAMINFO streaminfo;
	while ( m_iBuffersProcessed )
	{
		m_uiBuffer = 0;
		streaminfo.iPrimDataSize = ov_pcm_tell( &m_OggVorbisFile );
		alSourceUnqueueBuffers( m_uiSource, 1, &m_uiBuffer );
		m_uiBytesWritten = DecodeOggVorbis( &m_OggVorbisFile,
			(char*)m_pDecodeBuffer, m_uiBufferSize, m_iChannels );
		streaminfo.iPrimDataSize = ov_pcm_tell( &m_OggVorbisFile ) -
			streaminfo.iPrimDataSize;
		streaminfo.iRawDataSize = m_uiBytesWritten;
					
		if( m_uiBytesWritten )
		{
			alBufferData( m_uiBuffer, 
				m_uiFormat, m_pDecodeBuffer, m_uiBytesWritten, m_lFrequency );
			alSourceQueueBuffers( m_uiSource, 1, &m_uiBuffer );	
			m_iCurPos += m_queueStreams.front().iPrimDataSize;
			m_queueStreams.pop();
			m_queueStreams.push( streaminfo );
		}
		else
		{			
			m_iCurPos += m_queueStreams.front().iPrimDataSize;
			m_queueStreams.pop();
		}			

		m_iBuffersProcessed--;
	}

	ALint iState = 0;
	alGetSourcei( m_uiSource, AL_SOURCE_STATE, &iState );
	if( iState != AL_PLAYING )
	{
		alGetSourcei( m_uiSource, AL_BUFFERS_QUEUED, &m_iQueuedBuffers );
		if ( m_iQueuedBuffers )
			alSourcePlay( m_uiSource );
		else
		{
			m_bEnd = true;
		}		
	}

	return S_OK;
}

HRESULT OggDecodec::Replay()
{
	if( m_strFileName )
		return LoadMusicFromFile( m_strFileName );

	return S_FALSE;
}

HRESULT OggDecodec::Stop()
{
	ALint iState = 0;
	alGetSourcei( m_uiSource, AL_SOURCE_STATE, &iState );
	if( iState != AL_STOPPED )
		alSourceStop( m_uiSource );

	m_bEnd = true;

	return S_OK;
}

HRESULT OggDecodec::Clean()
{
	if( alIsSource( m_uiSource ) )
		alSourcei( m_uiSource, AL_BUFFER, 0 );

	for( int i = 0; i < 4; ++i )
	{
		if( m_uiBuffers[i] )
		{
			alDeleteBuffers( 1, &m_uiBuffers[i] );
			m_uiBuffers[i] = 0;
		}
	}

	if( m_uiSource )
	{
		alDeleteSources( 1, &m_uiSource );
		m_uiSource = 0;
	}

	if( m_pDecodeBuffer )
	{
		delete[] m_pDecodeBuffer;
		m_pDecodeBuffer = NULL;
	}

	m_bEnd		= false;
	m_iEndPos	= 1;
	m_iCurPos	= 0;
	m_fPlayingProcess = 0.0f;

	if( m_strFileName )
		SysFreeString( m_strFileName );
	m_strFileName = NULL;

	while( !m_queueStreams.empty() )
	{
		m_queueStreams.pop();
	}
	m_queueForUpdate = m_queueStreams;

	ov_clear( &m_OggVorbisFile );

	return S_OK;
}

HRESULT OggDecodec::IsEnd( VARIANT_BOOL* pbEnd )
{
	if( m_bEnd )
		*pbEnd = VARIANT_TRUE;
	else
		*pbEnd = VARIANT_FALSE;

	return S_OK;
}

HRESULT OggDecodec::GetPlayingProcess(FLOAT* pVal)
{
	ALint iBytes = 0;
	off_t iNewOffset = 0;

	alGetSourcei( m_uiSource, AL_BYTE_OFFSET, &iBytes );
	iBytes %= m_uiBufferSize;

	int iBuffersProcessed;
	alGetSourcei( m_uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed );
	m_queueForUpdate = m_queueStreams;

	if( !m_queueForUpdate.empty() )
	{
		while( iBuffersProcessed )
		{
			iNewOffset += m_queueForUpdate.front().iPrimDataSize;
			m_queueForUpdate.pop();
			--iBuffersProcessed;
		}
		if( !m_queueForUpdate.empty() )
		{
			STREAMINFO streaminfo = m_queueForUpdate.front();
			iNewOffset += (off_t)((float)iBytes / (float)streaminfo.iRawDataSize * 
				streaminfo.iPrimDataSize);
		}
	}

	m_fPlayingProcess = (float)(m_iCurPos + iNewOffset)/ (float)(m_iEndPos);
	*pVal = m_fPlayingProcess;


	return S_OK;
}

HRESULT OggDecodec::SetPlayingProcess(FLOAT newVal)
{
	return S_OK;
}

HRESULT OggDecodec::GetVolume(FLOAT* pVal)
{
	alGetListenerf( AL_GAIN, pVal );

	return S_OK;
}

HRESULT OggDecodec::SetVolume(FLOAT newVal)
{
	if( newVal >= 0.0 && newVal <= 1.0f )
		alListenerf( AL_GAIN, newVal );
	else
		return S_FALSE;

	return S_OK;
}

void OggDecodec::Swap( short &s1, short &s2 )
{
	short sTemp = s1;
	s1 = s2;
	s2 = sTemp;
}

unsigned long OggDecodec::DecodeOggVorbis( OggVorbis_File* pOggVorbisFile, 
	char* pDecodeBuffer, unsigned long ulBufferSize,
	int iChannels )
{
	int current_section;
	long lDecodeSize;
	unsigned long ulSamples;
	short *pSamples;

	unsigned long ulBytesDone = 0;
	while( true )
	{
		lDecodeSize = ov_read( pOggVorbisFile, pDecodeBuffer + ulBytesDone, 
			ulBufferSize - ulBytesDone, 0, 2, 1, &current_section );
		if (lDecodeSize > 0)
		{
			ulBytesDone += lDecodeSize;

			if (ulBytesDone >= ulBufferSize)
				break;
		}
		else
		{
			break;
		}
	}

	// Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
	// however 6-Channels files need to be re-ordered
	if ( iChannels == 6 )
	{		
		pSamples = (short*)pDecodeBuffer;
		for (ulSamples = 0; ulSamples < (ulBufferSize>>1); ulSamples+=6)
		{
			// WAVEFORMATEXTENSIBLE Order : FL, FR, FC, LFE, RL, RR
			// OggVorbis Order            : FL, FC, FR,  RL, RR, LFE
			Swap(pSamples[ulSamples+1], pSamples[ulSamples+2]);
			Swap(pSamples[ulSamples+3], pSamples[ulSamples+5]);
			Swap(pSamples[ulSamples+4], pSamples[ulSamples+5]);
		}
	}
	return ulBytesDone;
}

size_t OggDecodec::ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return fread(ptr, size, nmemb, (FILE*)datasource);
}

int OggDecodec::ov_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	return fseek((FILE*)datasource, (long)offset, whence);
}

int OggDecodec::ov_close_func(void *datasource)
{
	return fclose((FILE*)datasource);
}

long OggDecodec::ov_tell_func(void *datasource)
{
	return ftell((FILE*)datasource);
}