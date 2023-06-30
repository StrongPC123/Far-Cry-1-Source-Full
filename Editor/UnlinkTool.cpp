#include "StdAfx.h"
#include "unlinktool.h"

IMPLEMENT_DYNAMIC(CUnlinkTool,CEditTool)

CUnlinkTool::CUnlinkTool()
{
	CSelectionGroup *pSelection=GetIEditor()->GetObjectManager()->GetSelection();
	for (int i=0;i<pSelection->GetCount();i++)
	{
		CBaseObject *pBaseObj=pSelection->GetObject(i);
		if (IsRelevant(pBaseObj))
			pBaseObj->DetachThis();
	}
	GetIEditor()->SetEditTool(0);
}

CUnlinkTool::~CUnlinkTool()
{
}