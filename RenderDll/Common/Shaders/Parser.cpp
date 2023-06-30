/*=============================================================================
  Parser.cpp : Script parser implementations.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#include "RenderPCH.h"

#define PARSER_CPP

const char *kWhiteSpace = " ,";
char *pCurCommand;

/*----------------------------------------------------------------
  Skips all control characters and any characters in the toSkip
  string.  Changes the buf pointer.
----------------------------------------------------------------*/
void SkipCharacters (char **buf, const char *toSkip)
{
  char theChar;
  char *skip;

  while ((theChar = **buf) != 0)
  {
    if (theChar >= 0x20)        // NOT control character, tab cr, lf, etc
    {
      skip = (char *) toSkip;
      while (*skip)
      {
        if (theChar == *skip)
          break;
        ++skip;
      }
      if (*skip == 0)           // exhausted the skip string
        return;                 // we are now at first NON skipped character
    }
    ++*buf;                     // skip this character
  }
  // we must be at the end of the buffer if we get here
}


_inline static int IsComment(char **buf)
{
  if (!(*buf))
    return 0;
  
  if (**buf == ';')
    return 1;

  if ((*buf)[0] == '/' && (*buf)[1] == '/')
    return 2;

  if ((*buf)[0] == '/' && (*buf)[1] == '*')
    return 3;

  return 0;
}

char *gShObjectNotFound = NULL;


void SkipComments(char **buf, bool bSkipWhiteSpace)
{
  int n;
  static int m;

  while (n=IsComment(buf))
  {
    switch (n)
    {
      case 1:
        // skip comment lines.
        *buf = strchr (*buf, '\n');
        if (*buf && bSkipWhiteSpace)
          SkipCharacters (buf, kWhiteSpace);
        break;

      case 2:
        // skip comment lines.
        *buf = strchr (*buf, '\n');
        if (*buf && bSkipWhiteSpace)
          SkipCharacters (buf, kWhiteSpace);
        break;

      case 3:
        // skip comment blocks.
        m = 0;
        do
        {
          *buf = strchr (*buf, '*');
          if (!(*buf))
            break;
          if ((*buf)[-1] == '/')
          {
            *buf += 1;
            m++;
          }
          else
          if ((*buf)[1] == '/')
          {
            *buf += 2;
            m--;
          }
          else
            *buf += 1;
        } while (m);
        if (!(*buf))
        {
          if (gShObjectNotFound)
            iLog->Log (gShObjectNotFound);
          
          iLog->Log ("Warning: Comment lines don't closed\n");
          break;
        }
        if (bSkipWhiteSpace)
          SkipCharacters (buf, kWhiteSpace);
        break;
    }
  }
}

void RemoveCR(char *pbuf)
{
  while (*pbuf)
  {
    if (*pbuf == 0xd)
      *pbuf = 0x20;
    pbuf++;
  }
}

char *RemoveComments(char *pbuf)
{
  char *ppbuf[1];
  char *prevbuf = pbuf;

  while (true)
  {
    ppbuf[0] = prevbuf;
    SkipCharacters (ppbuf, kWhiteSpace);
    prevbuf = ppbuf[0];
    if (IsComment(ppbuf))
    {
      SkipComments (ppbuf, false);
      if (prevbuf != ppbuf[0])
      {
        memset(prevbuf, 0x20202020, ppbuf[0]-prevbuf);
        prevbuf = ppbuf[0];
      }
    }
    else
    {
      while (prevbuf[0] != ' ' && prevbuf[0] != 9 && prevbuf[0] != 0xa && prevbuf[0] != 0)
      {
        prevbuf++;
      }
    }
    if (!*prevbuf)
      break;
  }

  return pbuf;
}

/*----------------------------------------------------------------
  Pass in a pointer to a buffer of text.  This pointer is modified
  to point to the text after the object description.
  The token id for the object is returned.  The name pointer
  will point to the optional name string in the buffer.
  The data pointer will point to the optional data for
  the object.  The text buffer will get modified so BEWARE.
----------------------------------------------------------------*/
long shGetObject (char **buf, tokenDesc * tokens, char **name, char **data)
{
  if (!*buf)
    return 0;
  
  SkipCharacters (buf, kWhiteSpace);
  SkipComments(buf, true);

  if (!(*buf))
    return -2;

  if (!**buf)                   // at end of file
    return -2;

  // find the token.
  // for now just use brute force.  Improve later.
  tokenDesc *ptokens = tokens;

  while (tokens->id != 0)
  {
    if (!strnicmp (tokens->token, *buf, strlen (tokens->token)))
    {
      // here we could be matching PART of a string
      // we could detect this here or the token list
      // could just have the longer tokens first in the list
      pCurCommand = *buf;
      break;
    }
    ++tokens;
  }
  if (tokens->id == 0)
  {
    char *p = strchr (*buf, '\n');

    char pp[1024];
    if (p)
    {

      strncpy(pp, *buf, p - *buf);
      pp[p - *buf] = 0;

      *buf = p;
    }
    else
      strcpy(pp, *buf);

    if (gShObjectNotFound)
      iLog->Log (gShObjectNotFound);

    iLog->Log ("Warning: Found token '%s' which was not one of the list (Skipping).\n", pp);
    while (ptokens->id != 0)
    {
      iLog->Log ("    %s\n", ptokens->token);
      ptokens++;
    }
    return 0;                  // token not found !error!
  }
  // skip the token
  *buf += strlen (tokens->token);
  SkipCharacters (buf, kWhiteSpace);

  // get optional name
  *name = GetSubText (buf, 0x27, 0x27); // single quotes
  SkipCharacters (buf, kWhiteSpace);

  // get optional data
  if (**buf == '=')             // hmm, this is an assignment and not a command/object
  {
    ++*buf;                       // skip the '='
    *data = GetAssignmentText (buf);
  }
  else
  {
    *data = GetSubText (buf, '(', ')');
    if (!*data)
      *data = GetSubText (buf, '{', '}');
  }


  return tokens->id;
}

