////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particleexporter.cpp
//  Version:     v1.00
//  Created:     12/9/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ParticleExporter.h"

#include "Util\PakFile.h"

#include "Particles\ParticleItem.h"
#include "Particles\ParticleLibrary.h"
#include "Particles\ParticleManager.h"

//////////////////////////////////////////////////////////////////////////
// Signatures for particles.lst file.
#define PARTICLES_FILE_TYPE 2
#define PARTICLES_FILE_VERSION 4
#define PARTICLES_FILE_SIGNATURE "CRY"

#define PARTICLES_FILE "particles.lst"

#define EFFECTS_PAK_FILE "FCData\\Effects.pak"
#define EFFECTS_FOLDER "Effects\\"

#pragma pack(push,1)
//////////////////////////////////////////////////////////////////////////
struct SExportedParticlesHeader
{
	char signature[3];	// File signature.
	int filetype;				// File type.
	int	version;				// File version.
};

struct SExportParticleSound
{
	enum {
		LOOP = 0x01,
		EVERY_SPAWN = 0x02,
	};
	char soundfile[64];
	float volume;
	float minRadius;
	float maxRadius;
	char nSoundFlags;
};

enum EParticleExportFlags
{
	PARTICLE_EFFECT_DISABLED = 0x01
};

//////////////////////////////////////////////////////////////////////////

//! Particle system parameters
struct SExportParticleParams
{
	Vec3 vPosition; // spawn position
	Vec3 vDirection; // initial direction  (normalization not important)
	float fFocus; // if 0 - particles go in all directions, if more than 20 - particles go mostly in vDirection
	Vec3 vColorStart; // initial color
	Vec3 vColorEnd; // final color
	FloatVariant fSpeed; // initial speed ( +- 25% random factor applyed, m/sec )
	float fSpeedFadeOut; // Time in which before end of life time speed decreases from normal to 0.
	float fSpeedAccel;	// Constant speed acceleration along particle heading.
	float fAirResistance;	// Coefficient of Air Resistance.
	Vec3Variant vRotation; // rotation speed (degree/sec)
	Vec3Variant vInitAngles; // initial rotation
	int   nCount; // number of particles to spawn
	FloatVariant fSize; // initial size of particles
	float fSizeSpeed; // particles will grow with this speed
	float fSizeFadeIn; // Time in which at the begning of life time size goes from 0 to fSize.
	float fSizeFadeOut; // Time in which at the end of life time size goes from fSize to 0.
	float fThickness;	// lying thickness - for physicalized particles only
	FloatVariant fLifeTime; // time of life of particle
	float fFadeInTime; // particle will fade in slowly during this time
	int   nTexAnimFramesCount; // number of frames in animated texture ( 0 if no animation )
	ParticleBlendType eBlendType; // see ParticleBlendType
	float fTailLenght; // delay of tail ( 0 - no tail, 1 meter if speed is 1 meter/sec )
	float fStretch; // Stretch particles into moving direction.
	int   nParticleFlags; // see particle system flags
	bool  bRealPhysics; // use physics engine to control particles
	float fChildSpawnPeriod; // if more than 0 - run child process every x seconds, if 0 - run it at collision
	float fChildSpawnTime; // if more then 0, Spawn child process for max this ammount of time.
	int   nDrawLast; // add this element into second list and draw this list last
	float fBouncenes; // if 0 - particle will not bounce from the ground, 0.5 is good in most cases
	float  fTurbulenceSize; // radius of turbulence
	float  fTurbulenceSpeed; // speed of rotation
	float fDirVecScale; //the game need to store this(Alberto)
	float fPosRandomOffset; // maximum distance of random offset from original position

	//////////////////////////////////////////////////////////////////////////
	// New parameters, used by Particle effects.
	//////////////////////////////////////////////////////////////////////////
	//! Spawn Position offset from effect spawn position.
	Vec3 vPositionOffset;
	//! Random offset of particle relative to spawn position.
	Vec3 vRandomPositionOffset;
	//! Delay actual spawn time by this ammount.
	FloatVariant fSpawnDelay;
	//! Life time of emitter.
	FloatVariant fEmitterLifeTime;
	//! When using emitter, spawn time between between 2 particle bursts.
	float fSpawnPeriod;

	//! Global effect scale. (0 ignored)
	float fScale;
	//! Object scale, multiplied with fSize to give scale adjustment between object and texture.
	//! 0 not affect fSize.
	float fObjectScale;

