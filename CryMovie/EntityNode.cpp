////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   entitynode.cpp
//  Version:     v1.00
//  Created:     23/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EntityNode.h"
#include "EventTrack.h"
#include "CharacterTrack.h"
#include "AnimSplineTrack.h"
#include "ExprTrack.h"
#include "BoolTrack.h"
#include "isystem.h"
#include "ilog.h"
#include "Movie.h"

#include <ISound.h>
#include <ILipSync.h>
#include <ICryAnimation.h>
#include <CryCharMorphParams.h>
#include <IGame.h>

//////////////////////////////////////////////////////////////////////////
namespace
{
	//////////////////////////////////////////////////////////////////////////
	Matrix44 Vector2Matrix( const Vec3 &dir,const Vec3 &up,float rollAngle=0 )
	{
		Matrix44 M;
		// LookAt transform.
		Vec3d xAxis,yAxis,zAxis;
		Vec3d upVector = up;

		yAxis = GetNormalized(-dir);

		//if (zAxis.x == 0.0 && zAxis.z == 0)	up.Set( -zAxis.y,0,0 );	else up.Set( 0,1.0f,0 );

		xAxis = GetNormalized((upVector.Cross(yAxis)));
		zAxis = GetNormalized(xAxis.Cross(yAxis));

		// OpenGL kind of matrix.
		M[0][0] = xAxis.x;
		M[1][0] = yAxis.x;
		M[2][0] = zAxis.x;
		M[3][0] = 0;

		M[0][1] = xAxis.y;
		M[1][1] = yAxis.y;
		M[2][1] = zAxis.y;
		M[3][1] = 0;

		M[0][2] = xAxis.z;
		M[1][2] = yAxis.z;
		M[2][2] = zAxis.z;
		M[3][2] = 0;

		M[0][3] = 0;
		M[1][3] = 0;
		M[2][3] = 0;
		M[3][3] = 1;

		if (rollAngle != 0)
		{
			Matrix44 RollMtx;
			RollMtx.SetIdentity();

			float s = cry_sinf(rollAngle);
			float c = cry_cosf(rollAngle);

			RollMtx[0][0] = c; RollMtx[2][0] = -s;;
			RollMtx[0][2] = s; RollMtx[2][2] = c;

			// Matrix multiply.
			M = RollMtx * M;
		}

		return M;
	}

		
	bool s_nodeParamsInitialized = false;
	std::vector<IAnimNode::SParamInfo> s_nodeParams;

	void AddSupportedParam( std::vector<IAnimNode::SParamInfo> &nodeParams,const char *sName,int paramId,EAnimValue valueType )
	{
		IAnimNode::SParamInfo param;
		param.name = sName;
		param.paramId = paramId;
		param.valueType = valueType;
		nodeParams.push_back( param );
	}
};