static int sRecurse;
static char *sTempBuf1[32];
static char *sTempBuf2;
std::vector<SPair> g_Macros;
static std::vector<SPair> sStaticMacros;
static TArray<bool> sbIFDEF;

static int sFill(char **buf, char *dst)
{
  int n = 0;
  SkipCharacters (buf, kWhiteSpace);
  while (**buf > 0x20)
  {
    dst[n++] = **buf;
    ++*buf;
  }
  dst[n] = 0;
  return n;
}
int fxFill(char **buf, char *dst)
{
  int n = 0;
  SkipCharacters (buf, kWhiteSpace);
  while (**buf != ';')
  {
    if (**buf == 0)
      break;
    dst[n++] = **buf;
    ++*buf;
  }
  dst[n] = 0;
  return n;
}

static int sFillCR(char **buf, char *dst)
{
  int n = 0;
  SkipCharacters (buf, kWhiteSpace);
  while (**buf != 0xa)
  {
    if (**buf == 0)
      break;
    dst[n++] = **buf;
    ++*buf;
  }
  dst[n] = 0;
  return n;
}


static void fxAddMacro(char *Name, char *Macro)
{
  SPair pr;
  pr.MacroName = Name;
  pr.Macro = Macro ? Macro : "";
  sStaticMacros.push_back(pr);
}

void fxParserInit(void)
{
#if defined (DIRECT3D8) || defined (DIRECT3D9)
  fxAddMacro("D3D", NULL);
  fxAddMacro("DIRECT3D", NULL);
#if defined (DIRECT3D8)
  fxAddMacro("DIRECT3D8", NULL);
  fxAddMacro("D3D8", NULL);
#elif defined (DIRECT3D9)
  fxAddMacro("DIRECT3D9", NULL);
  fxAddMacro("D3D9", NULL);
#endif
#elif OPENGL
  fxAddMacro("OGL", NULL);
  fxAddMacro("OPENGL", NULL);
#elif defined (XBOX)
  fxAddMacro("XBOX", NULL);
#elif defined (XBOX2)
  fxAddMacro("XBOX2", NULL);
#elif defined (GC)
  fxAddMacro("GC", NULL);
  fxAddMacro("GAMECUBE", NULL);
#elif defined (PS2)
  fxAddMacro("PS2", NULL);
#elif defined (PS3)
  fxAddMacro("PS3", NULL);
#endif
#if !defined(LINUX)
  if (gRenDev->GetFeatures() & RFT_DEPTHMAPS)
    fxAddMacro("DEPTHMAP", NULL);
  if (gRenDev->GetFeatures() & RFT_SHADOWMAP_SELFSHADOW)
    fxAddMacro("SELFSHADOW", NULL);
  if (gRenDev->GetFeatures() & RFT_HW_ENVBUMPPROJECTED)
    fxAddMacro("PROJECTEDENVBUMP", NULL);
  int nGPU = gRenDev->GetFeatures() & RFT_HW_MASK;
  switch(nGPU)
  {
    case RFT_HW_GF2:
      fxAddMacro("NV1X", NULL);
    	break;
    case RFT_HW_GF3:
      fxAddMacro("NV2X", NULL);
  	  break;
    case RFT_HW_GFFX:
      fxAddMacro("NV3X", NULL);
  	  break;
    case RFT_HW_NV4X:
      fxAddMacro("NV4X", NULL);
  	  break;
    case RFT_HW_RADEON:
      fxAddMacro("R3XX", NULL);
      fxAddMacro("ATI", NULL);
      fxAddMacro("RADEON", NULL);
  	  break;
    default:
      assert(0); 
  }
  if (gRenDev->GetFeatures() & RFT_HW_HDR)
    fxAddMacro("HDR", NULL);
  if (gRenDev->GetFeatures() & RFT_HW_PS30)
    fxAddMacro("PS30", NULL);
  if (gRenDev->GetFeatures() & RFT_HW_PS30)
    fxAddMacro("PS30", NULL);
#endif//LINUX
  char *VPSup = gRenDev->GetVertexProfile(true);
  char *PPSup = gRenDev->GetPixelProfile(true);
  fxAddMacro(VPSup, NULL);
  fxAddMacro(PPSup, NULL);
}

void fxStart()
{
  int i;
  g_Macros.clear();
  sbIFDEF.Free();
  for (i=0; i<32; i++)
  {
    SAFE_DELETE_ARRAY(sTempBuf1[i]);
  }
  SAFE_DELETE_ARRAY(sTempBuf2);
  sRecurse = 0;
}

void fxIncrLevel()
{
  assert (sRecurse < 31);
  sRecurse++;
  sRecurse = min(sRecurse, 31);
  SAFE_DELETE_ARRAY(sTempBuf1[sRecurse]);
}

void fxDecrLevel()
{
  assert (sRecurse > 0);
  SAFE_DELETE_ARRAY(sTempBuf1[sRecurse]);
  sRecurse--;
  sRecurse = max(sRecurse, 0);
}

