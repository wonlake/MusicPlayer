#pragma once

#include <windows.h>

class Decodec
{
public:
	virtual HRESULT LoadMusicFromFile( const TCHAR* lpszFileName ) = 0;

	virtual HRESULT Play() = 0;

	virtual	HRESULT Replay() = 0;

	virtual HRESULT Stop() = 0;

	virtual HRESULT Clean() = 0;

	virtual HRESULT IsEnd( VARIANT_BOOL* pbEnd ) = 0;

	virtual HRESULT GetPlayingProcess(FLOAT* pVal) = 0;

	virtual HRESULT SetPlayingProcess(FLOAT newVal) = 0;

	virtual HRESULT GetVolume(FLOAT* pVal) = 0;

	virtual HRESULT SetVolume(FLOAT newVal) = 0;

	virtual HRESULT Pause() = 0;

public:
	virtual ~Decodec()
	{

	}
};

struct STREAMINFO
{
	long iPrimDataSize;
	long iRawDataSize;
};