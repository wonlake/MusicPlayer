#include "StdAfx.h"
#include "WaveDecodec.h"
#include <Mmreg.h>
// Speaker Positions:
#define SPEAKER_FRONT_LEFT              0x1
#define SPEAKER_FRONT_RIGHT             0x2
#define SPEAKER_FRONT_CENTER            0x4
#define SPEAKER_LOW_FREQUENCY           0x8
#define SPEAKER_BACK_LEFT               0x10
#define SPEAKER_BACK_RIGHT              0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER    0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER   0x80
#define SPEAKER_BACK_CENTER             0x100
#define SPEAKER_SIDE_LEFT               0x200
#define SPEAKER_SIDE_RIGHT              0x400
#define SPEAKER_TOP_CENTER              0x800
#define SPEAKER_TOP_FRONT_LEFT          0x1000
#define SPEAKER_TOP_FRONT_CENTER        0x2000
#define SPEAKER_TOP_FRONT_RIGHT         0x4000
#define SPEAKER_TOP_BACK_LEFT           0x8000
#define SPEAKER_TOP_BACK_CENTER         0x10000
#define SPEAKER_TOP_BACK_RIGHT          0x20000

#if !defined(WAVE_FORMAT_EXTENSIBLE)
#define  WAVE_FORMAT_EXTENSIBLE                 0xFFFE
#endif // !defined(WAVE_FORMAT_EXTENSIBLE)

#pragma pack(push, 4)

typedef struct
{
	char			szRIFF[4];
	unsigned long	ulRIFFSize;
	char			szWAVE[4];
} WAVEFILEHEADER;

typedef struct
{
	char			szChunkName[4];
	unsigned long	ulChunkSize;
} RIFFCHUNK;

typedef struct
{
	unsigned short	usFormatTag;
	unsigned short	usChannels;
	unsigned long	ulSamplesPerSec;
	unsigned long	ulAvgBytesPerSec;
	unsigned short	usBlockAlign;
	unsigned short	usBitsPerSample;
	unsigned short	usSize;
	unsigned short  usReserved;
	unsigned long	ulChannelMask;
	GUID            guidSubFormat;
} WAVEFMT;

#pragma pack(pop)

WaveDecodec::WaveDecodec(void)
{
	m_bEnd = false;
	m_bPlayed = false;
	memset(&m_WaveIDs, 0, sizeof(m_WaveIDs));
	m_strFileName = NULL;
}


WaveDecodec::~WaveDecodec(void)
{
	Clean();	
}

WAVERESULT WaveDecodec::LoadWaveFile(const char *szFilename, WAVEID *pWaveID)
{
	WAVERESULT wr = WR_OUTOFMEMORY;
	LPWAVEFILEINFO pWaveInfo;

	pWaveInfo = new WAVEFILEINFO;
	if (pWaveInfo)
	{
		if (SUCCEEDED(wr = ParseFile(szFilename, pWaveInfo)))
		{
			// Allocate memory for sample data
			pWaveInfo->pData = new char[pWaveInfo->ulDataSize];
			if (pWaveInfo->pData)
			{
				// Seek to start of audio data
				fseek(pWaveInfo->pFile, pWaveInfo->ulDataOffset, SEEK_SET);

				// Read Sample Data
				if (fread(pWaveInfo->pData, 1, pWaveInfo->ulDataSize, pWaveInfo->pFile) == pWaveInfo->ulDataSize)
				{
					long lLoop = 0;
					for (lLoop = 0; lLoop < MAX_NUM_WAVEID; lLoop++)
					{
						if (!m_WaveIDs[lLoop])
						{
							m_WaveIDs[lLoop] = pWaveInfo;
							*pWaveID = lLoop;
							wr = WR_OK;
							break;
						}
					}

					if (lLoop == MAX_NUM_WAVEID)
					{
						delete pWaveInfo->pData;
						wr = WR_OUTOFMEMORY;
					}
				}
				else
				{
					delete pWaveInfo->pData;
					wr = WR_BADWAVEFILE;
				}
			}
			else
			{
				wr = WR_OUTOFMEMORY;
			}

			fclose(pWaveInfo->pFile);
			pWaveInfo->pFile = 0;
		}

		if (wr != WR_OK)
			delete pWaveInfo;
	}

	return wr;
}

