/*=============================================================================
  Parser.h : Script parser declarations.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#ifndef PARSER_H
#define PARSER_H

extern char *pCurCommand;

/**
 * Structure to describe a single token
 */
typedef struct
{
  /// value returned when token is matched
  long  id;
  /// token to match
  char  *token;
} tokenDesc;

struct SFXTokenDesc
{
  int id;
  char *token;
};


extern const char *kWhiteSpace;


/// Get an object from the buffer.
long shGetObject(char **buf, tokenDesc *tokens, char **name, char **data);
/// Get a command from the buffer.
long shGetCommand(char **buf, tokenDesc *tokens, char **params);

extern std::vector<SPair> g_Macros;

long fxGetObject(char **buf, tokenDesc *tokens, char **name, char **data, int &nIndex);
long fxGetObject(char **buf, SFXTokenDesc *tokens, char **name, char **value, char **data, char **assign, char **annotations, std::vector<SFXStruct>& Structs);
void fxGetParams(char *annot, std::vector<SFXParam>& prms);
void fxParserInit(void);
void fxStart(void);
void fxIncrLevel();
void fxDecrLevel();
int fxFill(char **buf, char *dst);

void fxTranslate(char **buf);
char *fxGetAssignmentText(char **buf);
char *fxGetAssignmentText2(char **buf);

char *GetSubText(char **buf, char open, char close);
void SkipCharacters(char **buf, const char *toSkip);
char *GetAssignmentText(char **buf);
void SkipComments(char **buf, bool bSkipWhiteSpace);
char *RemoveComments(char *buf);
void RemoveCR(char *buf);

bool shGetBool(char *buf);
float shGetFloat(char *buf);
void shGetFloat(char *buf, float *v1, float *v2);
int shGetInt(char *buf);
int shGetHex(const char *buf);
uint64 shGetHex64(const char *buf);
void shGetVector(char *buf, Vec3d& v);
void shGetVector(char *buf, float v[3]);
void shGetVector4(char *buf, vec4_t& v);
void shGetColor(char *buf, CFColor& v);
void shGetColor(char *buf, float v[4]);
int shGetVar (char **buf, char **vr, char **val);

char *fxReplaceInText(char *pText, const char *pSubStr, const char *pReplace);
char *fxReplaceInText2(char *pText, const char *pSubStr, const char *pReplace);

#endif

