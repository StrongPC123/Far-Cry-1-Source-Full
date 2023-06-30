/*
=======================================================================
FILE : CREHDRProcess.cpp
DESC : HDR processing render element
PROJ : Crytek Engine
CODER: Andrey Honich

=======================================================================
*/

#include "RenderPCH.h"


// constructor/destructor
CREHDRProcess::CREHDRProcess()
{
  // setup screen process renderer type
  mfSetType(eDATA_HDRProcess);
  mfUpdateFlags(FCEF_TRANSFORM);
}

CREHDRProcess::~CREHDRProcess()
{  
};

// prepare screen processing
void CREHDRProcess:: mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_FlagsPerFlush |= RBSI_DRAWAS2D;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 0;
}

void CREHDRProcess::mfReset()
{
}

void CREHDRProcess::mfActivate(int iProcess)
{
}