void fxIgnoreBlock(char **buf)
{
  int nLevel = 0;
  char *start = *buf;
  while (true)
  {
    char *posS = strchr(*buf, '#');
    if (!posS)
    {
      Warning( 0,0,"Couldn't find #endif directive for associated #ifdef");
      return;
    }
    if (posS[1]=='i' && posS[2]=='f')
    {
      nLevel++;
      *buf = posS+2;
      continue;
    }
    if (!strncmp(&posS[1], "endif", 5))
    {
      if (!nLevel)
      {
        *buf = posS+6;
        break;
      }
      nLevel--;
      *buf = posS+4;
    }
    else
      *buf = posS+1;
  }
}

int fxFindMacro(std::vector<SPair>& Macros, char *name)
{
  int i;
  for (i=0; i<Macros.size(); i++)
  {
    if (Macros[i].MacroName == name)
      return i;
  }
  return -1;
}

void fxPreprocess(char **buf, const char *command)
{
  if (!strcmp(&command[1], "include"))
  {
    assert(**buf == '"' || **buf == '<');
    char brak = **buf;
    ++*buf;
    char name[128];
    int n = 0;
    while(**buf != brak)
    {
      if (**buf <= 0x20)
      {
        assert(0);
        break;
      }
      name[n++] = **buf;
      ++*buf;
    }
    if (**buf == brak)
      ++*buf;
    name[n] = 0;
    SkipCharacters (buf, kWhiteSpace);
    SkipComments(buf, true);
    char nameNew[256];
    sprintf(nameNew, "%sCryFX/%s", gRenDev->m_cEF.m_ShadersPath[1], name);
    FILE *fp = iSystem->GetIPak()->FOpen(nameNew, "rb");
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
    int lenInc = iSystem->GetIPak()->FTell(fp);
    iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
    int len = strlen(*buf);
    char *tmpBuf = new char[len+lenInc+1];
    lenInc = iSystem->GetIPak()->FRead(tmpBuf, 1, lenInc, fp);
    iSystem->GetIPak()->FClose(fp);

    tmpBuf[lenInc] = 0;
    RemoveCR(tmpBuf);

    memcpy(&tmpBuf[lenInc], *buf, len);
    tmpBuf[len+lenInc] = 0;
    SAFE_DELETE_ARRAY(sTempBuf1[sRecurse]);
    sTempBuf1[sRecurse] = tmpBuf;
    *buf = tmpBuf;
  }
  else
  if (!strcmp(&command[1], "define"))
  {
    char name[256];
    TArray<char> macro;
    sFill(buf, name);
    while (**buf <= 0x20) {++*buf;}
    while (**buf != 0xa && **buf != 0xd)
    {
      if (**buf == '\\')
      {
        macro.AddElem(0x20);
        while (**buf != '\n') {++*buf;}
        ++*buf;
        continue;
      }
      macro.AddElem(**buf);
      ++*buf;
    }
    macro.AddElem(0);
    SPair pr;
    pr.MacroName = name;
    pr.Macro = &macro[0];
    g_Macros.push_back(pr);
  }
  else
  if (!strcmp(&command[1], "ifdef"))
  {
    char name[256];
    sFill(buf, name);
    int nID;
    nID = fxFindMacro(g_Macros, name);
    if (nID == -1)
      nID = fxFindMacro(sStaticMacros, name);
    if (nID == -1)
    {
      sbIFDEF.AddElem(false);
      fxIgnoreBlock(buf);
    }
    else
      sbIFDEF.AddElem(true);
  }
  else
  if (!strcmp(&command[1], "else"))
  {
    int nLevel = sbIFDEF.Num()-1;
    if (nLevel < 0)
    {
      Warning( 0,0,"#else without #ifdef");
      return;
    }
    if (sbIFDEF[nLevel] == true)
      fxIgnoreBlock(buf);
  }
  else
  if (!strcmp(&command[1], "endif"))
  {
    int nLevel = sbIFDEF.Num()-1;
    if (nLevel < 0)
    {
      Warning( 0,0,"#endif without #ifdef");
      return;
    }
    sbIFDEF.Remove(nLevel);
  }
}

bool fxTranslate(char **buf, const char *command)
{
  int i;
  for (i=0; i<g_Macros.size(); i++)
  {
    if (g_Macros[i].MacroName == command)
    {
      int lenName = strlen(g_Macros[i].MacroName.c_str());
      int lenMacro = strlen(g_Macros[i].Macro.c_str());
      if (lenName < lenMacro)
      {
        int len = strlen(*buf)-lenName;
        char *tmpBuf = new char[len+lenMacro+1];
        memcpy(tmpBuf, g_Macros[i].Macro.c_str(), lenMacro);
        memcpy(&tmpBuf[lenMacro], &(*buf)[lenName], len);
        tmpBuf[len+lenMacro] = 0;
        SAFE_DELETE_ARRAY(sTempBuf1[sRecurse]);
        sTempBuf1[sRecurse] = tmpBuf;
        *buf = tmpBuf;
      }
      else
      {
        memcpy(*buf, g_Macros[i].Macro.c_str(), lenMacro);
        char *b = &(*buf)[lenMacro];
        memset(b, 0x20202020, lenName-lenMacro);
      }
      return true;
    }
  }
  return false;
}