	Vec3 vNormal; // lying normal - for physicalized particles only
	int iPhysMat; // material for physicalized particles
	Vec3 vGravity; // gravity(wind) vector

	//////////////////////////////////////////////////////////////////////////
	unsigned short nTailSteps;
	//////////////////////////////////////////////////////////////////////////
	// Reserve space for new members.
	//////////////////////////////////////////////////////////////////////////
	char reserve[126];
};

struct SExportParticleEffect
{
	char name[64];
	char texture[2][64];
	char geometry[2][64];
	char material[2][64];
	SExportParticleParams params[2]; // Primary and Child params.
	SExportParticleSound sound;
	int parent;	// Index of parent particle.
	int flags; // General flags.
};
#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////
#define PATICLEPARAMS_COPY_HELPER( inparam,outparam,name ) outparam.name = inparam.name;

//////////////////////////////////////////////////////////////////////////
static void ParticleParamsToExportData( const ParticleParams &inp,SExportParticleParams &outp )
{
	PATICLEPARAMS_COPY_HELPER( inp,outp,vPosition );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vDirection );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fFocus );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vColorStart );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vColorEnd );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSpeed );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSpeedFadeOut );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSpeedAccel );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fAirResistance );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vRotation );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vInitAngles );
	PATICLEPARAMS_COPY_HELPER( inp,outp,nCount );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSize );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSizeSpeed );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSizeFadeIn );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSizeFadeOut );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fThickness );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fLifeTime );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fFadeInTime );
	PATICLEPARAMS_COPY_HELPER( inp,outp,nTexAnimFramesCount );
	PATICLEPARAMS_COPY_HELPER( inp,outp,eBlendType );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fTailLenght );
	PATICLEPARAMS_COPY_HELPER( inp,outp,nTailSteps );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fStretch );
	PATICLEPARAMS_COPY_HELPER( inp,outp,nParticleFlags );
	PATICLEPARAMS_COPY_HELPER( inp,outp,bRealPhysics );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fChildSpawnPeriod );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fChildSpawnTime );
	PATICLEPARAMS_COPY_HELPER( inp,outp,nDrawLast );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fBouncenes );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fTurbulenceSize );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fTurbulenceSpeed );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fDirVecScale );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fPosRandomOffset );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vPositionOffset );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vRandomPositionOffset );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSpawnDelay );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fEmitterLifeTime );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fSpawnPeriod );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fScale );
	PATICLEPARAMS_COPY_HELPER( inp,outp,fObjectScale );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vNormal );
	PATICLEPARAMS_COPY_HELPER( inp,outp,iPhysMat );
	PATICLEPARAMS_COPY_HELPER( inp,outp,vGravity );
}

