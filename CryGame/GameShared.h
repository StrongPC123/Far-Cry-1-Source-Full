//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: GameShared.h
//  Description: Stuffs shared by the game's source files.
//
//  History:
//  - August 9, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_GAMESHARED_H
#define GAME_GAMESHARED_H
#if _MSC_VER > 1000
# pragma once
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
#define INVALID_WID		0	// Invalid WORD ID --- value for the CIDGenerator class

//////////////////////////////////////////////////////////////////////////////////////////////
// Actions

typedef unsigned char ACTIONTYPE;

#define ACTION_MOVE_LEFT			1
//#define ACTIONFLAG_MOVE_LEFT		(1<<ACTION_MOVE_LEFT)

#define ACTION_MOVE_RIGHT			2
//#define ACTIONFLAG_MOVE_RIGHT		(1<<ACTION_MOVE_RIGHT)

#define ACTION_MOVE_FORWARD			3
//#define ACTIONFLAG_MOVE_FORWARD		(1<<ACTION_MOVE_FORWARD)

#define ACTION_MOVE_BACKWARD		4
//#define ACTIONFLAG_MOVE_BACKWARD	(1<<ACTION_MOVE_BACKWARD)

#define ACTION_JUMP					5
//#define ACTIONFLAG_JUMP				(1<<ACTION_JUMP)

#define ACTION_MOVEMODE				6
//#define ACTIONFLAG_MOVEMODE			(1<<ACTION_MOVEMODE)

#define ACTION_FIRE0				7
//#define ACTIONFLAG_FIRE0			(1<<ACTION_FIRE0)

#define ACTION_VEHICLE_BOOST				8	// orphaned
//#define ACTIONFLAG_VEHICLE_BOOST			(1<<ACTION_VEHICLE_BOOST)
//<<FIXME>> we need more bits!!
#define ACTION_SCORE_BOARD				8
//#define ACTIONFLAG_SCORE_BOARD			(1<<ACTION_SCORE_BOARD)

#define ACTION_RELOAD				9
//#define ACTIONFLAG_RELOAD			(1<<ACTION_RELOAD)

#define ACTION_USE						10
//#define ACTIONFLAG_USE			(1<<ACTION_USE)

#define ACTION_TURNLR				11
//#define ACTIONFLAG_TURNLR			(1<<ACTION_TURNLR)

#define ACTION_TURNUD				12
//#define ACTIONFLAG_TURNUD			(1<<ACTION_TURNUD)

#define ACTION_WALK					13
//#define ACTIONFLAG_WALK				(1<<ACTION_WALK)

#define ACTION_NEXT_WEAPON	14
//#define ACTIONFLAG_NEXT_WEAPON	(1<<ACTION_NEXT_WEAPON)

#define ACTION_PREV_WEAPON	15
//#define ACTIONFLAG_PREV_WEAPON	(1<<ACTION_PREV_WEAPON)

#define ACTION_LEANRIGHT				16
//#define ACTIONFLAG_LEANRIGHT			(1<<ACTION_LEANRIGHT)

#define ACTION_HOLDBREATH					17
//#define ACTIONFLAG_HOLDBREATH (1<<ACTION_HOLDBREATH)

#define ACTION_FIREMODE					18
//#define ACTIONFLAG_FIREMODE (1<<ACTION_FIREMODE)

#define ACTION_LEANLEFT				19
//#define ACTIONFLAG_LEANLEFT			(1<<ACTION_LEANLEFT)

#define ACTION_FIRE_GRENADE 20
//#define ACTIONFLAG_FIRE_GRENADE (1<<ACTION_FIRE_GRENADE)

#define ACTION_WEAPON_0			21
#define ACTION_WEAPON_1			(ACTION_WEAPON_0+1)
#define ACTION_WEAPON_2			(ACTION_WEAPON_0+2)
#define ACTION_WEAPON_3			(ACTION_WEAPON_0+3)
#define ACTION_WEAPON_4			(ACTION_WEAPON_0+4)
/*#define ACTION_WEAPON_5			(ACTION_WEAPON_0+5)
#define ACTION_WEAPON_6			(ACTION_WEAPON_0+6)
#define ACTION_WEAPON_7			(ACTION_WEAPON_0+7)
#define ACTION_WEAPON_8			(ACTION_WEAPON_0+8)
*/
#define ACTION_CYCLE_GRENADE	(ACTION_WEAPON_0+9)