void fxTranslate(char **buf)
{
  int i;
  char *curBuf = *buf;
  while (true)
  {
    char token[256];
    SkipCharacters (buf, kWhiteSpace);
    SkipComments (buf, true);
    if (!**buf)
      break;
    char *tmpBuf = *buf;
    sFill(buf, token);
    for (i=0; i<g_Macros.size(); i++)
    {
      if (g_Macros[i].MacroName == token)
      {
        int lenName = strlen(g_Macros[i].MacroName.c_str());
        int lenMacro = strlen(g_Macros[i].Macro.c_str());
        if (lenName < lenMacro)
        {
          int lenSuff = tmpBuf - curBuf;
          int lenPost = strlen(curBuf)-lenName-lenSuff;
          char *tmp = new char[lenSuff+lenMacro+lenPost+1];
          memcpy(tmp, curBuf, lenSuff);
          memcpy(&tmp[lenSuff], g_Macros[i].Macro.c_str(), lenMacro);
          memcpy(&tmp[lenMacro+lenSuff], &tmpBuf[lenName], lenPost);
          tmp[lenSuff+lenMacro+lenPost] = 0;
          SAFE_DELETE_ARRAY(sTempBuf2);
          sTempBuf2 = tmp;
          curBuf = tmp;
          *buf = &tmp[lenSuff];
        }
        else
        {
          memcpy(tmpBuf, g_Macros[i].Macro.c_str(), lenMacro);
          memset(&tmpBuf[lenMacro], 0x20202020, lenName-lenMacro);
        }
        break;
      }
    }
  }
  *buf = curBuf;
}

long fxGetObject (char **buf, tokenDesc * tokens, char **name, char **data, int &nIndex)
{
  if (!*buf)
    return 0;
  
  SkipCharacters (buf, kWhiteSpace);
  SkipComments(buf, true);

  if (!(*buf))
    return -2;

  if (!**buf)                   // at end of file
    return -2;

  char token[256];
  char *saveBuf = *buf;
  sFill(buf, token);
  *buf = saveBuf;
  if (fxTranslate(buf, token))
  {
    SkipCharacters (buf, kWhiteSpace);
  }

  // find the token.
  // for now just use brute force.  Improve later.
  tokenDesc *ptokens = tokens;

  while (tokens->id != 0)
  {
    if (!strnicmp (tokens->token, *buf, strlen (tokens->token)))
    {
      // here we could be matching PART of a string
      // we could detect this here or the token list
      // could just have the longer tokens first in the list
      pCurCommand = *buf;
      break;
    }
    ++tokens;
  }
  if (tokens->id == 0)
  {
    if (gShObjectNotFound)
      iLog->Log (gShObjectNotFound);

    iLog->Log ("Warning: Found token '%s' which was not one of the list (Skipping).\n", token);
    while (ptokens->id != 0)
    {
      iLog->Log ("    %s\n", ptokens->token);
      ptokens++;
    }
    return 0;                  // token not found !error!
  }
  // skip the token
  *buf += strlen (tokens->token);
  SkipCharacters (buf, kWhiteSpace);
  if (**buf == '[')
  {
    ++*buf;
    SkipCharacters (buf, kWhiteSpace);
    nIndex = atoi(*buf);
    while(**buf != ']') {++*buf;}
    ++*buf;
    SkipCharacters (buf, kWhiteSpace);
  }

  // get optional name
  *name = GetSubText (buf, 0x27, 0x27); // single quotes
  SkipCharacters (buf, kWhiteSpace);

  // get optional data
  if (**buf == '=')             // hmm, this is an assignment and not a command/object
  {
    ++*buf;                       // skip the '='
    *data = fxGetAssignmentText (buf);
  }
  else
  {
    *data = GetSubText (buf, '(', ')');
    if (!*data)
      *data = GetSubText (buf, '{', '}');
  }


  return tokens->id;
}


