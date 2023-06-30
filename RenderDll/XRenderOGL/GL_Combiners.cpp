
#include "RenderPCH.h"
#include "GL_Renderer.h"

#include "NVParse\nvparse.h"

void build_normalize_combiner_dlist()
{
glEnable(GL_REGISTER_COMBINERS_NV);

float specular [4]={1,0,0.0,1};
glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV,&specular[0]);

//float diffuse [4]={1,1,1,1};
//glCombinerParameterfvNV(GL_CONSTANT_COLOR1_NV,&diffuse[0]);

nvparse(false,

"!!RC1.0"
/*
"{"
  "rgb{"
  "spare0=expand(col1).expand(col1);"
  "}"
"}"

"{"
  "rgb{"
  "discard=expand(col1);"
  "discard=half_bias(col1)*unsigned_invert(spare0);"
  "col1=sum();"
  "}"
"}"
*/
"{"
  "rgb{"
  "spare0=expand(tex2).expand(tex1);" // specular
  "spare1=expand(tex0).expand(tex1);" // diffuse
  "}"
"}"

// specular exp

"{"
  "rgb{"
  "spare0=spare0*spare0;"
  "}"
"}"

"{"
  "rgb{"
  "spare0=spare0*spare0;"
  "}"
"}"

"{"
  "rgb{"
  "spare0=spare0*spare0;"
  "}"
"}"

"{"
  "rgb{"
  "spare0=spare0*spare0;"
  "}"
"}"

"{"
  "rgb{"
  "spare0=spare0*spare0;"
  "}"
"}"

// attenuation
"{"
  "rgb{"
  "spare1=spare1*col0.a;"
  "}"
"}"

"{"
  "rgb{"
  "spare0=spare0*col0.a;"
  "}"
"}"

// summ result
"final_product = const0 * spare1;\n"
"out.rgb=spare0 + final_product;"
"out.a=unsigned_invert(zero);"

);
char*const*const szError=nvparse_get_errors();
if(szError[0])
  iConsole->Exit(szError[0]);
}
/*
void CGLRenderer::EnableBumpCombinersAdditive()
{
build_normalize_combiner_dlist();
return;

//Makefunctionsfirst
//GET_GL_PROC(PFNGLCOMBINERPARAMETERINVPROC,glCombinerParameteriNV);
//GET_GL_PROC(PFNGLCOMBINERPARAMETERFVNVPROC,glCombinerParameterfvNV);
//GET_GL_PROC(PFNGLCOMBINERINPUTNVPROC,glCombinerInputNV);
//GET_GL_PROC(PFNGLCOMBINEROUTPUTNVPROC,glCombinerOutputNV);
//GET_GL_PROC(PFNGLFINALCOMBINERINPUTNVPROC,glFinalCombinerInputNV);

//floatcon0[]={bright,bright,bright,bright};
//floatcon1[]={ambient,ambient,ambient,1};

glEnable(GL_REGISTER_COMBINERS_NV);

//glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV,con0);
//glCombinerParameterfvNV(GL_CONSTANT_COLOR1_NV,con1);

glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV,1);

//Stage0:
glCombinerInputNV(GL_COMBINER0_NV,GL_RGB,GL_VARIABLE_A_NV,
GL_TEXTURE0_ARB,GL_EXPAND_NORMAL_NV,GL_RGB);

glCombinerInputNV(GL_COMBINER0_NV,GL_RGB,GL_VARIABLE_B_NV,
GL_SECONDARY_COLOR_NV,GL_EXPAND_NORMAL_NV,GL_RGB);

glCombinerInputNV(GL_COMBINER0_NV,GL_RGB,GL_VARIABLE_C_NV,
GL_ZERO,GL_UNSIGNED_INVERT_NV,GL_RGB);

glCombinerInputNV(GL_COMBINER0_NV,GL_RGB,GL_VARIABLE_D_NV,
GL_PRIMARY_COLOR_NV,GL_UNSIGNED_IDENTITY_NV,GL_ALPHA);

glCombinerOutputNV(GL_COMBINER0_NV,GL_RGB,
GL_SPARE0_NV,GL_SPARE1_NV,GL_DISCARD_NV,
GL_NONE,GL_NONE,GL_TRUE,GL_FALSE,GL_FALSE);

glCombinerOutputNV(GL_COMBINER1_NV,GL_RGB,
GL_DISCARD_NV,GL_DISCARD_NV,GL_DISCARD_NV,
GL_NONE,GL_NONE,GL_FALSE,GL_FALSE,GL_FALSE);

//Finalstage
glFinalCombinerInputNV(GL_VARIABLE_A_NV,
GL_E_TIMES_F_NV,GL_UNSIGNED_IDENTITY_NV,GL_RGB);
glFinalCombinerInputNV(GL_VARIABLE_B_NV,
GL_SPARE1_NV,GL_UNSIGNED_IDENTITY_NV,GL_RGB);
glFinalCombinerInputNV(GL_VARIABLE_C_NV,
GL_ZERO,GL_UNSIGNED_IDENTITY_NV,GL_RGB);

glFinalCombinerInputNV(GL_VARIABLE_E_NV,
GL_SPARE0_NV,GL_UNSIGNED_IDENTITY_NV,GL_RGB);
glFinalCombinerInputNV(GL_VARIABLE_F_NV,
//GL_TEXTURE1_ARB,GL_UNSIGNED_IDENTITY_NV,GL_RGB);
GL_ZERO,GL_UNSIGNED_INVERT_NV,GL_RGB);
glFinalCombinerInputNV(GL_VARIABLE_D_NV,
GL_ZERO,GL_UNSIGNED_IDENTITY_NV,GL_RGB);
}*/

//extern "C" char * state_to_rc10 ();

void CGLRenderer::ConfigCombinersForHardwareShadowPass(int withTexture,float*lightDimColor)
{
  static int nList = 0;

  CPShader::m_CurRC = 0;

  if(nList)
  {
    glCallList(nList);
    return;
  }

  nList = glGenLists(1);
  
  glNewList(nList, GL_COMPILE);

  nvparse(false,
    "!!RC1.0\n"
    "{"
      "rgb{"
      "spare0=unsigned_invert(zero);"
      "}"
      "alpha{"
      "spare0.a=unsigned_invert(tex0.b)*col0.a;"
      "}"
    "}"
    "out.rgb = unsigned_invert(col0.a) + tex0;\n"
    "out.a = spare0.a;\n"
    );

  glEnable(GL_REGISTER_COMBINERS_NV);

  glEndList();

  int n = 0;

  for (char * const * errors = nvparse_get_errors(); *errors; errors++)
  {
    n++;
    iLog->Log(*errors);
  }
}
