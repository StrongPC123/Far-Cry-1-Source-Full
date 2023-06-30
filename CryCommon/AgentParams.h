#ifndef _AGENTPARAMS_H_
#define _AGENTPARAMS_H_


typedef struct AgentParameters
{
	//-------------
	// ranges
	//----------------
	// sensors:
	// sight
	float	m_fSightRange;		// how far can the agent see
	float m_fHorizontalFov;
	float m_fGravityMultiplier;
	float m_fAccuracy;
	float m_fOriginalAccuracy;
	float m_fResponsiveness;

	float m_fMaxHealth;

	// sound
	float m_fSoundRange;		// how far can agent hear

	// behaviour
	float m_fAttackRange;
	float m_fCommRange;

	//-----------
	// indices
	//-------------
	float m_fAggression;
	float m_fOriginalAggression;
	float m_fCohesion;
	float m_fPersistence;

	//-----------
	// hostility data
	//------------
	float m_fSpeciesHostility;
	float m_fGroupHostility;
	float m_fMeleeDistance;

	//-------------
	// grouping data
	//--------------
	int		m_nSpecies;
	int		m_nGroup;

	bool  m_bIgnoreTargets;
	bool  m_bPerceivePlayer;
	bool  m_bAwareOfPlayerTargeting;
	bool  m_bSpecial;
	bool  m_bSmartMelee;


	AgentParameters()
	{
			m_bSmartMelee = true;
			m_bSpecial = false;
			m_fSightRange=0;
			m_fHorizontalFov=0;
			m_fGravityMultiplier=1.f;
			m_fAccuracy=0;
			m_fResponsiveness=0;

			m_fMaxHealth=0;
			m_fSoundRange=0;
			m_fAttackRange=0;
			m_fCommRange=0;

			m_fAggression=0;
			m_fCohesion=0;
			m_fPersistence=0;

			m_fSpeciesHostility=0;
			m_fGroupHostility=0;

			m_nSpecies=0;
			m_nGroup=0;

			m_fMeleeDistance = 2.f;

			m_bIgnoreTargets=false;
			m_bPerceivePlayer=true;
			m_bAwareOfPlayerTargeting=false;
	}


	
} AgentParameters;


#endif _AGENTPARAMS_H_