long fxGetObject(char **buf, SFXTokenDesc *tokens, char **name, char **value, char **data, char **assign, char **annotations, std::vector<SFXStruct>& Structs)
{
  if (!*buf)
    return 0;
  
  // name: name of parameter
  // value: follows '=' assignment
  // data: inside '{' '}' brackets
  // assign: follows ':' assignment
  // annotations: inside '<' '>' brackets

  // find the token.
  // for now just use brute force.  Improve later.
  SFXTokenDesc *ptokens = tokens;
  char *tmpBuf;
  char Token[256];
  while (true)
  {
    SkipCharacters (buf, kWhiteSpace);
    SkipComments(buf, true);
    bool bFunction = false;

    if (!(*buf))
      return -2;

    if (!**buf)                   // at end of file
      return -2;

    tmpBuf = *buf;
    sFill(buf, Token);
    SkipCharacters (buf, kWhiteSpace);
    SkipComments(buf, true);
    bool bFound = false;
    if (Token[0] == '#')
    {
      if (Token[1] > 0x20)
        fxPreprocess(buf, Token);
      else
      {
        *buf = tmpBuf;
        sFillCR(buf, Token);
        SFXStruct str;
        str.m_Name = "";
        str.m_Struct = Token;
        Structs.push_back(str);
      }
    }
    else
    {
      *buf = tmpBuf;
      // Check for function
      while (true)
      {
        // Check for vertex/pixel shader/function
        SkipCharacters(buf, kWhiteSpace);
        SkipComments(buf, kWhiteSpace);
        if (!**buf)
          break;
        char szOut[128];
        char *saveBuf = *buf;
        int i = 0;
        while (**buf > 0x20) { szOut[i++] = **buf; ++*buf; }
        szOut[i] = 0;
        SkipCharacters(buf, kWhiteSpace);
        while (**buf != '(' && **buf > 0x20) { ++*buf; }
        if (**buf <= 0x20)
          SkipCharacters(buf, kWhiteSpace);
        if (**buf == '(')
        {
          char *s = strchr(*buf, '{');
          assert(s);
          if (s)
          {
            int nRecurse = 0;
            while (*s >= 0)
            {
              if (*s == '{')
                nRecurse++;
              else
              if (*s == '}')
              {
                nRecurse--;
                if (nRecurse == 0)
                  break;
              }
              s++;
            }
          }
          if (s)
          {
            char *sBuf = saveBuf;
            s[1] = 0;
            SFXStruct str;
            str.m_Name = sGetFuncName(sBuf);
            str.m_Struct = sBuf;
            Structs.push_back(str);
            *buf = s+2;
            bFunction = true;
          }
        }
        else
        {
          *buf = saveBuf;
          break;
        }
      }
      if (!bFunction)
      {
        bFound = fxTranslate(buf, Token);
        if (!bFound)
          break;
      }
    }
  }

  int len;
  while (tokens->id != 0)
  {
    len = strlen(tokens->token);
    if (!strnicmp (tokens->token, Token, len))
    {
      pCurCommand = *buf;
      break;
    }
    tokens++;
  }
  if (tokens->id == 0)
  {

    if (gShObjectNotFound)
      iLog->Log (gShObjectNotFound);

    iLog->Log ("Warning: FX parser found token '%s' which was not one of the list (Skipping).\n", Token);
    while (ptokens->id != 0)
    {
      iLog->Log ("    %s\n", ptokens->token);
      ptokens++;
    }
    return 0;                  // token not found !error!
  }
  if (tokens->id < 0)
  {
    *data = tmpBuf;
    while (**buf != 0xa && **buf != 0xd && **buf != 0)
    {
      ++*buf;
    }
    **buf = 0;
    ++*buf;
    fxTranslate(data);
    return -tokens->id;
  }
  // skip the token
  *buf += len;
  while (**buf > 0x20) { ++*buf; }
  SkipCharacters (buf, kWhiteSpace);
  *name = GetAssignmentText(buf);
  SkipCharacters (buf, kWhiteSpace);
  if (**buf == ':')
  {
    ++*buf;                       // skip the ':'
    *assign = GetAssignmentText(buf);
  }
  else
    *assign = 0;

  // get optional annotations
  SkipCharacters (buf, kWhiteSpace);
  *annotations = GetSubText (buf, '<', '>'); 
  SkipCharacters (buf, kWhiteSpace);

  // get optional data
  if (**buf == '=')             // hmm, this is an assignment and not a command/object
  {
    ++*buf;                       // skip the '='
    *value = fxGetAssignmentText2 (buf);
  }
  else
    *value = NULL;
  SkipCharacters (buf, kWhiteSpace);
  char *d = GetSubText (buf, '{', '}');
  //if (d)
  //  fxTranslate(&d);
  *data = d;
  if (!*data)
    *data = GetSubText (buf, '(', ')');

  return tokens->id;
}

void fxGetParams(char *annot, std::vector <SFXParam>& prms)
{
  char *buf = annot;
  SFXParam prm;
  while (buf[0])
  {
    SkipCharacters (&buf, kWhiteSpace);
    if (!buf[0])
      break;
    if (!strnicmp(buf, "string", sizeof("string")-1))
      prm.m_Type = eType_STRING;
    else
    if (!strnicmp(buf, "float", sizeof("float")-1))
      prm.m_Type = eType_FLOAT;
    else
    if (!strnicmp(buf, "int", sizeof("int")-1))
      prm.m_Type = eType_INT;
    else
    if (!strnicmp(buf, "bool", sizeof("bool")-1))
      prm.m_Type = eType_BOOL;
    while (*buf > 0x20) { ++buf; }
    SkipCharacters (&buf, kWhiteSpace);
    prm.m_Name = GetAssignmentText(&buf);
    SkipCharacters (&buf, kWhiteSpace);
    if (buf[0] == '=')
    {
      buf++;
      SkipCharacters (&buf, kWhiteSpace);
      if (buf[0] == '"')
      {
        prm.m_Value.m_String = &buf[1];
        char *s = strchr(&buf[1], '"');
        s[0] = 0;
        buf = s+1;
      }
      else
      {
        switch(prm.m_Type)
        {
          case eType_FLOAT:
            prm.m_Value.m_Float = shGetFloat(buf);
        	  break;
          case eType_INT:
            prm.m_Value.m_Float = shGetInt(buf);
        	  break;
          default:
            assert(0);
        }
        while (*buf > 0x20) { ++buf; }
      }
    }
    else
      prm.m_Value.m_Int = 0;
    prms.push_back(prm);
    SkipCharacters (&buf, kWhiteSpace);
    assert (buf[0] == ';');
    buf++;
  }
}

/*----------------------------------------------------------------
  Pass in a pointer to a buffer of text.  This pointer is modified
  to point to the text after the command description.
  The token id for the command is returned.
  The params pointer will point to the optional data for
  the object.  The text buffer will get modified so BEWARE.
----------------------------------------------------------------*/
long shGetCommand (char **buf, tokenDesc * tokens, char **params)
{
  char *name;

  return shGetObject (buf, tokens, &name, params);
}

