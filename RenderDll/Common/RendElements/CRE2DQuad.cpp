#include "RenderPCH.h"
#include "RendElement.h"
#include "CRE2DQuad.h"


void *CRE2DQuad::mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags)
{
  *Stride = sizeof(m_arrVerts[0]);

  switch(ePT) 
  {
    case eSrcPointer_Vert:
      return &m_arrVerts[0].xyz.x;
    case eSrcPointer_Tex:
      return &m_arrVerts[0].st[0];
  }
  return NULL;
}
