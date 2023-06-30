
//////////////////////////////////////////////////////////////////////
//
//	Game source code (c) Crytek 2001-2003
//	
//	File: GameRadar.cpp
//  
//	History:
//	-December 11,2001: created
//	-October	31,2003: split from GameLoading.cpp and other files
//	
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

#include "Game.h"
#include "XNetwork.h"
#include "XServer.h"
#include "XClient.h"

#include "UIHud.h"

#include "XPlayer.h"
#include "PlayerSystem.h"
#include "XServer.h"
#include "WeaponSystemEx.h"
#include "ScriptObjectGame.h"
#include "ScriptObjectInput.h"
#include <IEntitySystem.h>

#include "UISystem.h"
#include "ScriptObjectUI.h"

// render game radar
// notes: clean up/optimize code, lots of redundant stuff
//////////////////////////////////////////////////////////////////////////
void CXGame::DrawRadar(float x, float y, float w, float h, float fRange, INT_PTR *pRadarTextures, _SmartScriptObject *pEntities, char *pRadarObjective)
{
  // 0 - Radar
  // 1 - RadarMask
  // 2 - RadarPlayerIcon
  // 3 - RadarEnemyInRangeIcon
  // 4 - RadarEnemyOutRangeIcon
  // 5 - RadarSoundIcon
	// 6 - RadarObjectiveIcon

  // check if data ok
  if (!m_pRenderer || !pRadarTextures || !pRadarObjective)
  {
    return;
  }

  IEntity *pPlayer=GetMyPlayer();

  if (!pPlayer)
  {
    return;
  }

  // clamp minimum value
  if(fRange<10.0f) 
  {
    fRange=10.0f;
  }
  
  ICVar *pFadeAmount=0;
  if(m_pSystem && m_pSystem->GetIConsole()) 
  {
    pFadeAmount=m_pSystem->GetIConsole()->GetCVar("hud_fadeamount");
  }

  float fFadeAmount=1;
  if(pFadeAmount)
  {
     fFadeAmount=pFadeAmount->GetFVal();
  }
  
  // set render state
  m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
  m_pRenderer->Set2DMode(true, 800, 600);

  // radar texture id's
  INT_PTR iRadarID=pRadarTextures[0], 
      iRadarMaskID=pRadarTextures[1], 
      iPlayerIconID=pRadarTextures[2], 
      iEnemyInRangeIconID=pRadarTextures[3],
      iEnemyOutRangeIconID=pRadarTextures[4],
      iSoundIconID=pRadarTextures[5],
			iObjectiveIconID=pRadarTextures[6];

  // last radar alpha/used to mask out sound events
  static float fLastAlpha=0.0f, fLastAvgDist=0.0f;
  ITimer *pTimer=m_pSystem->GetITimer();

  float fCurrPosStep=fLastAlpha*5.0f; 

  if(fCurrPosStep>1.0f)
  {
    fCurrPosStep=1.0f;
  }

  Vec3d fMapCenter(x+w*0.5f, y+h*0.5f, 0.5f);

  // render radar/compass

  int iPx=0, iPy=0, iW=0, iH=0;
  m_pRenderer->GetViewport(&iPx, &iPy, &iW, &iH);

  // used to scale y coordinates correctly acoording to aspect ratio
  float fScaleY=((float)iW/(float)iH)*0.75f;
  
  // get radar mask data
  float fMaskPosX=fMapCenter.x-0.5f*186.0f;
  float fMaskPosY=fMapCenter.y-(0.5f*186.0f)*fScaleY;
  float fMaskW=186.0f;
  float fMaskH=186.0f*fScaleY;

  // clear radar background alpha
  m_pRenderer->SetState(GS_BLSRC_ONE | GS_BLDST_ZERO | GS_NODEPTHTEST | GS_COLMASKONLYALPHA);
  m_pRenderer->Draw2dImage(fMaskPosX, fMaskPosY, fMaskW, fMaskH, iRadarMaskID, 0, 0.1f, 0.1f, 0, 0, 1, 1, 1, 1);

  // get radar data
  float fRadarPosX=fMapCenter.x-0.5f*130.0f;
  float fRadarPosY=fMapCenter.y-(0.5f*130.0f)*fScaleY;

  float fRadarW=130.0f;
  float fRadarH=(130.0f)*fScaleY;
  // render radar mask into alpha channel
  m_pRenderer->Draw2dImage(fRadarPosX, fRadarPosY, fRadarW, fRadarH, iRadarMaskID, 0, 1, 1, 0, 0, 1,1, 1, 1); 

  // add the radar
  m_pRenderer->SetState(GS_BLSRC_ONEMINUSDSTALPHA | GS_BLDST_DSTALPHA | GS_NODEPTHTEST);
  static float fCurrCompassAngle=pPlayer->GetAngles().z;
  float fCurrAngle=pPlayer->GetAngles().z;
  fCurrCompassAngle+=(pPlayer->GetAngles().z-fCurrCompassAngle)*pTimer->GetFrameTime()*8.0f;    
  m_pRenderer->Draw2dImage(fRadarPosX, fRadarPosY, fRadarW, fRadarH, iRadarID, 0, 1, 1, 0, fCurrCompassAngle+90.0f, fFadeAmount,fFadeAmount, fFadeAmount, 1); 
     
  // add radar noise..
  static float fNoiseMove=0.0f;
  fNoiseMove+=pTimer->GetFrameTime()*2.0f;      
  m_pRenderer->Draw2dImage(fRadarPosX, fRadarPosY, fRadarW, fRadarH, iRadarMaskID, 0+fNoiseMove, 0, 0.1f+fNoiseMove, 1.5f, 0, 1, 1.0f, 1.0f, 1.0f);  

  // render player icon
  float fPlayerSize=16.0f;
  float fTexOffset=0.5f/fPlayerSize; 
  m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
  float fPlayerAlpha=(fLastAlpha<=0.6f)? 0.6f: fLastAlpha;
  m_pRenderer->Draw2dImage(fMapCenter.x-fPlayerSize*0.5f, fMapCenter.y-fPlayerSize*0.5f,
    fPlayerSize, fPlayerSize, iPlayerIconID, fTexOffset, 1-fTexOffset, 1-fTexOffset, fTexOffset, 0, fPlayerAlpha*fFadeAmount, fPlayerAlpha*fFadeAmount, fPlayerAlpha*fFadeAmount, fPlayerAlpha);   

  // get player position
  Vec3d pPlayerPos=pPlayer->GetPos();

  // draw sound-events
  CXClient *pClient=GetClient(); 
  m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
  if (pClient) //&& fLastAlpha>0.0f)
  {
    TSoundList SoundList=pClient->GetSoundEventList();
        
    float fScale=(w*0.5f); ///fRange;        
    Matrix33 mtxTransformNoMove;
    mtxTransformNoMove.SetScale(Vec3(fScale , (fScale)*fScaleY, 0.0f));

    mtxTransformNoMove=mtxTransformNoMove*Matrix33::CreateRotationZ(DEG2RAD(-pPlayer->GetAngles().z));
    Matrix34 mtxTransform=mtxTransformNoMove;
    Matrix34 TransferVector;
    TransferVector.SetTranslationMat(-pPlayer->GetPos());
    mtxTransform=mtxTransform*TransferVector;

    for (TSoundListIt It=SoundList.begin();It!=SoundList.end();++It)
    {
      SSoundInfo &SoundInfo=(*It);
      if (SoundInfo.fThread==0.0f)
      {
        // only display threatening sounds
        continue;
      }

      Vec3d pPos=mtxTransform*SoundInfo.Pos;
      
      Vec3d pSoundPos=SoundInfo.Pos;
      float fCurrDist=cry_sqrtf((pPlayerPos.x-pSoundPos.x)*(pPlayerPos.x-pSoundPos.x)+(pPlayerPos.y-pSoundPos.y)*(pPlayerPos.y-pSoundPos.y)+(pPlayerPos.z-pSoundPos.z)*(pPlayerPos.z-pSoundPos.z));    

      float fRadius=fScale*SoundInfo.fRadius/fRange;
      float fPhase=SoundInfo.fTimeout/pClient->cl_sound_event_timeout->GetFVal();
      pPos.x=-pPos.x;
      float fSoundIconSize; 
      
      if(fCurrDist>fRange)  
      {
        if(fCurrDist==0.0f)
        {
          fCurrDist=1.0f;  
        }

        fSoundIconSize=20.0f*(1.0f-fPhase);
        fCurrDist=1.0f/fCurrDist;
        m_pRenderer->Draw2dImage(fMapCenter.x+pPos.x*fCurrDist-fSoundIconSize*0.5f, fMapCenter.y+pPos.y*fCurrDist-fSoundIconSize*0.5f,
                                 fSoundIconSize, fSoundIconSize, iSoundIconID, 0, 0, 1, 1, 0, fFadeAmount, fFadeAmount, fFadeAmount, fPhase);
      }
      else
      {
        fCurrDist/=15.0f; 
        if(fCurrDist<1.2f && fCurrDist>0.0f)
        {
          fCurrDist=1.2f; 
        }
        else
          if(fCurrDist==0.0f)
          {
            fCurrDist=1.0f;  
          }

        fSoundIconSize=fRadius*(1.0f-fPhase);
        fCurrDist=1.0f/fCurrDist;         
        m_pRenderer->Draw2dImage(fMapCenter.x+(pPos.x/fRange)-fSoundIconSize*0.5f*fCurrDist, fMapCenter.y+(pPos.y/fRange)-fSoundIconSize*0.5f*fCurrDist, fSoundIconSize*fCurrDist, fSoundIconSize*fCurrDist, iSoundIconID, 0, 0, 1, 1, 0, fFadeAmount, fFadeAmount, fFadeAmount, fPhase);
      }
    }
  }

  // draw players
  float PlayerZAngle=pPlayer->GetAngles().z;
  int nCount=1;
  _SmartScriptObject pEntitySO(m_pScriptSystem, true);
  _SmartScriptObject pColor(m_pScriptSystem, true);
  (*pEntities)->BeginIteration();

  // compute average distance to player, for dinamyc radar scale adjustment   
  float fAvgDist=0.0f;
  int  iTotalEntities=1;

  float fScale=(w*0.5f);
  ASSERT(fScale>0.0f);
  Matrix33 mtxTransformNoMove;
  mtxTransformNoMove.SetScale(Vec3(fScale, (fScale)*fScaleY, 0.0f));
  mtxTransformNoMove=mtxTransformNoMove*Matrix33::CreateRotationZ(DEG2RAD(-pPlayer->GetAngles().z));
  Matrix34 mtxTransform=mtxTransformNoMove; 
  Matrix34 TransferVector;
  TransferVector.SetTranslationMat(-pPlayer->GetPos());
  mtxTransform=mtxTransform*TransferVector;

  m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);

  // show radar objective direction
  // get radar objective position
  Vec3d pObjectivePos(0,0,0);
  int iHaveObjective= strcmp("NoObjective", pRadarObjective);
  if(iHaveObjective)
  {
    if(sscanf(pRadarObjective, "%f %f %f", &pObjectivePos.x, &pObjectivePos.y, &pObjectivePos.z)!=3)
    {
      return;
    }

    Vec3d pObjectiveScreenPos=mtxTransform*pObjectivePos;
    pObjectiveScreenPos.x=-pObjectiveScreenPos.x;// invert x due to different mapping

    // compute distance and sum values    
    float fCurrDist=cry_sqrtf((pPlayerPos.x-pObjectivePos.x)*(pPlayerPos.x-pObjectivePos.x)+(pPlayerPos.y-pObjectivePos.y)*(pPlayerPos.y-pObjectivePos.y));    

     
    float r=0.0f, g=1.0f, b=1.0f, a=1.0f;   


    if(fCurrDist>fRange)
    {
      float fPosX=pObjectiveScreenPos.x, fPosY=pObjectiveScreenPos.y; 
      float fLen=cry_sqrtf(fPosX*fPosX + fPosY*fPosY);
      if(fLen)
      {
        fPosX/=fLen;
        fPosY/=fLen;
      }

      float fOutRangeAngle=0;
      if(fPosX<0.0f) 
      {
        fOutRangeAngle=RAD2DEG(cry_acosf(fPosY));  
      }
      else
      {
        fOutRangeAngle=360.0f-RAD2DEG(cry_acosf(fPosY));   
      }  

      m_pRenderer->Draw2dImage(fMapCenter.x+fPosX*52.0f-10.0f*0.5f, fMapCenter.y+fPosY*52.0f*fScaleY-10.0f*0.5f,
        10.0f, 10.0f, iEnemyOutRangeIconID, 0, 0, 1, 1, fOutRangeAngle, r*fFadeAmount, g*fFadeAmount, b*fFadeAmount, a*0.5f);     

     }
    else
    {

      float r=0.0f, g=1.0f, b=1.0f, a=1.0f;
      fCurrDist/=15.0f; 
      if(fCurrDist<1.2f && fCurrDist>0.0f)
      {
        fCurrDist=1.2f; 
      }
      else
        if(fCurrDist==0.0f)
        {
          fCurrDist=1.0f;  
        }

        fCurrDist=1.0f/fCurrDist;

        float fVerticalDist=fabsf(pPlayerPos.z-pObjectivePos.z);
        // to distant on vertical range, must be on diferent floor/level, put icons gray
        if(fVerticalDist>fRange*0.15f) 
        {
          r*=0.5f;
          g*=0.5f;
          b*=0.5f; 
        }

        static float fCurrSize=0.0f;
        fCurrSize+=pTimer->GetFrameTime()*2.0f;
        if(fCurrSize>1.0f)
        {
          fCurrSize=0.0f;
        }        
        m_pRenderer->Draw2dImage(fMapCenter.x+(pObjectiveScreenPos.x/fRange)-10.0f*fCurrSize*0.5f, fMapCenter.y+(pObjectiveScreenPos.y/fRange)-10.0f*fCurrSize*0.5f,
          10.0f*fCurrSize, 10.0f*fCurrSize, iObjectiveIconID, 0, 0, 1, 1, 0.0f, r*a*fFadeAmount, g*a*fFadeAmount, b*a*fFadeAmount, a);  
    } 
  }

  // render entities 
  while ((*pEntities)->MoveNext())
  {
    if (!(*pEntities)->GetCurrent(*pEntitySO))
    {
      continue;
    }

    int nId;
    if (!pEntitySO->GetValue("nId", nId))
    {
      continue;
    }

    IEntity *pEntity=m_pEntitySystem->GetEntity(nId);
    if (!pEntity)
    {
      continue;
    }

    Vec3d Pos=mtxTransform*pEntity->GetPos();
    Pos.x=-Pos.x;// invert x due to different mapping

    float r=1.0f, g=1.0f, b=1.0f, a=1.0f;
    if (pEntitySO->GetValue("Color", *pColor))
    {
      pColor->GetValue("r", r);
      pColor->GetValue("g", g);
      pColor->GetValue("b", b);
      pColor->GetValue("a", a);
    }

    fLastAlpha=a;

    // no need to do more stuff..
    if(fLastAlpha==0.0f)
    {
      continue;
    }

    Matrix33 IconMatrix;
    float fAngleZ=PlayerZAngle-pEntity->GetAngles(true).z;

    // [marco] temporay fix to be able
    // to run a version with debug info
    // on and do not get an assert every frame
    if (fAngleZ<-360.0f)
    {
      fAngleZ+=360;
    }
    else
      if (fAngleZ>360.0f)
      {
        fAngleZ-=360; 
      }

      float fEnemyInRangeSize=10.0f, 
        fEnemyOutRangeSize=5.0f; 

      // compute distance and sum values
      Vec3d pEntityPos=pEntity->GetPos();
      float fCurrDist=cry_sqrtf((pPlayerPos.x-pEntityPos.x)*(pPlayerPos.x-pEntityPos.x)+(pPlayerPos.y-pEntityPos.y)*(pPlayerPos.y-pEntityPos.y));    

      // skip player..
      if(fCurrDist<0.01)
      {
        continue;
      }

      // render enemy icons
      if(fCurrDist>fRange)
      {    
        if(fCurrDist==0.0f)
        {
          fCurrDist=1.0f;  
        }

        fCurrDist=1.0f/fCurrDist;

        float fPosX=Pos.x, fPosY=Pos.y; 
        float fLen=cry_sqrtf(fPosX*fPosX + fPosY*fPosY);
        if(fLen)
        {
          fPosX/=fLen;
          fPosY/=fLen;
        }

        float fOutRangeAngle=0;
        if(fPosX<0.0f) 
        {
          fOutRangeAngle=RAD2DEG(cry_acosf(fPosY));  
        }
        else
        {
          fOutRangeAngle=360.0f-RAD2DEG(cry_acosf(fPosY));   
        } 

        m_pRenderer->Draw2dImage(fMapCenter.x+Pos.x*fCurrDist*0.98f-fEnemyOutRangeSize*0.5f, fMapCenter.y+Pos.y*fCurrDist*0.98f-fEnemyOutRangeSize*0.5f,
          fEnemyOutRangeSize, fEnemyOutRangeSize, iEnemyOutRangeIconID, 0, 0, 1, 1, fOutRangeAngle, 0, a*fFadeAmount, 0, a);  
      }
      else
      {
        fCurrDist/=15.0f; 
        if(fCurrDist<1.2f && fCurrDist>0.0f)
        {
          fCurrDist=1.2f; 
        }
        else
          if(fCurrDist==0.0f)
          {
            fCurrDist=1.0f;  
          }

          fCurrDist=1.0f/fCurrDist;

          float fVerticalDist=fabsf(pPlayerPos.z-pEntityPos.z);
          // to distant on vertical range, must be on diferent floor/level, put icons gray
          if(fVerticalDist>fRange*0.15f) 
          {
            r*=0.5f;
            g*=0.5f;
            b*=0.5f; 
          }

          m_pRenderer->Draw2dImage(fMapCenter.x+(Pos.x*0.98f/fRange)-fEnemyInRangeSize*0.5f*fCurrDist, fMapCenter.y+(Pos.y*0.98f/fRange)-fEnemyInRangeSize*0.5f*fCurrDist,
            fEnemyInRangeSize*fCurrDist, fEnemyInRangeSize*fCurrDist, iEnemyInRangeIconID, 0, 0, 1, 1, fAngleZ+180.0f, r*a*fFadeAmount, g*a*fFadeAmount, b*a*fFadeAmount, a);
      }
  }

  (*pEntities)->EndIteration();

  // reset states 
  m_pRenderer->SetState(GS_NODEPTHTEST);  
  m_pRenderer->Set2DMode(false, 800, 600);   
}