/*----------------------------------------------------------------
  Returns the string of text between the open and close
  characters.  Modifies the buffer.  Moves the buffer pointer
  to after the last delimiter.  Can return nil;
  buffer MUST already be at the opening delimiter.
  Skips nested delimiters too.
  NOTE: Should skip quoted text, does not at this time.
----------------------------------------------------------------*/
char *GetSubText (char **buf, char open, char close)
{
  if (**buf == 0 || **buf != open)
    return 0;
  ++*buf;                       // skip opening delimiter
  char *result = *buf;

  // now find the closing delimiter
  char theChar;
  long skip = 1;

  if (open == close)            // we cant nest identical delimiters
    open = 0;
  while ((theChar = **buf) != 0)
  {
    if (theChar == open)
      ++skip;
    if (theChar == close)
    {
      if (--skip == 0)
      {
        **buf = 0;              // null terminate the result string
        ++*buf;                 // move past the closing delimiter
        break;
      }
    }
    ++*buf;                     // try next character
  }
  return result;
}

int shGetVar (char **buf, char **vr, char **val)
{
  static char v[128];

  SkipCharacters (buf, kWhiteSpace);
  SkipComments(buf, true);

  if (!**buf)                   // at end of file
    return -2;

  int i = 0;
  v[0] = 0;
  char c;
  while (c=(*buf)[i])
  {
    if (c == 0x20 || c == '=')
    {
      strncpy(v, *buf, i);
      v[i] = 0;
      break;
    }
    i++;
  }
  if (!v[0])
    return 0;

  int ret = 1;

  *vr = v;

  // skip the token
  *buf += strlen (v);
  SkipCharacters (buf, kWhiteSpace);

  // get optional data
  if (**buf == '=')             // hmm, this is an assignment and not a command/object
  {
    ++*buf;                       // skip the '='
    *val = GetAssignmentText (buf);
  }
  else
  {
    *val = GetSubText (buf, '(', ')');
    if (!*val)
      *val = GetSubText (buf, '{', '}');
  }

  return ret;
}


/*----------------------------------------------------------------
  Returns the string of text after a = up to the next
  whitespace.  Terminates the string and moves the buf pointer.
----------------------------------------------------------------*/
char *GetAssignmentText (char **buf)
{
  SkipCharacters (buf, kWhiteSpace);
  char *result = *buf;

  // now, we need to find the next whitespace to end the data
  char theChar;

  while ((theChar = **buf) != 0)
  {
    if (theChar <= 0x20 || theChar == ';')        // space and control chars
      break;
    ++*buf;
  }
  **buf = 0;                    // null terminate it
  if (theChar)                  // skip the terminator
    ++*buf;
  return result;
}

char *fxGetAssignmentText (char **buf)
{
  SkipCharacters (buf, kWhiteSpace);
  char *result = *buf;

  // now, we need to find the next ';' to end the data
  char theChar;

  while ((theChar = **buf) != 0)
  {
    if (theChar == ';')        // line terminate
      break;
    ++*buf;
  }
  **buf = 0;                    // null terminate it
  if (theChar)                  // skip the terminator
    ++*buf;
  return result;
}

char *fxGetAssignmentText2 (char **buf)
{
  SkipCharacters (buf, kWhiteSpace);
  char *result = *buf;

  // now, we need to find the next ';' to end the data
  char theChar;

  if (**buf == '{')
  {
    while ((theChar = **buf) != 0)
    {
      if (theChar == ';')        // line terminate
        break;
      ++*buf;
    }
  }
  else
  {
    while ((theChar = **buf) != 0)
    {
      if (theChar <= 0x20)        // space and control chars
        break;
      ++*buf;
    }
  }
  **buf = 0;                    // null terminate it
  if (theChar)                  // skip the terminator
    ++*buf;
  return result;
}

bool shGetBool(char *buf)
{
  if (!buf)
    return false;

  if ( !strnicmp(buf, "yes", 3) )
    return true;

  if ( !strnicmp(buf, "true", 4) )
    return true;

  if ( !strnicmp(buf, "on", 2) )
    return true;

  if ( !strncmp(buf, "1", 1) )
    return true;

  return false;
}

float shGetFloat(char *buf)
{
  if (!buf)
    return 0;
  float f;

  sscanf(buf, "%f", &f);

  return f;
}

void shGetFloat(char *buf, float *v1, float *v2)
{
  if (!buf)
    return;
  float f, f1;

  int n = sscanf(buf, "%f %f", &f, &f1);
  if (n == 1)
  {
    *v1 = f;
    *v2 = f;
  }
  else
  {
    *v1 = f;
    *v2 = f1;
  }
}

int shGetInt(char *buf)
{
  if (!buf)
    return 0;
  int i;

  sscanf(buf, "%i", &i);

  return i;
}

int shGetHex(const char *buf)
{
  if (!buf)
    return 0;
  int i;

  sscanf(buf, "%x", &i);

  return i;
}

uint64 shGetHex64(const char *buf)
{
  if (!buf)
    return 0;
  uint64 i;

  sscanf(buf, "%I64x", &i);

  return i;
}

void shGetVector(char *buf, Vec3d& v)
{
  if (!buf)
    return;
  sscanf(buf, "%f %f %f", &v[0], &v[1], &v[2]);
}

void shGetVector(char *buf, float v[3])
{
  if (!buf)
    return;
  sscanf(buf, "%f %f %f", &v[0], &v[1], &v[2]);
}

