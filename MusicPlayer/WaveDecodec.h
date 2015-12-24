#pragma once

#include "Decodec.h"

#define MAX_NUM_WAVEID			1024

enum WAVEFILETYPE
{
	WF_EX  = 1,
	WF_EXT = 2
};

enum WAVERESULT
{
	WR_OK = 0,
	WR_INVALIDFILENAME					= - 1,
	WR_BADWAVEFILE						= - 2,
	WR_INVALIDPARAM						= - 3,
	WR_INVALIDWAVEID					= - 4,
	WR_NOTSUPPORTEDYET					= - 5,
	WR_WAVEMUSTBEMONO					= - 6,
	WR_WAVEMUSTBEWAVEFORMATPCM			= - 7,
	WR_WAVESMUSTHAVESAMEBITRESOLUTION	= - 8,
	WR_WAVESMUSTHAVESAMEFREQUENCY		= - 9,
	WR_WAVESMUSTHAVESAMEBITRATE			= -10,
	WR_WAVESMUSTHAVESAMEBLOCKALIGNMENT	= -11,
	WR_OFFSETOUTOFDATARANGE				= -12,
	WR_FILEERROR						= -13,
	WR_OUTOFMEMORY						= -14,
	WR_INVALIDSPEAKERPOS				= -15,
	WR_INVALIDWAVEFILETYPE				= -16,
	WR_NOTWAVEFORMATEXTENSIBLEFORMAT	= -17
};

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
	WORD    wFormatTag;
	WORD    nChannels;
	DWORD   nSamplesPerSec;
	DWORD   nAvgBytesPerSec;
	WORD    nBlockAlign;
	WORD    wBitsPerSample;
	WORD    cbSize;
} WAVEFORMATEX;
#endif /* _WAVEFORMATEX_ */

#ifndef _WAVEFORMATEXTENSIBLE_
#define _WAVEFORMATEXTENSIBLE_
typedef struct {
	WAVEFORMATEX    Format;
	union {
		WORD wValidBitsPerSample;       /* bits of precision  */
		WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
		WORD wReserved;                 /* If neither applies, set to zero. */
	} Samples;
	DWORD           dwChannelMask;      /* which channels are */
	/* present in stream  */
	GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
#endif // !_WAVEFORMATEXTENSIBLE_

typedef struct
{
	WAVEFILETYPE	wfType;
	WAVEFORMATEXTENSIBLE wfEXT;		// For non-WAVEFORMATEXTENSIBLE wavefiles, the header is stored in the Format member of wfEXT
	char			*pData;
	unsigned long	ulDataSize;
	FILE			*pFile;
	unsigned long	ulDataOffset;
} WAVEFILEINFO, *LPWAVEFILEINFO;

typedef int (__cdecl *PFNALGETENUMVALUE)( const char *szEnumName );
typedef int	WAVEID;

class WaveDecodec : public Decodec
{
public:
	WaveDecodec(void);
	~WaveDecodec(void);

private:
	WAVERESULT LoadWaveFile(const char *szFilename, WAVEID *WaveID);
	WAVERESULT GetWaveData(WAVEID WaveID, void **ppAudioData);
	WAVERESULT GetWaveSize(WAVEID WaveID, unsigned long *pulDataSize);
	WAVERESULT GetWaveFrequency(WAVEID WaveID, unsigned long *pulFrequency);
	WAVERESULT GetWaveALBufferFormat(WAVEID WaveID, PFNALGETENUMVALUE pfnGetEnumValue, unsigned long *pulFormat);
	WAVERESULT DeleteWaveFile(WAVEID WaveID);
	bool IsWaveID(WAVEID WaveID);

private:
	void*		  m_pDecodeBuffer;
	ALenum		  m_BufferFormat;
	unsigned long m_ulFrequency;
	unsigned long m_ulBufferSize;

	ALuint		  m_uiSource;
	ALuint		  m_uiBuffer;
	bool          m_bEnd;
	bool		  m_bPlayed;
	BSTR		  m_strFileName;

private:
	WAVERESULT ParseFile(const char *szFilename, LPWAVEFILEINFO pWaveInfo);
	WAVEID InsertWaveID(LPWAVEFILEINFO pWaveFileInfo);

	LPWAVEFILEINFO	m_WaveIDs[MAX_NUM_WAVEID];

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