WAVERESULT WaveDecodec::ParseFile(const char *szFilename, LPWAVEFILEINFO pWaveInfo)
{
	WAVEFILEHEADER	waveFileHeader;
	RIFFCHUNK		riffChunk;
	WAVEFMT			waveFmt;
	WAVERESULT		wr = WR_BADWAVEFILE;

	if (!szFilename || !pWaveInfo)
		return WR_INVALIDPARAM;

	memset(pWaveInfo, 0, sizeof(WAVEFILEINFO));

	// 打开WAVE文件
	pWaveInfo->pFile = fopen(szFilename, "rb");
	if (pWaveInfo->pFile)
	{
		// 读文件头
		fread(&waveFileHeader, 1, sizeof(WAVEFILEHEADER), pWaveInfo->pFile);
		if (!_strnicmp(waveFileHeader.szRIFF, "RIFF", 4) && !_strnicmp(waveFileHeader.szWAVE, "WAVE", 4))
		{
			while (fread(&riffChunk, 1, sizeof(RIFFCHUNK), pWaveInfo->pFile) == sizeof(RIFFCHUNK))
			{
				if (!_strnicmp(riffChunk.szChunkName, "fmt ", 4))
				{
					if (riffChunk.ulChunkSize <= sizeof(WAVEFMT))
					{
						fread(&waveFmt, 1, riffChunk.ulChunkSize, pWaveInfo->pFile);

						// 判断是类型为WAVEFORMATEX还是类型为WAVEFORMATEXTENSIBLE的WAVE文件
						if (waveFmt.usFormatTag == WAVE_FORMAT_PCM)
						{
							pWaveInfo->wfType = WF_EX;
							memcpy(&pWaveInfo->wfEXT.Format, &waveFmt, sizeof(PCMWAVEFORMAT));
						}
						else if (waveFmt.usFormatTag == WAVE_FORMAT_EXTENSIBLE)
						{
							pWaveInfo->wfType = WF_EXT;
							memcpy(&pWaveInfo->wfEXT, &waveFmt, sizeof(WAVEFORMATEXTENSIBLE));
						}
					}
					else
					{
						fseek(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
					}
				}
				else if (!_strnicmp(riffChunk.szChunkName, "data", 4))
				{
					pWaveInfo->ulDataSize = riffChunk.ulChunkSize;
					pWaveInfo->ulDataOffset = ftell(pWaveInfo->pFile);
					fseek(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
				}
				else
				{
					fseek(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
				}

				// 确保正确分配了下一个数据块
				if (riffChunk.ulChunkSize & 1)
					fseek(pWaveInfo->pFile, 1, SEEK_CUR);
			}

			if (pWaveInfo->ulDataSize && pWaveInfo->ulDataOffset && ((pWaveInfo->wfType == WF_EX) || (pWaveInfo->wfType == WF_EXT)))
				wr = WR_OK;
			else
				fclose(pWaveInfo->pFile);
		}
	}
	else
	{
		wr = WR_INVALIDFILENAME;
	}

	return wr;
}

WAVERESULT WaveDecodec::DeleteWaveFile(WAVEID WaveID)
{
	WAVERESULT wr = WR_OK;

	if (IsWaveID(WaveID))
	{
		if (m_WaveIDs[WaveID]->pData)
			delete m_WaveIDs[WaveID]->pData;

		if (m_WaveIDs[WaveID]->pFile)
			fclose(m_WaveIDs[WaveID]->pFile);

		delete m_WaveIDs[WaveID];
		m_WaveIDs[WaveID] = 0;
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

WAVERESULT WaveDecodec::GetWaveData(WAVEID WaveID, void **ppAudioData)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if ( !ppAudioData )
		return WR_INVALIDPARAM;

	*ppAudioData = m_WaveIDs[WaveID]->pData;

	return WR_OK;
}

WAVERESULT WaveDecodec::GetWaveSize(WAVEID WaveID, unsigned long *size)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (!size)
		return WR_INVALIDPARAM;

	*size = m_WaveIDs[WaveID]->ulDataSize;

	return WR_OK;
}

WAVERESULT WaveDecodec::GetWaveFrequency(WAVEID WaveID, unsigned long *pulFrequency)
{
	WAVERESULT wr = WR_OK;

	if (IsWaveID(WaveID))
	{
		if (pulFrequency)
			*pulFrequency = m_WaveIDs[WaveID]->wfEXT.Format.nSamplesPerSec;
		else
			wr = WR_INVALIDPARAM;
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

WAVERESULT WaveDecodec::GetWaveALBufferFormat(WAVEID WaveID, PFNALGETENUMVALUE pfnGetEnumValue, unsigned long *pulFormat)
{
	WAVERESULT wr = WR_OK;

	if (IsWaveID(WaveID))
	{
		if (pfnGetEnumValue && pulFormat)
		{
			*pulFormat = 0;

			if (m_WaveIDs[WaveID]->wfType == WF_EX)
			{
				if (m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 1)
					*pulFormat = m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16 ? pfnGetEnumValue("AL_FORMAT_MONO16") : pfnGetEnumValue("AL_FORMAT_MONO8");
				else if (m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 2)
					*pulFormat = m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16 ? pfnGetEnumValue("AL_FORMAT_STEREO16") : pfnGetEnumValue("AL_FORMAT_STEREO8");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 4) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_QUAD16");
			}
			else if (m_WaveIDs[WaveID]->wfType == WF_EXT)
			{
				if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 1) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == SPEAKER_FRONT_CENTER))
					*pulFormat = m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16 ? pfnGetEnumValue("AL_FORMAT_MONO16") : pfnGetEnumValue("AL_FORMAT_MONO8");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 2) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)))
					*pulFormat = m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16 ? pfnGetEnumValue("AL_FORMAT_STEREO16") : pfnGetEnumValue("AL_FORMAT_STEREO8");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 2) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					*pulFormat =  pfnGetEnumValue("AL_FORMAT_REAR16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 4) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_QUAD16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 6) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_51CHN16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 7) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_BACK_CENTER)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_61CHN16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 8) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_71CHN16");
			}

			if (*pulFormat == 0)
				wr = WR_INVALIDWAVEFILETYPE;
		}
		else
		{
			wr = WR_INVALIDPARAM;
		}
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