void shGetVector4(char *buf, vec4_t& v)
{
  if (!buf)
    return;
  sscanf(buf, "%f %f %f %f", &v[0], &v[1], &v[2], &v[3]);
}

static struct SColAsc
{
  char *nam;
  CFColor col;

  SColAsc(char *name, const CFColor& c)
  {
    nam = name;
    col = c;
  }
} sCols[] =
{
  SColAsc("Aquamarine", Col_Aquamarine),
  SColAsc("Black", Col_Black),
  SColAsc("Blue", Col_Blue),
  SColAsc("BlueViolet", Col_BlueViolet),
  SColAsc("Brown", Col_Brown),
  SColAsc("CadetBlue", Col_CadetBlue),
  SColAsc("Coral", Col_Coral),
  SColAsc("CornflowerBlue", Col_CornflowerBlue),
  SColAsc("Cyan", Col_Cyan),
  SColAsc("DarkGreen", Col_DarkGreen),
  SColAsc("DarkOliveGreen", Col_DarkOliveGreen),
  SColAsc("DarkOrchild", Col_DarkOrchild),
  SColAsc("DarkSlateBlue", Col_DarkSlateBlue),
  SColAsc("DarkSlateGray", Col_DarkSlateGray),
  SColAsc("DarkSlateGrey", Col_DarkSlateGrey),
  SColAsc("DarkTurquoise", Col_DarkTurquoise),
  SColAsc("DarkWood", Col_DarkWood),
  SColAsc("DimGray", Col_DimGray),
  SColAsc("DimGrey", Col_DimGrey),
  SColAsc("FireBrick", Col_FireBrick),
  SColAsc("ForestGreen", Col_ForestGreen),
  SColAsc("Gold", Col_Gold),
  SColAsc("Goldenrod", Col_Goldenrod),
  SColAsc("Gray", Col_Gray),
  SColAsc("Green", Col_Green),
  SColAsc("GreenYellow", Col_GreenYellow),
  SColAsc("Grey", Col_Grey),
  SColAsc("IndianRed", Col_IndianRed),
  SColAsc("Khaki", Col_Khaki),
  SColAsc("LightBlue", Col_LightBlue),
  SColAsc("LightGray", Col_LightGray),
  SColAsc("LightGrey", Col_LightGrey),
  SColAsc("LightSteelBlue", Col_LightSteelBlue),
  SColAsc("LightWood", Col_LightWood),
  SColAsc("LimeGreen", Col_LimeGreen),
  SColAsc("Magenta", Col_Magenta),
  SColAsc("Maroon", Col_Maroon),
  SColAsc("MedianWood", Col_MedianWood),
  SColAsc("MediumAquamarine", Col_MediumAquamarine),
  SColAsc("MediumBlue", Col_MediumBlue),
  SColAsc("MediumForestGreen", Col_MediumForestGreen),
  SColAsc("MediumGoldenrod", Col_MediumGoldenrod),
  SColAsc("MediumOrchid", Col_MediumOrchid),
  SColAsc("MediumSeaGreen", Col_MediumSeaGreen),
  SColAsc("MediumSlateBlue", Col_MediumSlateBlue),
  SColAsc("MediumSpringGreen", Col_MediumSpringGreen),
  SColAsc("MediumTurquoise", Col_MediumTurquoise),
  SColAsc("MediumVioletRed", Col_MediumVioletRed),
  SColAsc("MidnightBlue", Col_MidnightBlue),
  SColAsc("Navy", Col_Navy),
  SColAsc("NavyBlue", Col_NavyBlue),
  SColAsc("Orange", Col_Orange),
  SColAsc("OrangeRed", Col_OrangeRed),
  SColAsc("Orchid", Col_Orchid),
  SColAsc("PaleGreen", Col_PaleGreen),
  SColAsc("Pink", Col_Pink),
  SColAsc("Plum", Col_Plum),
  SColAsc("Red", Col_Red),
  SColAsc("Salmon", Col_Salmon),
  SColAsc("SeaGreen", Col_SeaGreen),
  SColAsc("Sienna", Col_Sienna),
  SColAsc("SkyBlue", Col_SkyBlue),
  SColAsc("SlateBlue", Col_SlateBlue),
  SColAsc("SpringGreen", Col_SpringGreen),
  SColAsc("SteelBlue", Col_SteelBlue),
  SColAsc("Tan", Col_Tan),
  SColAsc("Thistle", Col_Thistle),
  SColAsc("Turquoise", Col_Turquoise),
  SColAsc("Violet", Col_Violet),
  SColAsc("VioletRed", Col_VioletRed),
  SColAsc("Wheat", Col_Wheat),
  SColAsc("White", Col_White),
  SColAsc("Yellow", Col_Yellow),
  SColAsc("YellowGreen", Col_YellowGreen),

  SColAsc(NULL, CFColor(1.0f,1.0f,1.0f))
};

#include <ctype.h>

