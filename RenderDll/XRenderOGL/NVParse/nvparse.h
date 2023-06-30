#ifndef NVPARSE_H
#define NVPARSE_H

#define NVPARSE 1

#include "nvparse_errors.h"
#include "nvparse_externs.h"
#include "../GL_Renderer.h"

void nvparse(bool m_bPerStage, const char * input_string, ...);
char * const * const nvparse_get_errors();

extern TArray<SCGBindConst> gParseConsts;

//======================================================================

#endif