bool WaveDecodec::IsWaveID(WAVEID WaveID)
{
	bool bReturn = false;

	if ((WaveID >= 0) && (WaveID < MAX_NUM_WAVEID))
	{
		if (m_WaveIDs[WaveID])
			bReturn = true;
	}

	return bReturn;
}

HRESULT WaveDecodec::LoadMusicFromFile(const TCHAR* lpszFileName)
{
	CString strFileName = lpszFileName;

	Clean();

	WAVEID WaveID;

	alGenSources( 1, &m_uiSource );
	alGenBuffers( 1, &m_uiBuffer );
	
	int len = WideCharToMultiByte( CP_ACP, 0, (LPCTSTR)strFileName, -1, NULL, 0, NULL, NULL );
	char* pFileName = new char[len];
	WideCharToMultiByte( CP_ACP, 0, (LPCTSTR)strFileName, -1, pFileName, len, NULL, NULL );
	m_strFileName = SysAllocString( (LPCTSTR)strFileName );

	if( LoadWaveFile( pFileName, &WaveID ) != WR_OK )
	{
		delete[] pFileName;
		return S_FALSE;
	}
	delete[] pFileName;

	GetWaveSize( WaveID, (unsigned long*)&m_ulBufferSize );
	GetWaveData( WaveID, (void**)&m_pDecodeBuffer );
	GetWaveFrequency(WaveID, (unsigned long*)&m_ulFrequency );
	GetWaveALBufferFormat( WaveID, &alGetEnumValue, (unsigned long*)&m_BufferFormat );

	alBufferData( m_uiBuffer, m_BufferFormat, m_pDecodeBuffer, m_ulBufferSize, m_ulFrequency );
	DeleteWaveFile( WaveID );

	alSourcei( m_uiSource, AL_BUFFER, m_uiBuffer );
	alSourcef( m_uiSource, AL_PITCH, 1.0f );
	alSourcef( m_uiSource, AL_GAIN, 1.0f );

	return S_OK;
}

