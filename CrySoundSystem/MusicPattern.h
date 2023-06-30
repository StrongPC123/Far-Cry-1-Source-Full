#pragma once

#include <vector>
#include "MusicPatternInstance.h"

struct IMusicSystem;
struct SMusicPatternFileInfo;
struct IMusicPatternDecoder;

typedef std::vector<int>			TMarkerVec;
typedef TMarkerVec::iterator	TMarkerVecIt;

class CMusicPattern
{
protected:
	IMusicSystem *m_pMusicSystem;
	IMusicPatternDecoder *m_pDecoder;
	string m_sName;
	string m_sFilename;
	TMarkerVec m_vecFadePoints;
	int m_nLayeringVolume;
	friend class CMusicPatternInstance;
	int m_numPatternInstances;
	int m_nSamples; //!< Number of Samples in pattern.
private:
	IMusicPatternDecoderInstance* CreateDecoderInstance();
public:
	CMusicPattern(IMusicSystem *pMusicSystem, const char *pszName,const char *pszFilename);
	~CMusicPattern();
	bool Open(const char *pszFilename);
	bool Close();
	bool AddFadePoint(int nFadePos);
	void ClearFadePoints();
	void SetLayeringVolume(int nVol) { m_nLayeringVolume=nVol; }
	int GetLayeringVolume() { return m_nLayeringVolume; }
	CMusicPatternInstance* CreateInstance();
	void ReleaseInstance( CMusicPatternInstance* pInstance );
	void SetFilename( const char *sFilename ) { m_sFilename = sFilename; };
	const char* GetName() { return m_sName.c_str(); }
	const char* GetFilename() { return m_sFilename.c_str(); }
	bool GetFileInfo(SMusicPatternFileInfo &FileInfo);
	void GetMemoryUsage(class ICrySizer* pSizer);
};