//////////////////////////////////////////////////////////////////////////
CAnimEntityNode::CAnimEntityNode( IMovieSystem *sys )
: CAnimNode(sys)
{
	m_dwSupportedTracks = PARAM_BIT(APARAM_POS)|PARAM_BIT(APARAM_ROT)|PARAM_BIT(APARAM_SCL)|	
												PARAM_BIT(APARAM_EVENT)|PARAM_BIT(APARAM_VISIBLE)|
												PARAM_BIT(APARAM_SOUND1)|PARAM_BIT(APARAM_SOUND2)|PARAM_BIT(APARAM_SOUND3)|
												PARAM_BIT(APARAM_CHARACTER1)|PARAM_BIT(APARAM_CHARACTER2)|PARAM_BIT(APARAM_CHARACTER3)|
												PARAM_BIT(APARAM_EXPRESSION1)|PARAM_BIT(APARAM_EXPRESSION2)|PARAM_BIT(APARAM_EXPRESSION3);

	m_entity = 0;
	m_EntityId = 0;
	m_target=NULL;
	m_pMovie=sys;
	m_bMatrixValid = 0;
	m_bMatrixInWorldSpace = 0;
	m_worldTM.SetIdentity();
	m_pos(0,0,0);
	m_scale(1,1,1);
	m_visible = true;

	m_lastEntityKey = -1;
	m_lastCharacterKey[0] = -1;
	m_lastCharacterKey[1] = -1;
	m_lastCharacterKey[2] = -1;

	for (int i=0;i<ENTITY_SOUNDTRACKS;i++)
	{
		m_SoundInfo[i].nLastKey=-1;
		m_SoundInfo[i].pSound=NULL;
		m_SoundInfo[i].sLastFilename="";
	}

	if (!s_nodeParamsInitialized)
	{
		s_nodeParamsInitialized = true;
		AddSupportedParams( s_nodeParams );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::AddSupportedParams( std::vector<IAnimNode::SParamInfo> &nodeParams )
{
	AddSupportedParam( nodeParams,"Position",APARAM_POS,AVALUE_VECTOR );
	AddSupportedParam( nodeParams,"Rotation",APARAM_ROT,AVALUE_QUAT );
	AddSupportedParam( nodeParams,"Scale",APARAM_SCL,AVALUE_VECTOR );
	AddSupportedParam( nodeParams,"Visibility",APARAM_VISIBLE,AVALUE_BOOL );
	AddSupportedParam( nodeParams,"Event",APARAM_EVENT,AVALUE_EVENT );
	AddSupportedParam( nodeParams,"Sound1",APARAM_SOUND1,AVALUE_SOUND );
	AddSupportedParam( nodeParams,"Sound2",APARAM_SOUND2,AVALUE_SOUND );
	AddSupportedParam( nodeParams,"Sound3",APARAM_SOUND3,AVALUE_SOUND );
	AddSupportedParam( nodeParams,"Animation1",APARAM_CHARACTER1,AVALUE_CHARACTER );
	AddSupportedParam( nodeParams,"Animation2",APARAM_CHARACTER2,AVALUE_CHARACTER );
	AddSupportedParam( nodeParams,"Animation3",APARAM_CHARACTER3,AVALUE_CHARACTER );
	AddSupportedParam( nodeParams,"Expression1",APARAM_EXPRESSION1,AVALUE_EXPRESSION );
	AddSupportedParam( nodeParams,"Expression2",APARAM_EXPRESSION2,AVALUE_EXPRESSION );
	AddSupportedParam( nodeParams,"Expression3",APARAM_EXPRESSION3,AVALUE_EXPRESSION );
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::CreateDefaultTracks()
{
	CreateTrack(APARAM_POS);
	CreateTrack(APARAM_ROT);
};

//////////////////////////////////////////////////////////////////////////
CAnimEntityNode::~CAnimEntityNode()
{
	ReleaseSounds();
}

//////////////////////////////////////////////////////////////////////////
int CAnimEntityNode::GetParamCount() const
{
	return s_nodeParams.size();
}

//////////////////////////////////////////////////////////////////////////
bool CAnimEntityNode::GetParamInfo( int nIndex, SParamInfo &info ) const
{
	if (nIndex >= 0 && nIndex < s_nodeParams.size())
	{
		info = s_nodeParams[nIndex];
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimEntityNode::GetParamInfoFromId( int paramId, SParamInfo &info ) const
{
	for (int i = 0; i < s_nodeParams.size(); i++)
	{
		if (s_nodeParams[i].paramId == paramId)
		{
			info = s_nodeParams[i];
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::SetEntity( IEntity *entity )
{
	m_entity = entity;
	if (m_entity)
		m_EntityId = m_entity->GetId();
	else
		m_EntityId = 0;
}

//////////////////////////////////////////////////////////////////////////
IEntity* CAnimEntityNode::GetEntity()
{
	if (!m_entity)
		ResolveEntity();
	return m_entity;
}
/*
//////////////////////////////////////////////////////////////////////////
IAnimBlock* CAnimEntityNode::CreateAnimBlock()
{
	// Assign entity tracks to this anim node.
	IAnimBlock *block = new CAnimBlock;
	CreateTrack(block, APARAM_POS);
	CreateTrack(block, APARAM_ROT);
	return block;
};*/

//////////////////////////////////////////////////////////////////////////
bool CAnimEntityNode::ResolveEntity()
{
	IEntitySystem *pEntitySystem=GetMovieSystem()->GetSystem()->GetIEntitySystem();
	if (!pEntitySystem)
		return false;
	m_entity = pEntitySystem->GetEntity(m_EntityId);
	return (m_entity != NULL);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::Animate( SAnimContext &ec )
{
	CAnimBlock *anim = (CAnimBlock*)GetAnimBlock();
	if (!anim)
		return;
	
	if (!m_entity)
	{
		if (!ResolveEntity())
			return;
	}

	Vec3 pos = m_pos;
	Quat rotate = m_rotate;
	Vec3 scale = m_scale;
	
	IAnimTrack *posTrack = NULL;
	IAnimTrack *rotTrack = NULL;
	IAnimTrack *sclTrack = NULL;

	if (!ec.bResetting)
	{
		StopExpressions();
	}

	int paramCount = anim->GetTrackCount();
	for (int paramIndex = 0; paramIndex < paramCount; paramIndex++)
	{
		int trackType;
		IAnimTrack *pTrack;
		if (!anim->GetTrackInfo( paramIndex,trackType,&pTrack ))
			continue;
		switch (trackType)
		{
		case APARAM_POS:
			posTrack = pTrack;
			posTrack->GetValue( ec.time,pos );
			break;
		case APARAM_ROT:
			rotTrack = pTrack;
			rotTrack->GetValue( ec.time,rotate );
			break;
		case APARAM_SCL:
			sclTrack = pTrack;
			sclTrack->GetValue( ec.time,scale );
			break;
		case APARAM_EVENT:
			if (!ec.bResetting)
			{
				CEventTrack *entityTrack = (CEventTrack*)pTrack;
				IEventKey key;
				int entityKey = entityTrack->GetActiveKey(ec.time,&key);
				// If key is different or if time is standing exactly on key time.
				//if ((entityKey != m_lastEntityKey || key.time == ec.time) && (!ec.bSingleFrame))
				if (entityKey != m_lastEntityKey || key.time == ec.time)
				{
					m_lastEntityKey = entityKey;
					if (entityKey >= 0)
					{
						if (!ec.bSingleFrame || key.time == ec.time) // If Single frame update key time must match current time.
							ApplyEventKey( entityTrack,entityKey,key );
					}
				}
			}
			break;
		case APARAM_VISIBLE:
			if (!ec.bResetting)
			{
				IAnimTrack *visTrack = pTrack;
				bool visible = m_visible;
				visTrack->GetValue( ec.time,visible );
				m_entity->Hide(!visible);
				m_visible = visible;
			}
			break;

		//////////////////////////////////////////////////////////////////////////
		case APARAM_SOUND1:
		case APARAM_SOUND2:
		case APARAM_SOUND3:
			if (!ec.bResetting)
			{
				int nSoundIndex = trackType - APARAM_SOUND1;
				CSoundTrack *pSoundTrack = (CSoundTrack*)pTrack;
				ISoundKey key;
				int nSoundKey = pSoundTrack->GetActiveKey(ec.time, &key);
				if (nSoundKey!=m_SoundInfo[nSoundIndex].nLastKey || key.time==ec.time || nSoundKey==-1)
				{
					if (!ec.bSingleFrame || key.time == ec.time) // If Single frame update key time must match current time.
					{
						m_SoundInfo[nSoundIndex].nLastKey=nSoundKey;
						ApplySoundKey( pSoundTrack,nSoundKey,nSoundIndex, key, ec);
					}
				}
			}
			break;
		
		//////////////////////////////////////////////////////////////////////////
		case APARAM_CHARACTER1:
		case APARAM_CHARACTER2:
		case APARAM_CHARACTER3:
			if (!ec.bResetting)
			{
				int index = trackType - APARAM_CHARACTER1;
				CCharacterTrack *pCharTrack = (CCharacterTrack*)pTrack;
				AnimateCharacterTrack( pCharTrack,ec,index );
			}
			break;
			
		//////////////////////////////////////////////////////////////////////////
		case APARAM_EXPRESSION1:
		case APARAM_EXPRESSION2:
		case APARAM_EXPRESSION3:
			if (!ec.bResetting)
			{
				CExprTrack *pExpTrack = (CExprTrack*)pTrack;
				AnimateExpressionTrack( pExpTrack,ec );
			}
			break;
		};
	}
	
	if (!ec.bResetting)
	{
		//////////////////////////////////////////////////////////////////////////
		// If not restting animation sequence.
		//////////////////////////////////////////////////////////////////////////
		if (!ec.bSingleFrame)
		{
			IPhysicalEntity *pEntityPhysics = m_entity->GetPhysics();
			if (pEntityPhysics)
			{
				if (ec.time - m_time < 0.1f)
				{
					float timeStep = ec.time - m_time;

					float rtimeStep = timeStep>1E-5f ? 1.0f/timeStep : 0.0f;
					pe_action_set_velocity asv;
					asv.v = (pos - m_pos)*rtimeStep;
					asv.w = (rotate*!m_rotate).v*(rtimeStep*2);
					pEntityPhysics->Action(&asv);
				}
			}
		}
	} // not single frame

	bool bMatrixModified = false;
	bool bPosModified = (m_pos != pos);
	bool bAnglesModified = (m_rotate.v != rotate.v) || (m_rotate.w != rotate.w);

	if (bPosModified || bAnglesModified || (m_scale != scale) || (m_target!=NULL))
	{
		InvalidateTM();
		bMatrixModified = true;

		m_pos = pos;
		m_scale = scale;
		m_rotate = rotate;
	}
		
	m_time = ec.time;

	if (bMatrixModified)
	{
/*
		if (m_target)
		{
			Matrix Mat;
			GetWorldTM(Mat);
			Quat q(Mat);
			Vec3 angles = q.GetEulerAngles();
			angles = RAD2DEG(angles);
			angles = angles;
		}
*/

		if (m_callback)
		{
			if (m_target)
			{
				// To Update m_rotate member for Editor.
				Matrix44 Mat;
				GetWorldTM(Mat);
			}
			m_callback->OnNodeAnimated();
		}
		else	// no callback specified, so lets move the entity directly
		{
			if (bPosModified && posTrack)
				m_entity->SetPos(m_pos,false);
			if (m_target)
			{
				Matrix44 Mat;
				GetWorldTM(Mat);

				//CHAMGED_BY_IVO	
				//Quat q(Mat);
				//Quat q = CovertMatToQuat<float>( GetTransposed44(Mat) );
				Quat q = Quat( GetTransposed44(Mat) );

				Vec3 angles = Ang3::GetAnglesXYZ(Matrix33(q));
				angles = RAD2DEG(angles);
				m_entity->SetAngles(angles);
			}else
			{
				if (bAnglesModified && rotTrack)
					m_entity->SetAngles( RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(m_rotate))) );
			}
			if (sclTrack)
				m_entity->SetScale(m_scale.x);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::ReleaseSounds()
{
	// stop all sounds
	for (int i=0;i<ENTITY_SOUNDTRACKS;i++)
	{
		if (m_SoundInfo[i].pSound)
			m_SoundInfo[i].pSound->Stop();
		m_SoundInfo[i].pSound = NULL;
		m_SoundInfo[i].nLastKey=-1;
		if (!m_SoundInfo[i].sLastFilename.empty())
			m_SoundInfo[i].sLastFilename = "";
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::Reset()
{
	m_lastEntityKey = -1;
	m_lastCharacterKey[0] = -1;
	m_lastCharacterKey[1] = -1;
	m_lastCharacterKey[2] = -1;
	ReleaseSounds();
	ReleaseAllAnims();
	m_entity = NULL;
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::Pause()
{
	ReleaseSounds();
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::CalcLocalMatrix()
{
	if (m_target)
	{
		// Calculate loookat matrix.
		float fRollAngle = 0;
		Matrix44 trgTM;
		m_target->GetWorldTM(trgTM);

		Vec3 pos = m_pos;
		IAnimNode *pParent = GetParent();
		if (pParent)
		{
			// Get our world position.
			Matrix44 parentTM;
			pParent->GetWorldTM(parentTM);
			pos = parentTM.TransformPointOLD(pos);
		}

		Vec3 camDirection = trgTM.GetTranslationOLD() - pos;
		m_worldTM = Vector2Matrix( camDirection,Vec3(0,0,-1),fRollAngle );

		// Rotate.. Assign to quaternion actual rotation.
		//QUAT_CHANGED_BY_IVO
		//m_rotate = Quat(m_worldTM);
		//m_rotate = CovertMatToQuat<float>( GetTransposed44(m_worldTM) );
		m_rotate = Quat( GetTransposed44(m_worldTM) );


		// Translate matrix.
		m_worldTM[3][0] = m_pos.x;
		m_worldTM[3][1] = m_pos.y;
		m_worldTM[3][2] = m_pos.z;
		m_worldTM[3][3] = 1.0f;

		m_bMatrixInWorldSpace = true;
		m_bMatrixValid = true;
	}
	else
	{
		// Calculate PRS (Pos-Rotation-Scale) Transform.
		//Q2M_CHANGED_BY_IVO
		//m_rotate.GetMatrix( m_worldTM );
		m_worldTM = GetTransposed44( Matrix44(m_rotate) );

		//SCALE_CHANGED_BY_IVO
		//m_worldTM.ScaleMatrix( m_scale.x,m_scale.y,m_scale.z );
		m_worldTM=Matrix33::CreateScale( Vec3(m_scale.x,m_scale.y,m_scale.z) ) * m_worldTM;

		// Translate matrix.
		m_worldTM[3][0] = m_pos.x;
		m_worldTM[3][1] = m_pos.y;
		m_worldTM[3][2] = m_pos.z;
		m_worldTM[3][3] = 1.0f;

		m_bMatrixInWorldSpace = false;
		m_bMatrixValid = true;
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::GetWorldTM( Matrix44 &tm ) 
{
	if (!m_bMatrixValid)
	{
		CalcLocalMatrix();
	}
	if (!m_bMatrixInWorldSpace)
	{
		IAnimNode *parent = GetParent();
		if (parent)
		{
			Matrix44 parentTM;
			parent->GetWorldTM(parentTM);
			m_worldTM = m_worldTM * parentTM;
		}
		m_bMatrixInWorldSpace = true;
	}
	tm = m_worldTM;
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::InvalidateTM()
{
	m_bMatrixInWorldSpace = false;
	m_bMatrixValid = false;
}

/*
//////////////////////////////////////////////////////////////////////////
Vec3 CAnimEntityNode::GetPos( float time )
{
	Vec3 pos(0,0,0);
	IAnimBlock *anim = GetAnimBlock();
	if (!anim)
		return pos;
	IAnimTrack *posTrack = anim->GetTrack(APARAM_POS);
	if (posTrack)
		posTrack->GetValue( time,pos );
	return pos;
}

//////////////////////////////////////////////////////////////////////////
Quat CAnimEntityNode::GetRotate( float time )
{
	Quat q;
	IAnimBlock *anim = GetAnimBlock();
	if (!anim)
		return q;
	IAnimTrack *rotTrack = anim->GetTrack(APARAM_ROT);
	if (rotTrack)
		rotTrack->GetValue( time,q );
	return q;
}

//////////////////////////////////////////////////////////////////////////
Vec3 CAnimEntityNode::GetScale( float time )
{
	Vec3 scl(1,1,1);
	IAnimBlock *anim = GetAnimBlock();
	if (!anim)
		return scl;
	IAnimTrack *sclTrack = anim->GetTrack(APARAM_SCL);
	if (sclTrack)
		sclTrack->GetValue( time,scl );
	return scl;
}
*/

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::SetPos( float time,const Vec3 &pos )
{
	bool bDefault = !(m_pMovieSystem->IsRecording() && (m_flags&ANODE_FLAG_SELECTED)); // Only selected nodes can be recorded
	IAnimBlock *anim = GetAnimBlock();
	if (anim)
	{
		IAnimTrack *posTrack = anim->GetTrack(APARAM_POS);
		if (posTrack)
		{
			posTrack->SetValue( time,pos,bDefault );
			if (bDefault && posTrack->GetNumKeys() > 0)
			{
				// Track is not empty. offset all keys by move ammount.
				Vec3 offset = pos - m_pos;
				// Iterator over all keys.
				ITcbKey key;
				for (int i = 0; i < posTrack->GetNumKeys(); i++)
				{
					// Offset each key.
					posTrack->GetKey(i,&key);
					key.SetVec3( key.GetVec3()+offset );
					posTrack->SetKey(i,&key);
				}
			}
		}
	}
	m_pos = pos;
	InvalidateTM();
}
	
//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::SetRotate( float time,const Quat &quat )
{
	m_rotate = quat;
	InvalidateTM();

	bool bDefault = !(m_pMovieSystem->IsRecording() && (m_flags&ANODE_FLAG_SELECTED)); // Only selected nodes can be recorded
	IAnimBlock *anim = GetAnimBlock();
	if (anim)
	{
		IAnimTrack *rotTrack = anim->GetTrack(APARAM_ROT);
		if (rotTrack)
			rotTrack->SetValue( time,m_rotate,bDefault );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::SetScale( float time,const Vec3 &scale )
{
	m_scale = scale;
	InvalidateTM();

	bool bDefault = !(m_pMovieSystem->IsRecording() && (m_flags&ANODE_FLAG_SELECTED)); // Only selected nodes can be recorded
	IAnimBlock *anim = GetAnimBlock();
	if (anim)
	{
		IAnimTrack *sclTrack = anim->GetTrack(APARAM_SCL);
		if (sclTrack)
			sclTrack->SetValue( time,scale,bDefault );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::ApplyEventKey( CEventTrack *track,int keyIndex,IEventKey &key )
{
	if (!m_entity)
		return;

	if (*key.animation) // if there is an animation
	{
		// Start playing animation.
		m_entity->StartAnimation( 0,key.animation,0,0 );
		ICryCharInstance *pCharacter = m_entity->GetCharInterface()->GetCharacter(0);
		if (pCharacter)
		{
			IAnimationSet* pAnimations = pCharacter->GetModel()->GetAnimationSet();
			assert (pAnimations);
			float duration = pAnimations->GetLength( key.animation );
			if (duration != key.duration)
			{
				key.duration = duration;
				track->SetKey( keyIndex,&key );
			}
		}
		//char str[1024];
		//sprintf( str,"StartAnim: %s",key.animation );
		//GetMovieSystem()->GetSystem()->GetILog()->LogToConsole( str );
	}
	
	if (*key.event) // if there's an event
	{
		// Fire event on Entity.
    m_entity->CallEventHandler(key.event);		
	}
	//m_entity->Hide( key.hidden );
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::ApplySoundKey( IAnimTrack *pTrack,int nCurrKey,int nLayer, ISoundKey &key, SAnimContext &ec )
{
	if (m_SoundInfo[nLayer].nLastKey==-1)
	{
		if (m_SoundInfo[nLayer].pSound)
			m_SoundInfo[nLayer].pSound->Stop();
		m_SoundInfo[nLayer].pSound=NULL;
		return;
	}
	if (((strcmp(m_SoundInfo[nLayer].sLastFilename.c_str(), key.pszFilename)) || (!m_SoundInfo[nLayer].pSound)))
	{
		int flags = 0;
		//if (key.bStream)
			//flags |= FLAG_SOUND_STREAM;
		//else
			flags |= FLAG_SOUND_LOAD_SYNCHRONOUSLY; // Allways synchronously for now.

		if (key.bLoop)
			flags |= FLAG_SOUND_LOOP;
		if (key.b3DSound)
		{
			// 3D sound.
			flags |= FLAG_SOUND_3D|FLAG_SOUND_RADIUS;
		}
		else
		{
			// 2D sound.
			flags |= FLAG_SOUND_2D|FLAG_SOUND_STEREO|FLAG_SOUND_16BITS;
		} 
		// we have a different sound now
		if (m_SoundInfo[nLayer].pSound)
			m_SoundInfo[nLayer].pSound->Stop();
		m_SoundInfo[nLayer].pSound=m_pMovie->GetSystem()->GetISoundSystem()->LoadSound(key.pszFilename, flags | FLAG_SOUND_UNSCALABLE );
		m_SoundInfo[nLayer].sLastFilename=key.pszFilename;
		if (m_SoundInfo[nLayer].pSound)
		{
			m_SoundInfo[nLayer].nLength=m_SoundInfo[nLayer].pSound->GetLengthMs();
			key.fDuration = ((float)m_SoundInfo[nLayer].nLength) / 1000.0f;
			pTrack->SetKey( nCurrKey,&key ); // Update key duration.
		}
	}else
	{
		if (m_SoundInfo[nLayer].pSound)
			m_SoundInfo[nLayer].pSound->Stop();
	}
	if (!m_SoundInfo[nLayer].pSound)
		return;
	
	m_SoundInfo[nLayer].pSound->SetSoundPriority( MOVIE_SOUND_PRIORITY );
	m_SoundInfo[nLayer].pSound->SetVolume(key.nVolume);
	if (key.b3DSound)
	{
		// 3D sound.
		m_SoundInfo[nLayer].pSound->SetPosition( m_entity->GetPos() );
		m_SoundInfo[nLayer].pSound->SetMinMaxDistance( key.inRadius,key.outRadius );
	}
	else
	{
		// 2D sound.
		m_SoundInfo[nLayer].pSound->SetPan(key.nPan);
	} 

	int nOffset=(int)((ec.time-key.time)*1000.0f);
	if (nOffset < m_SoundInfo[nLayer].nLength)
	{
		//return;
		m_SoundInfo[nLayer].pSound->SetCurrentSamplePos(nOffset, true);
	}
	((CMovieSystem*)m_pMovie)->OnPlaySound( m_SoundInfo[nLayer].pSound );
	if (!m_SoundInfo[nLayer].pSound->IsPlaying())
		m_SoundInfo[nLayer].pSound->Play();
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::ReleaseAllAnims()
{
	ResolveEntity(); // Resolve entity here as it may get deleted already.
	if (!m_entity)
		return;
	ICryCharInstance *pCharacter = m_entity->GetCharInterface()->GetCharacter(0);
	if (!pCharacter)
		return;
	ICryAnimationSet* pAnimations = pCharacter->GetModel()->GetAnimationSet();
	assert(pAnimations);
	for (TStringSetIt It=m_setAnimationSinks.begin();It!=m_setAnimationSinks.end();++It)
	{
		const char *pszName=(*It).c_str();
		pCharacter->RemoveAnimationEventSink(pszName, this);
		pAnimations->UnloadAnimation(pszName);
	}
	m_setAnimationSinks.clear();
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::OnEndAnimation(const char *sAnimation)
{
	if (!m_entity)
		return;
	TStringSetIt It=m_setAnimationSinks.find(sAnimation);
	if (It==m_setAnimationSinks.end())
		return;	// this anim was not started by us...
	m_setAnimationSinks.erase(It);
	ICryCharInstance *pCharacter = m_entity->GetCharInterface()->GetCharacter(0);
	if (!pCharacter)
		return;
	ICryAnimationSet* pAnimations = pCharacter->GetModel()->GetAnimationSet();
	assert(pAnimations);
	pCharacter->RemoveAnimationEventSink(sAnimation, this);
	pAnimations->UnloadAnimation(sAnimation);
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::AnimateCharacterTrack( class CCharacterTrack* track,SAnimContext &ec,int layer )
{
	ICryCharInstance *pCharacter = m_entity->GetCharInterface()->GetCharacter(0);
	if (!pCharacter)
		return;

	ICharacterKey key;
	int currKey = track->GetActiveKey(ec.time,&key);
	// If key is different or if time is standing exactly on key time.
	if (currKey != m_lastCharacterKey[layer] || key.time == ec.time || ec.time < m_time)
	{
		m_lastCharacterKey[layer] = currKey;
		if (strlen(key.animation) > 0)
		{
			// retrieve the animation collection for the model
			ICryAnimationSet* pAnimations = pCharacter->GetModel()->GetAnimationSet();
			assert (pAnimations);

			if (key.bUnload)
			{
				m_setAnimationSinks.insert(TStringSetIt::value_type(key.animation));
				pCharacter->AddAnimationEventSink(key.animation, this);
			}

			// Start playing animation.
			m_entity->StartAnimation( 0,key.animation,layer,key.blendTime );

			int animId = pAnimations->Find( key.animation );

//			pCharacter->EnableTimeUpdate(false);
			pCharacter->SetAnimationSpeed(layer, 0.0f);	// stop anim this way, so we dont block all other running anims
			float duration = pAnimations->GetLength(animId) + pAnimations->GetStart(animId);
			if (duration != key.duration)
			{
				key.duration = duration;
				track->SetKey( currKey,&key );
			}
			/*
			if (key.bLoop)
			{
				if (animId >= 0)
					pAnimations->SetLoop( animId,key.bLoop );
			}
			*/
		}
	}

	// Animate.
	if (currKey >= 0)
	{
		float t = ec.time - key.time;
		t = key.startTime + t*key.speed;
		if (t < key.duration || key.bLoop)
		{
			if (key.bLoop)
			{
				t = fmod(t,key.duration);
			}
			// Start playing animation.
			//m_character->EnableTimeUpdate(false);
			pCharacter->SetLayerTime(layer,t);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::StopExpressions()
{
	if (m_setExpressions.empty())
		return;
	IEntityCharacter *pEntChar=m_entity->GetCharInterface();
	if (!pEntChar)
		return;
	ILipSync *pLipSync=pEntChar->GetLipSyncInterface();
	if (!pLipSync)
		return;
	for (TStringSetIt It=m_setExpressions.begin();It!=m_setExpressions.end();++It)
	{
		pLipSync->StopExpression((*It).c_str());
	}
	m_setExpressions.clear();
}

void CAnimEntityNode::AnimateExpressionTrack(CExprTrack *pTrack, SAnimContext &ec)
{
	IExprKey Key;
	int nKeys=pTrack->GetNumKeys();
	// we go through all the keys, since expressions may overlap
	for (int nKey=0;nKey<nKeys;nKey++)
	{
		pTrack->GetKey(nKey, &Key);
		if ((!Key.pszName) || (!strlen(Key.pszName)))
			return;
		IEntityCharacter *pEntChar=m_entity->GetCharInterface();
		if (!pEntChar)
			return;
		ILipSync *pLipSync=pEntChar->GetLipSyncInterface();
		if (pLipSync)
		{
			float fKeyLentgh=Key.fBlendIn+Key.fHold+Key.fBlendOut;
			float fOffset=ec.time-Key.time;
			if ((Key.time>ec.time) || (fOffset>=fKeyLentgh))
			{
//				pLipSync->StopExpression(Key.pszName);
				continue;
			}
			CryCharMorphParams MorphParams;
			MorphParams.fAmplitude=Key.fAmp;
			MorphParams.fBlendIn=Key.fBlendIn;
			MorphParams.fBlendOut=Key.fBlendOut;
			MorphParams.fLength=Key.fHold;
			MorphParams.fStartTime=fOffset;
			if (pLipSync->DoExpression(Key.pszName, MorphParams, false))
				m_setExpressions.insert(Key.pszName);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::SetTarget( IAnimNode *node )
{
	IAnimNode *prevTarget = m_target;
	m_target = node;
	if (prevTarget != m_target)
		InvalidateTM();
}

//////////////////////////////////////////////////////////////////////////
IAnimNode* CAnimEntityNode::GetTarget() const
{
	return m_target;
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::SetAnimBlock( IAnimBlock *block )
{
	CAnimNode::SetAnimBlock(block);
	if (block)
	{
		// Initialize new block with default track values.
		// Used exlusively in Editor.
		IAnimTrack *posTrack = block->GetTrack(APARAM_POS);
		IAnimTrack *rotTrack = block->GetTrack(APARAM_ROT);
		IAnimTrack *sclTrack = block->GetTrack(APARAM_SCL);
		if (posTrack)
			posTrack->SetValue( 0,m_pos,true );
		if (rotTrack)
			rotTrack->SetValue( 0,m_rotate,true );
		if (sclTrack)
			sclTrack->SetValue( 0,m_scale,true );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimEntityNode::Serialize( XmlNodeRef &xmlNode,bool bLoading )
{
	CAnimNode::Serialize( xmlNode,bLoading );
	if (bLoading)
	{
		xmlNode->getAttr( "Pos",m_pos );
		xmlNode->getAttr( "Rotate",m_rotate );
		xmlNode->getAttr( "Scale",m_scale );
		xmlNode->getAttr( "EntityId",m_EntityId );
	}
	else
	{
		xmlNode->setAttr( "Pos",m_pos );
		xmlNode->setAttr( "Rotate",m_rotate );
		xmlNode->setAttr( "Scale",m_scale );
		xmlNode->setAttr( "EntityId",m_EntityId );
	}
}