HRESULT WaveDecodec::Pause()
{
	ALint iState = 0;
	alGetSourcei( m_uiSource, AL_SOURCE_STATE, &iState );
	if( iState != AL_PAUSED )
		alSourcePause( m_uiSource );

	return S_OK;
}

HRESULT WaveDecodec::Play()
{
	ALint iState = 0;
	alGetSourcei( m_uiSource, AL_SOURCE_STATE, &iState );
	if( iState != AL_PLAYING )
	{
		alSourcei( m_uiSource, AL_LOOPING, AL_FALSE );

		if( m_bPlayed && iState == AL_STOPPED )
			m_bEnd = true;
		
		
		if( !m_bPlayed )
		{
			alSourcePlay( m_uiSource );	
			m_bPlayed = true;
		}

		if( iState == AL_PAUSED )
			alSourcePlay( m_uiSource );
	}
	
	return S_OK;
}

HRESULT WaveDecodec::Replay()
{
	if( m_strFileName )
		return LoadMusicFromFile( m_strFileName );

	return S_FALSE;
}

HRESULT WaveDecodec::Stop()
{
	ALint iState = 0;
	alGetSourcei( m_uiSource, AL_SOURCE_STATE, &iState );
	if( iState != AL_STOPPED )
		alSourceStop( m_uiSource );

	m_bEnd = true;

	return S_OK;
}

HRESULT WaveDecodec::Clean()
{
	alDeleteSources( 1, &m_uiSource );
	alDeleteBuffers( 1, &m_uiBuffer );

	for ( int iLoop = 0; iLoop < MAX_NUM_WAVEID; ++iLoop )
	{
		if( m_WaveIDs[iLoop] )
		{
			if (m_WaveIDs[iLoop]->pData)
				delete m_WaveIDs[iLoop]->pData;

			if (m_WaveIDs[iLoop]->pFile)
				fclose( m_WaveIDs[iLoop]->pFile );

			delete m_WaveIDs[iLoop];
			m_WaveIDs[iLoop] = NULL;
		}
	}
	m_bEnd = false;
	m_bPlayed = false;
	if( m_strFileName )
		SysFreeString( m_strFileName );
	m_strFileName = NULL;

	return S_OK;
}

HRESULT WaveDecodec::IsEnd( VARIANT_BOOL* pbEnd )
{
	if( m_bEnd )
		*pbEnd = VARIANT_TRUE;
	else
		*pbEnd = VARIANT_FALSE;

	return S_OK;
}

HRESULT WaveDecodec::GetPlayingProcess(FLOAT* pVal)
{
	return S_OK;
}

HRESULT WaveDecodec::SetPlayingProcess(FLOAT newVal)
{
	return S_OK;
}

HRESULT WaveDecodec::GetVolume(FLOAT* pVal)
{
	alGetListenerf( AL_GAIN, pVal );

	return S_OK;
}

HRESULT WaveDecodec::SetVolume(FLOAT newVal)
{
	if( newVal >= 0.0 && newVal <= 1.0f )
		alListenerf( AL_GAIN, newVal );
	else
		return S_FALSE;

	return S_OK;
}