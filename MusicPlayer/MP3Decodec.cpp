#include "StdAfx.h"
#include "MP3Decodec.h"

#pragma comment(lib, "libmpg123.lib")

MP3Decodec::MP3Decodec(void)
{
	m_bEnd = false;
	m_fPlayingProcess = 0.0f;
	m_iEndPos	= 1;
	m_iCurPos	= 0;

	m_uiSource = 0;
	for( int i = 0; i < 4; ++i )
		m_uiBuffers[i] = 0;
	m_pDecodeBuffer = NULL;

	m_strFileName = NULL;

	int err = mpg123_init();
	if( err != MPG123_OK)
		MessageBox( NULL, _T("error init!!!"), _T("Info"), MB_OK );
	if( (m_phMPG123 = mpg123_new(NULL, &err)) == NULL )
		MessageBox( NULL, _T("error new object!!!"), _T("Info"), MB_OK );
}


MP3Decodec::~MP3Decodec(void)
{
	Clean();
	mpg123_close( m_phMPG123 );
	mpg123_delete( m_phMPG123 );
	mpg123_exit();	
}

HRESULT MP3Decodec::LoadMusicFromFile(const TCHAR* lpszFileName)
{
	CString strFileName = lpszFileName;

	Clean();

	if( m_phMPG123 == NULL )
		return S_FALSE;

	
	int len = WideCharToMultiByte( CP_ACP, 0, (LPCTSTR)strFileName, -1, NULL, 0, NULL, NULL );
	char* pFileName = new char[len];
	WideCharToMultiByte( CP_ACP, 0, (LPCTSTR)strFileName, -1, pFileName, len, NULL, NULL );

	m_strFileName = SysAllocString( (LPCTSTR)strFileName );

	if( mpg123_open( m_phMPG123, pFileName ) != MPG123_OK ||
		mpg123_getformat( m_phMPG123, &m_lFrequency, &m_iChannels, &m_iEncoding ) != MPG123_OK )
	{
		delete[] pFileName;
		return S_FALSE;
	}
	delete[] pFileName;	

	mpg123_seek( m_phMPG123, 0, SEEK_END );
	m_iEndPos = mpg123_tell(m_phMPG123);
	mpg123_seek( m_phMPG123, 0, SEEK_SET );

	if( m_iEncoding != MPG123_ENC_SIGNED_16 )
	{
		mpg123_format_none( m_phMPG123 );
		mpg123_format( m_phMPG123, m_lFrequency, m_iChannels, MPG123_ENC_SIGNED_16 );
	}

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

	m_pDecodeBuffer = new ALubyte[m_uiBufferSize];
	
	alGenBuffers( 4, m_uiBuffers );
	alGenSources( 1, &m_uiSource );
	
	STREAMINFO streaminfo;
	for( ALint iLoop = 0; iLoop < 4; iLoop++ )
	{
		streaminfo.iPrimDataSize = mpg123_tell(m_phMPG123);
		int err = mpg123_read( m_phMPG123,
			m_pDecodeBuffer, m_uiBufferSize, &m_uiBytesWritten );		
		streaminfo.iPrimDataSize = mpg123_tell(m_phMPG123) -
			streaminfo.iPrimDataSize;
		streaminfo.iRawDataSize = m_uiBytesWritten;

		if( err == MPG123_OK )
		{
			alBufferData( m_uiBuffers[iLoop], 
				m_uiFormat, m_pDecodeBuffer, m_uiBytesWritten, m_lFrequency );
			alSourceQueueBuffers( m_uiSource, 1, &m_uiBuffers[iLoop] );
			m_queueStreams.push( streaminfo );
		}
	}

	return S_OK;
}

HRESULT MP3Decodec::Pause()
{
	ALint iState = 0;
	alGetSourcei( m_uiSource, AL_SOURCE_STATE, &iState );
	if( iState != AL_PAUSED )
		alSourcePause( m_uiSource );

	return S_OK;
}

HRESULT MP3Decodec::Play()
{
	m_iBuffersProcessed = 0;
	alGetSourcei( m_uiSource, AL_BUFFERS_PROCESSED, &m_iBuffersProcessed );
	
	STREAMINFO streaminfo;
	while ( m_iBuffersProcessed )
	{
		m_uiBuffer = 0;
		alSourceUnqueueBuffers( m_uiSource, 1, &m_uiBuffer );
		streaminfo.iPrimDataSize = mpg123_tell(m_phMPG123);
		int err = mpg123_read( m_phMPG123, m_pDecodeBuffer, m_uiBufferSize, &m_uiBytesWritten );
		streaminfo.iPrimDataSize = mpg123_tell(m_phMPG123) -
			streaminfo.iPrimDataSize;
		streaminfo.iRawDataSize = m_uiBytesWritten;

		if ( err == MPG123_OK )
		{
			alBufferData( m_uiBuffer, m_uiFormat, m_pDecodeBuffer, m_uiBytesWritten, m_lFrequency);
			alSourceQueueBuffers( m_uiSource, 1, &m_uiBuffer);
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

HRESULT MP3Decodec::Stop()
{
	ALint iState = 0;
	alGetSourcei( m_uiSource, AL_SOURCE_STATE, &iState );
	if( iState != AL_STOPPED )
		alSourceStop( m_uiSource );

	m_bEnd = true;

	return S_OK;
}

HRESULT MP3Decodec::Clean()
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

	m_bEnd = false;
	m_fPlayingProcess = 0.0f;
	m_iEndPos	= 1;
	m_iCurPos	= 0;

	if( m_strFileName )
		SysFreeString( m_strFileName );
	m_strFileName = NULL;

	while( !m_queueStreams.empty() )
	{
		m_queueStreams.pop();
	}
	m_queueForUpdate = m_queueStreams;

	return S_OK;
}

HRESULT MP3Decodec::IsEnd( VARIANT_BOOL* pbEnd )
{
	if( m_bEnd )
		*pbEnd = VARIANT_TRUE;
	else
		*pbEnd = VARIANT_FALSE;

	return S_OK;
}

HRESULT MP3Decodec::Replay()
{
	if( m_strFileName )
		return LoadMusicFromFile( m_strFileName );

	return S_FALSE;
}

HRESULT MP3Decodec::GetPlayingProcess(FLOAT* pVal)
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
			iNewOffset += (float)iBytes / (float)streaminfo.iRawDataSize * 
				streaminfo.iPrimDataSize;
		}
	}
	m_fPlayingProcess = (float)(m_iCurPos + iNewOffset)/ (float)m_iEndPos;
	*pVal = m_fPlayingProcess;

	return S_OK;
}

HRESULT MP3Decodec::SetPlayingProcess(FLOAT newVal)
{
	if( m_phMPG123 == NULL )
		return S_FALSE;

	m_fPlayingProcess = newVal;
	m_iCurPos = m_fPlayingProcess * m_iEndPos;
	
	if( m_iCurPos >= 0 && m_iCurPos <= m_iEndPos )
		mpg123_seek( m_phMPG123, m_iCurPos, SEEK_SET );

	return S_OK;
}

HRESULT MP3Decodec::GetVolume(FLOAT* pVal)
{
	alGetListenerf( AL_GAIN, pVal );

	return S_OK;
}

HRESULT MP3Decodec::SetVolume(FLOAT newVal)
{
	if( newVal >= 0.0 && newVal <= 1.0f )
		alListenerf( AL_GAIN, newVal );
	else
		return S_FALSE;

	return S_OK;
}