void shGetColor(char *buf, CFColor& v)
{
  char name[64];

  if (!buf)
  {
    v = Col_White;
    return;
  }
  if (isalpha(buf[0]))
  {
    int n = 0;
    float scal = 1;
    strcpy(name, buf);
    char nm[64];
    if (strchr(buf, '*'))
    {
      while (buf[n] != '*')
      {
        if (buf[n] == 0x20)
          break;
        nm[n] = buf[n];
        n++;
      }
      nm[n] = 0;
      if (buf[n] == 0x20)
      {
        while (buf[n] != '*')
          n++;
      }
      n++;
      while (buf[n] == 0x20)
        n++;
      scal = shGetFloat(&buf[n]);
      strcpy(name, nm);
    }
    n = 0;
    while (sCols[n].nam)
    {
      if (!stricmp(sCols[n].nam, name))
      {
        v = sCols[n].col;
        if (scal != 1)
          v.ScaleCol(scal);
        return;
      }
      n++;
    }
  }
  int n = sscanf(buf, "%f %f %f %f", &v.r, &v.g, &v.b, &v.a);
  switch (n)
  {
    case 0:
      v.r = v.g = v.b = v.a = 1.0f;
      break;

    case 1:
      v.g = v.b = v.a = 1.0f;
      break;

    case 2:
      v.b = v.a = 1.0f;
      break;

    case 3:
      v.a = 1.0f;
      break;
  }
  //v.Clamp();
}

void shGetColor(char *buf, float v[4])
{
  char name[64];

  if (!buf)
  {
    v[0] = 1.0f;
    v[1] = 1.0f;
    v[2] = 1.0f;
    v[3] = 1.0f;
    return;
  }
  if (isalpha(buf[0]))
  {
    int n = 0;
    float scal = 1;
    strcpy(name, buf);
    char nm[64];
    if (strchr(buf, '*'))
    {
      while (buf[n] != '*')
      {
        if (buf[n] == 0x20)
          break;
        nm[n] = buf[n];
        n++;
      }
      nm[n] = 0;
      if (buf[n] == 0x20)
      {
        while (buf[n] != '*')
          n++;
      }
      n++;
      while (buf[n] == 0x20)
        n++;
      scal = shGetFloat(&buf[n]);
      strcpy(name, nm);
    }
    n = 0;
    while (sCols[n].nam)
    {
      if (!stricmp(sCols[n].nam, name))
      {
        v[0] = sCols[n].col[0];
        v[1] = sCols[n].col[1];
        v[2] = sCols[n].col[2];
        v[3] = sCols[n].col[3];
        if (scal != 1)
        {
          v[0] *= scal;
          v[1] *= scal;
          v[2] *= scal;
        }
        return;
      }
      n++;
    }
  }
  int n = sscanf(buf, "%f %f %f %f", &v[0], &v[1], &v[2], &v[3]);
  switch (n)
  {
    case 0:
      v[0] = v[1] = v[2] = v[3] = 1.0f;
      break;

    case 1:
      v[1] = v[2] = v[3] = 1.0f;
      break;

    case 2:
      v[2] = v[3] = 1.0f;
      break;

    case 3:
      v[3] = 1.0f;
      break;
  }
  //v.Clamp();
}

char *fxReplaceInText(char *pText, const char *pSubStr, const char *pReplace)
{
  char *pTextIn = pText;
  int nLenSubStr = strlen(pSubStr);
  int nLenReplace = strlen(pReplace);
  char *pOut = NULL;
  int nCurSearch = 0;
  while (true)
  {
    char *Str = strstr(&pText[nCurSearch], pSubStr);
    if (!Str)
      return pText;
    if (nLenSubStr == nLenReplace)
    {
      memcpy(Str, pReplace, nLenReplace);
      continue;
    }
    int nLenText = strlen(pText);
    int nNewLen = nLenText-nLenSubStr+nLenReplace;
    pOut = new char[nNewLen+1];
    int nOffs = Str-pText;
    memcpy(pOut, pText, nOffs);
    memcpy(&pOut[nOffs],pReplace,nLenReplace);
    memcpy(&pOut[nOffs+nLenReplace],&pText[nOffs+nLenSubStr],nLenText-nOffs-nLenSubStr);
    pOut[nNewLen] = 0;

    if (pText && pText != pTextIn)
      delete [] pText;

    pText = pOut;
    nCurSearch = nOffs + nLenReplace;
  }
}

char *fxReplaceInText2(char *pText, const char *pSubStr, const char *pReplace)
{
  char *pTextIn = pText;
  int nLenSubStr = strlen(pSubStr);
  int nLenReplace = strlen(pReplace);
  char *pOut = NULL;
  int nCurSearch = 0;
  while (true)
  {
    char *Str = strstr(&pText[nCurSearch], pSubStr);
    if (!Str)
      return pText;
    if ((Str[-1] >= 0x30 && Str[-1] <= 0x7f) || (Str[nLenSubStr] >= 0x30 && Str[nLenSubStr] <= 0x7f))
    {
      if (Str[-1] != ';' && Str[nLenSubStr] != ';')
      {
        nCurSearch += nLenSubStr;
        continue;
      }
    }
    if (nLenSubStr == nLenReplace)
    {
      memcpy(Str, pReplace, nLenReplace);
      continue;
    }
    int nLenText = strlen(pText);
    int nNewLen = nLenText-nLenSubStr+nLenReplace;
    pOut = new char[nNewLen+1];
    int nOffs = Str-pText;
    memcpy(pOut, pText, nOffs);
    memcpy(&pOut[nOffs],pReplace,nLenReplace);
    memcpy(&pOut[nOffs+nLenReplace],&pText[nOffs+nLenSubStr],nLenText-nOffs-nLenSubStr);
    pOut[nNewLen] = 0;

    if (pText && pText != pTextIn)
      delete [] pText;

    pText = pOut;
    nCurSearch = nOffs + nLenReplace;
  }
}
