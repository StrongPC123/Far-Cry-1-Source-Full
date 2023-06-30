//////////////////////////////////////////////////////////////////////
//
//	Physical Placeholder header
//	
//	File: physicalplaceholder.h
//	Description : PhysicalPlaceholder class declarations
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef physicalplaceholder_h
#define physicalplaceholder_h
#pragma once

struct pe_gridthunk {
	pe_gridthunk *next;
	pe_gridthunk *prev;
	class CPhysicalPlaceholder *pent;
};
class CPhysicalEntity;

class CPhysicalPlaceholder : public IPhysicalEntity {
public:
	vectorf m_BBox[2];

	void *m_pForeignData;
	int m_iForeignData  : 16;
	int m_iForeignFlags : 16;

	struct vec2dpacked {
		int x : 16;
		int y : 16;
	};
	vec2dpacked m_ig[2];
	pe_gridthunk *m_pGridThunks;
	int m_nGridThunks : 16;
	int m_nGridThunksAlloc : 16;

	CPhysicalPlaceholder *m_pEntBuddy;
	unsigned int m_bProcessed : 1;
	int m_id : 23;
	int m_iSimClass : 8;

	virtual CPhysicalEntity *GetEntity();
	virtual CPhysicalEntity *GetEntityFast() { return (CPhysicalEntity*)m_pEntBuddy; }

	virtual pe_type GetType();
	virtual int SetParams(pe_params* params);
	virtual int GetParams(pe_params* params);
	virtual int GetStatus(pe_status* status);
	virtual int Action(pe_action* action);

	virtual int AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id=-1);
	virtual void RemoveGeometry(int id);

	virtual void *GetForeignData(int itype=0) { return m_iForeignData==itype ? m_pForeignData : 0; }
	virtual int GetiForeignData() { return m_iForeignData; }

	virtual int GetStateSnapshot(class CStream &stm, float time_back=0, int flags=0);
	virtual int SetStateFromSnapshot(class CStream &stm, int flags=0);
	virtual int PostSetStateFromSnapshot();
	virtual int GetStateSnapshotTxt(char *txtbuf,int szbuf, float time_back=0);
	virtual void SetStateFromSnapshotTxt(const char *txtbuf,int szbuf);
	virtual unsigned int GetStateChecksum();

	virtual void StartStep(float time_interval);
	virtual int Step(float time_interval);
	virtual void StepBack(float time_interval);
	virtual IPhysicalWorld *GetWorld();

	virtual void GetMemoryStatistics(ICrySizer *pSizer) {};
};

#endif