//////////////////////////////////////////////////////////////////////////
void CParticlesExporter::AddParticleExportItem( std::vector<SExportParticleEffect> &effects,CParticleItem *pItem,int parent )
{
	SExportParticleEffect exportData;
	ZeroStruct(exportData);

	exportData.parent = parent;

	strncpy( exportData.name,pItem->GetFullName(),sizeof(exportData.name) );
	exportData.name[sizeof(exportData.name)-1] = 0;

	IParticleEffect *pEffect = pItem->GetEffect();
	if (!pEffect)
		return;
	for (int p = 0; p < IParticleEffect::NUM_PARTICLE_PROCESSES; p++)
	{
		ParticleParamsToExportData( pEffect->GetParticleParams(p),exportData.params[p] );

		strncpy( exportData.texture[p],pEffect->GetTexture(p),sizeof(exportData.texture[0])-1 );
		strncpy( exportData.geometry[p],pEffect->GetGeometry(p),sizeof(exportData.geometry[0])-1 );
		if (pEffect->GetMaterial(p))
		{
			strncpy( exportData.material[p],pEffect->GetMaterial(p)->GetName(),sizeof(exportData.material[0])-1 );
		}
	}

	if (!pEffect->IsEnabled())
		exportData.flags |= PARTICLE_EFFECT_DISABLED;

	IParticleEffect::SoundParams sndParams;
	pEffect->GetSoundParams(sndParams);
	strncpy( exportData.sound.soundfile,sndParams.szSound,sizeof(exportData.sound.soundfile) );
	exportData.sound.soundfile[sizeof(exportData.sound.soundfile)-1] = 0;
	exportData.sound.volume = sndParams.volume;
	exportData.sound.minRadius = sndParams.minRadius;
	exportData.sound.maxRadius = sndParams.maxRadius;
	exportData.sound.nSoundFlags = 0;
	exportData.sound.nSoundFlags |= (sndParams.bLoop) ? SExportParticleSound::LOOP : 0;
	exportData.sound.nSoundFlags |= (sndParams.bOnEverySpawn) ? SExportParticleSound::EVERY_SPAWN : 0;

	effects.push_back( exportData );
	int currentId = effects.size()-1;
	for (int i = 0; i < pItem->GetChildCount(); i++)
	{
		AddParticleExportItem( effects,pItem->GetChild(i),currentId );
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticlesExporter::ExportParticleLib( CParticleLibrary *pLib,CFile &file )
{
	std::vector<SExportParticleEffect> effects;
	effects.reserve( pLib->GetItemCount() );

	int i;
	for (i = 0; i < pLib->GetItemCount(); i++)
	{
		CParticleItem *pItem = (CParticleItem*)pLib->GetItem(i);
		if (!pItem->GetParent())
			AddParticleExportItem( effects,pItem,-1 );
	}

	//////////////////////////////////////////////////////////////////////////
	// Write particles file header.
	//////////////////////////////////////////////////////////////////////////
	SExportedParticlesHeader header;
	memcpy( header.signature,PARTICLES_FILE_SIGNATURE,3 );
	header.filetype = PARTICLES_FILE_TYPE;
	header.version = PARTICLES_FILE_VERSION;
	file.Write( &header,sizeof(header) );

	//////////////////////////////////////////////////////////////////////////
	// Write particles data.
	//////////////////////////////////////////////////////////////////////////
	int numItems = effects.size();
	// Write number of particles in file.
	file.Write( &numItems,sizeof(numItems) );
	// Write actual particles data.
	if (!effects.empty())
		file.Write( &effects[0],numItems*sizeof(SExportParticleEffect) );
}

//////////////////////////////////////////////////////////////////////////
void CParticlesExporter::ExportParticles( const CString &path,const CString &levelName,CPakFile &levelPakFile )
{
	CParticleManager *pPartManager = GetIEditor()->GetParticleManager();
	if (pPartManager->GetLibraryCount() == 0)
		return;

	ISystem *pISystem = GetIEditor()->GetSystem();

	// If have more then onle library save them into shared Effects.pak
	bool bNeedEffectsPak = pPartManager->GetLibraryCount() > 1;

	bool bEffectsPak = true;

	CString pakFilename = EFFECTS_PAK_FILE;
	CPakFile effectsPak;

	if (bNeedEffectsPak)
	{
		// Close main game pak file if open.
		if (!pISystem->GetIPak()->ClosePack( pakFilename ))
		{
			CLogFile::FormatLine( "Cannot close Pak file %s",(const char*)pakFilename );
			bEffectsPak = false;
		}

		//////////////////////////////////////////////////////////////////////////
		if (CFileUtil::OverwriteFile(pakFilename))
		{
			// Delete old pak file.
			if (!effectsPak.Open( pakFilename,false ))
			{
				CLogFile::FormatLine( "Cannot open Pak file for Writing %s",(const char*)pakFilename );
				bEffectsPak = false;
			}
		}
		else
			bEffectsPak = false;
	}

	// Effects pack.
	int i;
	for (i = 0; i < pPartManager->GetLibraryCount(); i++)
	{
		CParticleLibrary *pLib = (CParticleLibrary*)pPartManager->GetLibrary(i);
		if (pLib->IsLevelLibrary())
		{
			CMemFile file;
			ExportParticleLib( pLib,file );
			CString filename = Path::Make( path,PARTICLES_FILE );
			levelPakFile.UpdateFile( filename,file );
		}
		else
		{
			if (bEffectsPak)
			{
				CMemFile file;
				CString filename = Path::Make( EFFECTS_FOLDER,pLib->GetName() + ".prt" );
				ExportParticleLib( pLib,file );
				effectsPak.UpdateFile( filename,file );
			}

			/*
			bool bNotExist = false;
			CString filename = Path::Make( EFFECTS_FOLDER,pLib->GetName() + ".prt" );
			{
			CFile tempFile;
			if (!tempFile.Open( filename,CFile::modeRead ))
			{
			bNotExist = true;
			}
			}
			if (pLib->IsModified() || bNotExist)
			{
			if (!CFileUtil::OverwriteFile(filename))
			return;

			CFile file;
			if (file.Open( filename,CFile::modeCreate|CFile::modeWrite ))
			{
			ExportParticleLib( pLib,file );
			}
			}
			*/
		}
	}

	// Open Pak, which was closed before.
	pISystem->GetIPak()->OpenPack( pakFilename );
}