#define ACTION_DROPWEAPON		(ACTION_WEAPON_0+10)

#define ACTION_CONCENTRATION	32
//#define ACTIONFLAG_WEAPON_0	(1<<ACTION_WEAPON_0)
//#define ACTIONFLAG_WEAPON_1	(1<<ACTION_WEAPON_1)
//#define ACTIONFLAG_WEAPON_2	(1<<ACTION_WEAPON_2)
//#define ACTIONFLAG_WEAPON_3	(1<<ACTION_WEAPON_3)
//#define ACTIONFLAG_WEAPON_4	(1<<ACTION_WEAPON_4)
//#define ACTIONFLAG_WEAPON_5	(1<<ACTION_WEAPON_5)
//#define ACTIONFLAG_WEAPON_6	(1<<ACTION_WEAPON_6)
//#define ACTIONFLAG_WEAPON_7	(1<<ACTION_WEAPON_7)
//#define ACTIONFLAG_WEAPON_8	(1<<ACTION_WEAPON_8)
//#define ACTIONFLAG_WEAPON_9	(1<<ACTION_WEAPON_9)

//#define ACTIONFLAG_DROPWEAPON	(1<<ACTION_DROPWEAPON)

//client side only 
#define ACTION_ITEM_0		35
#define ACTION_ITEM_1		36
#define ACTION_ITEM_2		37
#define ACTION_ITEM_3		38
#define ACTION_ITEM_4		39
#define ACTION_ITEM_5		40
#define ACTION_ITEM_6		41
#define ACTION_ITEM_7		42
#define ACTION_ITEM_8		43

#define ACTION_ZOOM_TOGGLE	45
#define ACTION_ZOOM_IN			46
#define ACTION_ZOOM_OUT			47

//#define ACTION_RELOAD_WEAPON	48


#define ACTION_MOVEMODE2						49
//#define ACTIONLOCALFLAG_MOVEMODE2		1	


////#define ACTIONFLAG_RUN				(1<<ACTION_RUN)

//#define ACTION_QUICKLOAD			50
//#define ACTION_QUICKSAVE			51

#define ACTION_SAVEPOS				54
#define ACTION_LOADPOS				55


#define ACTION_FIRECANCEL			56

#define ACTION_FLASHLIGHT			57

// to switch between 1st and 3rd person mode while driving
// and shooting
#define ACTION_CHANGE_VIEW		58

//
// make player run very fast
#define ACTION_RUNSPRINT			59

#define ACTION_MESSAGEMODE		60
#define ACTION_MESSAGEMODE2		61
#define ACTION_TAKESCREENSHOT	62

#define ACTION_MOVEMODE_TOGGLE 63
#define ACTION_AIM_TOGGLE	64

//////////////////////////////////////////////////////////////////////////////////////////////
#define PLAYER_MAX_WEAPONS 9

//////////////////////////////////////////////////////////////////////////
typedef struct  
{
	int									m_nFireMode;		//< active firemode
	std::vector<int>		m_nAmmoInClip;	//< amount of ammo in the clip of that firemode 
} tWeaponPersistentData;

typedef struct  
{
	bool								m_bDataSaved;
	int									m_nHealth,m_nArmor;
	int									m_nSelectedWeaponID;
	int									m_nAmmo;				//< only valid if m_nSelectedWeaponID != -1
	int									m_nAmmoInClip;	//< only valid if m_nSelectedWeaponID != -1
	int									m_vWeaponSlots[PLAYER_MAX_WEAPONS];
	std::map<int, tWeaponPersistentData>	m_mapWeapons;
	std::map<string, int>	m_mapAmmo;
	std::list<string>	m_lItems;	 
} tPlayerPersistentData;

//////////////////////////////////////////////////////////////////////////


#endif // GAME_GAMESHARED_H
