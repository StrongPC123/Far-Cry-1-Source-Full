// ResFile.cpp : implementation file
//

#include "RenderPCH.h"
#include "ResFile.h"

#include "lzss.h"

#include <sys/stat.h>

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#include <io.h>
#elif defined(LINUX)

#endif

CResFile CResFile::m_Root;
int CResFile::m_nNumOpenResources=0;

void CResFile::mfDeactivate()
{
  Unlink();
  if (m_handle)
  {
    iSystem->GetIPak()->FClose(m_handle);
    m_handle = NULL;
    m_nNumOpenResources--;
  }
}

bool CResFile::mfActivate(bool bFirstTime)
{
  Relink(&m_Root);
  if (m_handle)
    return true;
  if (m_nNumOpenResources > MAX_OPEN_RESFILES)
  {
    CResFile *rf = m_Root.m_Prev;
    assert(rf && rf->m_handle);
    rf->mfDeactivate();
  }
  if (!bFirstTime && m_szAccess[0] == 'w')
  {
    char szAcc[16];
    strcpy(szAcc, m_szAccess);
    szAcc[0] = 'r';
    m_handle = iSystem->GetIPak()->FOpen(m_name, szAcc);
  }
  else
    m_handle = iSystem->GetIPak()->FOpen(m_name, m_szAccess);
  if (!m_handle)
  {
    sprintf(m_ermes, "CResFile::Activate - Can't open resource file <%s>", m_name);
    Unlink();
    return false;
  }
  m_nNumOpenResources++;
  return true;
}

CResFile::CResFile()
{
  m_name[0] = 0;
  m_handle = NULL;
  m_ermes[0] = 0;
  m_holesSize = 0;
  m_eDT = eFSD_name;

  m_Next = NULL;
  m_Prev = NULL;

  if (!m_Root.m_Next)
  {
    m_Root.m_Next = &m_Root;
    m_Root.m_Prev = &m_Root;
  }
}

CResFile::CResFile(const char* name, EFS_DirType eDT)
{
  strcpy(m_name, name);
  m_handle = NULL;
  m_ermes[0] = 0;
  m_holesSize = 0;
  m_eDT = eDT;

  m_Next = NULL;
  m_Prev = NULL;

  if (!m_Root.m_Next)
  {
    m_Root.m_Next = &m_Root;
    m_Root.m_Prev = &m_Root;
  }
}

CResFile::~CResFile()
{
  mfClose();
}

void CResFile::mfSetError(const char *er)
{
  strcpy(m_ermes, er);
}

char* CResFile::mfGetError(void)
{
  if (m_ermes[0])
    return m_ermes;
  else
    return NULL;
}

int CResFile::mfGetResourceSize()
{
  if (!m_handle)
    return 0;
  iSystem->GetIPak()->FSeek(m_handle, 0, SEEK_END);
  int length = iSystem->GetIPak()->FTell(m_handle);
  iSystem->GetIPak()->FSeek(m_handle, 0, SEEK_SET);

  return length;
}

int CResFile::mfGetHolesSize()
{
  if (!m_handle)
    return 0;
  return m_holesSize;
}

int CResFile::mfFileExist(int ID)
{
  SDirEntry *de = mfGetEntry(ID);
  if (!de)
    return -1;
  assert(ID == de->ID);
  return de->ID;
}

int CResFile::mfFileExist(const char* name)
{
  CName nm = CName(name, eFN_Find);
  if (!nm.GetIndex())
    return -1;
  SDirEntry *de = mfGetEntry(nm.GetIndex());
  if (!de)
    return -1;
  return de->ID;
}

bool CResFile::mfOpen(int type)
{
  SFileResHeader frh;
  SFileDirEntry_0 fde0;
  SFileDirEntry_1 fde1;

  if (m_name[0] == 0)
  {
    strcpy (m_ermes, "CResFile::Open - No Resource name");
    return false;
  }
  if (type == RA_READ)
    m_szAccess = "rb";
  else
  if (type == (RA_WRITE|RA_READ))
    m_szAccess = "r+b";
  else
  if (type & RA_CREATE)
    m_szAccess = "w+b";
  else
  {
    sprintf (m_ermes, "CResFile::Open - Error access mode for file <%s>", m_name);
    return false;
  }
  mfActivate(true);
  if (!m_handle)
  {
    if (type & (RA_WRITE | RA_CREATE))
    {
      FILE *statusdst = fopen(m_name, "rb");
      if (statusdst)
      {
#if defined(LINUX)
        struct stat64 st;
#else
        struct __stat64 st;
#endif
        int result = _fstat64(fileno(statusdst), &st);
        if (result == 0)
        {
#if defined(LINUX)
          if (st.st_mode & FILE_ATTRIBUTE_READONLY)
            chmod(m_name, 0x777);//set to full access
#else
          if (!(st.st_mode & _S_IWRITE))
            _chmod(m_name, _S_IREAD | _S_IWRITE);
#endif
        }
        fclose(statusdst);
        m_ermes[0] = 0;
        mfActivate(true);
      }
    }
    if (!m_handle)
    {
      sprintf(m_ermes, "CResFile::Open - Can't open resource file <%s>", m_name);
      return false;
    }
  }
  m_typeaccess = type;

  if (type & RA_READ)
  {
    if ( iSystem->GetIPak()->FRead(&frh,1, sizeof(frh), m_handle) != sizeof(SFileResHeader))
    {
      sprintf (m_ermes, "CResFile::Open - Error read header in resource file <%s>", m_name);
      return false;
    }
    if (frh.hid != IDRESHEADER)
    {
      sprintf (m_ermes, "CResFile::Open - Error header-id in resource file <%s>", m_name);
      return false;
    }
    if (frh.ver != RESVERSION)
    {
      sprintf (m_ermes, "CResFile::Open - Error version number in resource file <%s>", m_name);
      return false;
    }
    m_version = frh.ver;
    if (!frh.num_files)
    {
      sprintf (m_ermes, "CResFile::Open - Empty resource file <%s>", m_name);
      return false;
    }

    int curseek = frh.ofs_first;
    int nums = 0;

    while (curseek >= 0)
    {
      if (nums >= frh.num_files)
      {
        sprintf (m_ermes, "CResFile::Open - Invalid count of directory files in resource file <%s>", m_name);
        return false;
      }
      iSystem->GetIPak()->FSeek(m_handle, curseek, SEEK_SET);
      SDirEntry *de = new SDirEntry;
      if (m_eDT == eFSD_name)
      {
        if (iSystem->GetIPak()->FRead(&fde0, 1, sizeof(fde0), m_handle) != sizeof(fde0))
        {
          sprintf (m_ermes, "CResFile::Open - Error read directory in resource file <%s>", m_name);
          return false;
        }
        de->ID = CName(fde0.name, eFN_Add).GetIndex();
        de->size = fde0.size;
        de->offset = fde0.offset;
        de->curseek = 0;
        de->eid = fde0.eid;
        de->earc = fde0.earc;
        de->flags = 0;
        de->user.data = NULL;
        if (CName(de->ID) == CName("$deleted$"))
        {
          de->flags |= RF_DELETED;
          m_holesSize += de->size;
        }
        nums++;
        curseek = fde0.offnext;
      }
      else
      {
        if (iSystem->GetIPak()->FRead(&fde1, 1, sizeof(fde1), m_handle) != sizeof(fde1))
        {
          sprintf (m_ermes, "CResFile::Open - Error read directory in resource file <%s>", m_name);
          return false;
        }
        de->ID = fde1.ID;
        de->size = fde1.size;
        de->offset = fde1.offset;
        de->curseek = 0;
        de->eid = fde1.eid;
        de->earc = fde1.earc;
        de->flags = 0;
        de->user.data = NULL;
        if (de->ID == -1)
        {
          de->flags |= RF_DELETED;
          m_holesSize += de->size;
        }
        nums++;
        curseek = fde1.offnext;
      }
      m_dir.insert(ResFilesMapItor::value_type(de->ID, de));
    }
    if (nums != frh.num_files)
    {
      sprintf (m_ermes, "CResFile::Open - Invalid count of directory files in resource file <%s>", m_name);
      return false;
    }
  }
  else
  {
    frh.hid = IDRESHEADER;
    frh.ver = RESVERSION;
    frh.num_files = 0;
    frh.ofs_first = -1;
    m_version = RESVERSION;
    if (iSystem->GetIPak()->FWrite(&frh, 1, sizeof(frh), m_handle) != sizeof(frh))
    {
      sprintf (m_ermes, "CResFile::Open - Error write header in resource file <%s>", m_name);
      return false;
    }
  }

  return true;
}

bool CResFile::mfClose(void)
{
  ResFilesMapItor itor = m_dir.begin();
  while(itor != m_dir.end())
  {
    SDirEntry* de = itor->second;
    SAFE_DELETE_ARRAY(de->user.data);
    delete de;
    itor++;
  }
  m_dir.clear();

  mfDeactivate();

  return true;
}

bool CResFile::mfGetDir(TArray<SDirEntry *>& Dir)
{
  ResFilesMapItor itor = m_dir.begin();
  while(itor != m_dir.end())
  {
    SDirEntry* de = itor->second;
    Dir.AddElem(de);
    itor++;
  }

  return true;
}

SDirEntry *CResFile::mfGetEntry(int num)
{
  ResFilesMapItor it = m_dir.find(num);
  if (it != m_dir.end())
    return it->second;
  return NULL;
}

int CResFile::mfFileGetNum(const char* name)
{
  CName nm = CName(name, eFN_Find);
  if (!nm.GetIndex())
  {
    sprintf (m_ermes, "CResFile::mfFileGetNum - Couldn't find file <%s> in resource file <%s>", name, m_name);
    return -1;
  }
  return nm.GetIndex();
}

int CResFile::mfFileRead(SDirEntry *de)
{
  int size;

  if (de->user.data)
    return de->size;

  mfActivate(false);

  if (de->earc != eARC_NONE)
  {
    iSystem->GetIPak()->FSeek(m_handle, de->offset, SEEK_SET);
    iSystem->GetIPak()->FRead(&size, 1, sizeof(int), m_handle);
    de->user.data = new byte [size];
    if (!de->user.data )
    {
      sprintf (m_ermes, "CResFile::mfFileRead - Couldn't allocate %i memory for file <%s> in resource file <%s>", size, CName(de->ID).c_str(), m_name);
      return 0;
    }
    byte* buf = new byte [de->size];
    if (!buf)
    {
      sprintf (m_ermes, "CResFile::mfFileRead - Couldn't allocate %i memory for file <%s> in resource file <%s>", de->size, CName(de->ID).c_str(), m_name);
      return 0;
    }
    iSystem->GetIPak()->FRead(buf, de->size-4, 1, m_handle);
    switch (de->earc)
    {
      case eARC_LZSS:
        Decodem(buf, (byte *)de->user.data, de->size-4);
        break;

    }
    delete [] buf;
    return size;
  }

  de->user.data = new byte [de->size];
  if (!de->user.data)
  {
    sprintf (m_ermes, "CResFile::mfFileRead - Couldn't allocate %i memory for file <%s> in resource file <%s>", de->size, CName(de->ID).c_str(), m_name);
    return 0;
  }
  iSystem->GetIPak()->FSeek(m_handle, de->offset, SEEK_SET);
  if (iSystem->GetIPak()->FRead(de->user.data, 1, de->size, m_handle) != de->size)
  {
    sprintf (m_ermes, "CResFile::mfFileRead - Error reading file <%s> in resource file <%s>", CName(de->ID).c_str(), m_name);
    return 0;
  }

  return de->size;
}

int CResFile::mfFileRead(int num)
{
  SDirEntry *de = mfGetEntry(num);

  if (!de)
  {
    sprintf (m_ermes, "CResFile::mfFileRead - invalid file id in resource file <%s>", m_name);
    return 0;
  }
  return mfFileRead(de);
}

int CResFile::mfFileRead(const char* name)
{
  return mfFileRead(mfFileGetNum(name));
}

int CResFile::mfFileRead(SDirEntry *de, void* data)
{
  int size;

  if (!data)
  {
    sprintf (m_ermes, "CResFile::mfFileRead - invalid data for file <%s> in resource file <%s>", CName(de->ID).c_str(), m_name);
    return 0;
  }

  mfActivate(false);

  if (de->earc != eARC_NONE)
  {
    iSystem->GetIPak()->FSeek(m_handle, de->offset, SEEK_SET);
    iSystem->GetIPak()->FRead(&size, 1, sizeof(int), m_handle);
    byte* buf = new byte [de->size];
    if ( !buf )
    {
      sprintf (m_ermes, "CResFile::mfFileRead - Couldn't allocate %i memory for file <%s> in resource file <%s>", de->size, CName(de->ID).c_str(), m_name);
      return 0;
    }
    iSystem->GetIPak()->FRead(buf, de->size-4, 1, m_handle);
    switch (de->earc)
    {
      case eARC_LZSS:
        Decodem(buf, (byte *)data, de->size-4);
        break;

    }
    delete [] buf;
    return size;
  }

  iSystem->GetIPak()->FSeek(m_handle, de->offset, SEEK_SET);
  if (iSystem->GetIPak()->FRead(data, 1, de->size, m_handle) != de->size)
  {
    sprintf (m_ermes, "CResFile::mfFileRead - Error reading file <%s> in resource file <%s>", CName(de->ID).c_str(), m_name);
    return 0;
  }

  return de->size;
}

int CResFile::mfFileRead(int num, void* data)
{
  SDirEntry *de = mfGetEntry(num);

  if (!de)
  {
    sprintf (m_ermes, "CResFile::mfFileRead - invalid file number in resource file <%s>", m_name);
    return 0;
  }
  return mfFileRead(de, data);
}

int CResFile::mfFileRead(const char* name, void* data)
{
  return mfFileRead(mfFileGetNum(name), data);
}

void* CResFile::mfFileRead2(SDirEntry *de, int size)
{
  void* buf;

  buf = new byte [size];
  if (!buf)
  {
    sprintf (m_ermes, "CResFile::mfFileRead2 - Couldn't allocate %i memory for file <%s> in resource file <%s>", size, de->Name(), m_name);
    return NULL;
  }
  if (de->user.data )
  {
    memcpy (buf, (byte *)(de->user.data)+de->curseek, size);
    de->curseek += size;
    return buf;
  }
  mfActivate(false);

  iSystem->GetIPak()->FSeek(m_handle, de->offset+de->curseek, SEEK_SET);
  if (iSystem->GetIPak()->FRead(buf, 1, size, m_handle) != size)
  {
    sprintf (m_ermes, "CResFile::mfFileRead2 - Error reading file <%s> in resource file <%s>", de->Name(), m_name);
    return NULL;
  }
  de->curseek += size;

  return buf;
}

void* CResFile::mfFileRead2(int num, int size)
{
  SDirEntry *de = mfGetEntry(num);
  if (!de)
  {
    sprintf (m_ermes, "CResFile::mfFileRead2 - invalid file id in resource file <%s>", m_name);
    return NULL;
  }
  return mfFileRead2(de, size);
}

void* CResFile::mfFileRead2(const char* name, int size)
{
  return mfFileRead2(mfFileGetNum(name), size);
}

void CResFile::mfFileRead2(SDirEntry *de, int size, void *buf)
{
  if (!buf)
  {
    sprintf (m_ermes, "CResFile::mfFileRead2 - Buffer is invalid for file %s in resource file <%s>", de->Name(), m_name);
    return;
  }
  if (de->user.data)
  {
    memcpy (buf, (byte *)(de->user.data)+de->curseek, size);
    de->curseek += size;
    return;
  }
  mfActivate(false);

  iSystem->GetIPak()->FSeek(m_handle, de->offset+de->curseek, SEEK_SET);
  if (iSystem->GetIPak()->FRead(buf, 1, size, m_handle) != size)
  {
    sprintf (m_ermes, "CResFile::mfFileRead2 - Error reading file <%s> in resource file <%s>", de->Name(), m_name);
    return;
  }
  de->curseek += size;

  return;
}

void CResFile::mfFileRead2(int num, int size, void *buf)
{
  SDirEntry *de = mfGetEntry(num);
  if (!de)
  {
    sprintf (m_ermes, "CResFile::mfFileRead2 - invalid file id in resource file <%s>", m_name);
    return;
  }
  return mfFileRead2(de, size, buf);
}

void* CResFile::mfFileGetBuf(SDirEntry *de)
{
  return de->user.data;
}

void* CResFile::mfFileGetBuf(int num)
{
  SDirEntry *de = mfGetEntry(num);
  if (!de)
  {
    sprintf (m_ermes, "CResFile::mfFileGetBuf - invalid file id in resource file <%s>", m_name);
    return NULL;
  }
  return mfFileGetBuf(de);
}

void* CResFile::mfFileGetBuf(char* name)
{
  return mfFileGetBuf(mfFileGetNum(name));
}

int CResFile::mfFileSeek(SDirEntry *de, int ofs, int type)
{
  int m;

  mfActivate(false);

  switch ( type )
  {
    case SEEK_CUR:
      de->curseek += ofs;
      m = iSystem->GetIPak()->FSeek(m_handle, de->offset+de->curseek, SEEK_SET);
      break;

    case SEEK_SET:
      m = iSystem->GetIPak()->FSeek(m_handle, de->offset+ofs, SEEK_SET);
      de->curseek = ofs;
      break;

    case SEEK_END:
      de->curseek = de->size-ofs;
      m = iSystem->GetIPak()->FSeek(m_handle, de->offset+de->curseek, SEEK_SET);
      break;

    default:
    sprintf (m_ermes, "CResFile::mfFileSeek - invalid seek type in resource file <%s>", m_name);
    return -1;
  }

  return m;
}

int CResFile::mfFileSeek(int num, int ofs, int type)
{
  SDirEntry *de = mfGetEntry(num);

  if (!de)
  {
    sprintf (m_ermes, "CResFile::mfFileSeek - invalid file id in resource file <%s>", m_name);
    return -1;
  }
  return mfFileSeek(de, ofs, type);
}

int CResFile::mfFileSeek(char* name, int ofs, int type)
{
  return mfFileSeek(mfFileGetNum(name), ofs, type);
}

int CResFile::mfFileLength(SDirEntry *de)
{
  return de->size;
}

int CResFile::mfFileLength(int num)
{
  SDirEntry *de = mfGetEntry(num);

  if (!de)
  {
    sprintf (m_ermes, "CResFile::mfFileLength - invalid file id in resource file <%s>", m_name);
    return -1;
  }
  return mfFileLength(de);
}

int CResFile::mfFileLength(char* name)
{
  return mfFileLength(mfFileGetNum(name));
}


int CResFile::mfFileAdd(SDirEntry* de)
{
  if (m_typeaccess == RA_READ)
  {
    sprintf (m_ermes, "CResFile::mfFileAdd - Operation 'Add' during RA_READ access mode in resource file <%s>", m_name);
    return -1;
  }
  if (de->size!=0 && de->user.data)
  {
    SDirEntry *newDE = new SDirEntry;
    *newDE = *de;
    newDE->flags |= RF_NOTSAVED;
    m_dir.insert(ResFilesMapItor::value_type(de->ID, newDE));
  }
  return m_dir.size();
}

bool CResFile::mfFlush(void)
{
  SFileResHeader frh;
  SFileDirEntry_0 fde0;
  SFileDirEntry_1 fde1;
  int m = 0;
  int size;

  if (m_typeaccess == RA_READ)
  {
    sprintf (m_ermes, "CResFile::mfFlush - Operation 'Flush' during RA_READ access mode in resource file <%s>", m_name);
    return false;
  }
  mfActivate(false);

  iSystem->GetIPak()->FSeek(m_handle, 0, SEEK_SET);
  iSystem->GetIPak()->FRead(&frh, 1, sizeof(frh), m_handle);

  int prevseek = -1;
  int curseek = frh.ofs_first;
  int nums = 0;

  while (curseek >= 0)
  {
    iSystem->GetIPak()->FSeek(m_handle, curseek, SEEK_SET);
    if (m_eDT == eFSD_name)
    {
      if (iSystem->GetIPak()->FRead(&fde0, 1, sizeof(fde0), m_handle) != sizeof(fde0))
      {
        sprintf (m_ermes, "CResFile::mfFlush - Error reading directory in resource file <%s>", m_name);
        return false;
      }
      prevseek = curseek;
      nums++;
      curseek = fde0.offnext;
    }
    else
    if (m_eDT == eFSD_id)
    {
      if (iSystem->GetIPak()->FRead(&fde1, 1, sizeof(fde1), m_handle) != sizeof(fde1))
      {
        sprintf (m_ermes, "CResFile::mfFlush - Error reading directory in resource file <%s>", m_name);
        return false;
      }
      prevseek = curseek;
      nums++;
      curseek = fde1.offnext;
    }
    m = 1;
  }
  if (nums == m_dir.size())
    return true;
  if (nums > (int)m_dir.size())
  {
    sprintf (m_ermes, "CResFile::mfFlush - Error number directory files in resource file <%s>", m_name);
    return false;
  }

  iSystem->GetIPak()->FSeek(m_handle, 0, SEEK_END);
  int nextseek = iSystem->GetIPak()->FTell(m_handle);
  if (!m)
    frh.ofs_first = nextseek;
  else
  {
    iSystem->GetIPak()->FSeek(m_handle, prevseek, SEEK_SET);
    if (m_eDT == eFSD_name)
    {
      fde0.offnext = nextseek;
      iSystem->GetIPak()->FWrite(&fde0, 1, sizeof(fde0), m_handle);
    }
    else
    if (m_eDT == eFSD_id)
    {
      fde1.offnext = nextseek;
      iSystem->GetIPak()->FWrite(&fde1, 1, sizeof(fde1), m_handle);
    }
  }

  ResFilesMapItor itor = m_dir.begin();
  while(itor != m_dir.end())
  {
    SDirEntry* de = itor->second;
    if (de->flags & RF_NOTSAVED)
    {
      if (m_eDT == eFSD_name)
      {
        de->flags &= ~RF_NOTSAVED;
        strcpy(fde0.name, CName(de->ID).c_str());
        fde0.eid = de->eid;
        fde0.earc = de->earc;
        fde0.size = de->size;
        if (fde0.earc != eARC_NONE)
        {
          byte* buf = new byte [fde0.size*2+128];
          if (!buf)
          {
            sprintf (m_ermes, "CResFile::mfFlush - Can't allocate %i memory for file <%s> in resource file <%s>", fde0.size, fde0.name, m_name);
            return false;
          }
          switch (fde0.earc)
          {
            case eARC_LZSS:
              size = Encodem((byte *)de->user.data, &buf[4], de->size);
              *(int *)buf = fde0.size;
              de->size = fde0.size = size+4;
              fde0.offset = nextseek+sizeof(SFileDirEntry_0);
              de->offset = fde0.offset;
              de->offsetHeader = nextseek;
              fde0.offnext = fde0.offset+fde0.size;
              fde0.offprev = prevseek;
              prevseek = nextseek;
              iSystem->GetIPak()->FSeek(m_handle, nextseek, SEEK_SET);
              nextseek = fde0.offnext;
              if (iSystem->GetIPak()->FWrite(&fde0, 1, sizeof(fde0), m_handle) != sizeof(fde0))
              {
                sprintf (m_ermes, "CResFile::mfFlush - Error write directory in resource file <%s>", m_name);
                return false;
              }
              if (iSystem->GetIPak()->FWrite(buf, 1, fde0.size, m_handle) != fde0.size)
              {
                sprintf (m_ermes, "CResFile::mfFlush - Error write file <%s> in resource file <%s>", CName(de->ID).c_str(), m_name);
                return false;
              }
              break;

            case eARC_RLE:
              break;
          }
          delete [] buf;
        }
        else
        {
          fde0.offset = nextseek+sizeof(SFileDirEntry_0);
          de->offset = fde0.offset;
          de->offsetHeader = nextseek;
          fde0.offnext = fde0.offset+fde0.size;
          fde0.offprev = prevseek;
          prevseek = nextseek;
          iSystem->GetIPak()->FSeek(m_handle, nextseek, SEEK_SET);
          nextseek = fde0.offnext;
          if (iSystem->GetIPak()->FWrite(&fde0, 1, sizeof(fde0), m_handle) != sizeof(fde0))
          {
            sprintf (m_ermes, "CResFile::mfFlush - Error write directory in resource file <%s>", m_name);
            return false;
          }
          if (iSystem->GetIPak()->FWrite(de->user.data, 1, de->size, m_handle) != de->size)
          {
            sprintf (m_ermes, "CResFile::mfFlush - Error write file <%s> in resource file <%s>", de->Name(), m_name);
            return false;
          }
        }
      }
      else
      if (m_eDT == eFSD_id)
      {
        de->flags &= ~RF_NOTSAVED;
        fde1.ID = de->ID;
        fde1.eid = de->eid;
        fde1.earc = de->earc;
        fde1.size = de->size;
        if (fde1.earc != eARC_NONE)
        {
          byte* buf = new byte [fde1.size*2+128];
          if (!buf)
          {
            sprintf (m_ermes, "CResFile::mfFlush - Can't allocate %i memory for file <%x> in resource file <%s>", fde1.size, fde1.ID, m_name);
            return false;
          }
          switch (fde1.earc)
          {
            case eARC_LZSS:
              size = Encodem((byte *)de->user.data, &buf[4], de->size);
              *(int *)buf = fde1.size;
              de->size = fde1.size = size+4;
              fde1.offset = nextseek+sizeof(SFileDirEntry_1);
              de->offset = fde1.offset;
              de->offsetHeader = nextseek;
              fde1.offnext = fde1.offset+fde1.size;
              fde1.offprev = prevseek;
              prevseek = nextseek;
              iSystem->GetIPak()->FSeek(m_handle, nextseek, SEEK_SET);
              nextseek = fde1.offnext;
              if (iSystem->GetIPak()->FWrite(&fde1, 1, sizeof(fde1), m_handle) != sizeof(fde1))
              {
                sprintf (m_ermes, "CResFile::mfFlush - Error write directory in resource file <%s>", m_name);
                return false;
              }
              if (iSystem->GetIPak()->FWrite(buf, 1, fde1.size, m_handle) != fde1.size)
              {
                sprintf (m_ermes, "CResFile::mfFlush - Error write file <%s> in resource file <%s>", de->Name(), m_name);
                return false;
              }
              break;

            case eARC_RLE:
              break;
          }
          delete [] buf;
        }
        else
        {
          fde1.offset = nextseek+sizeof(SFileDirEntry_1);
          de->offset = fde1.offset;
          de->offsetHeader = nextseek;
          fde1.offnext = fde1.offset+fde1.size;
          fde1.offprev = prevseek;
          prevseek = nextseek;
          iSystem->GetIPak()->FSeek(m_handle, nextseek, SEEK_SET);
          nextseek = fde1.offnext;
          if (iSystem->GetIPak()->FWrite(&fde1, 1, sizeof(fde1), m_handle) != sizeof(fde1))
          {
            sprintf (m_ermes, "CResFile::mfFlush - Error write directory in resource file <%s>", m_name);
            return false;
          }
          if (iSystem->GetIPak()->FWrite(de->user.data, 1, de->size, m_handle) != de->size)
          {
            sprintf (m_ermes, "CResFile::mfFlush - Error write file <%s> in resource file <%s>", de->Name(), m_name);
            return false;
          }
        }
      }

      if (de->flags & RF_TEMPDATA)
        SAFE_DELETE_ARRAY(de->user.data);
      de->user.data = NULL;
    }
    itor++;
  }
  if (prevseek != -1)
  {
    iSystem->GetIPak()->FSeek(m_handle, prevseek, SEEK_SET);
    if (m_eDT == eFSD_name)
    {
      fde0.offnext = -1;
      iSystem->GetIPak()->FWrite(&fde0, 1, sizeof(fde0), m_handle);
    }
    else
    if (m_eDT == eFSD_id)
    {
      fde1.offnext = -1;
      iSystem->GetIPak()->FWrite(&fde1, 1, sizeof(fde1), m_handle);
    }
  }
  frh.num_files = m_dir.size();
  iSystem->GetIPak()->FSeek(m_handle, 0, SEEK_SET);
  iSystem->GetIPak()->FWrite(&frh, 1, sizeof(frh), m_handle);
  iSystem->GetIPak()->FFlush(m_handle);

  return true;
}

bool CResFile::mfOptimize(uint type)
{
  return true;
}

int CResFile::mfFileDelete(SDirEntry *de)
{
  SFileDirEntry_0 fde0;
  SFileDirEntry_1 fde1;

  mfActivate(false);

  de->flags |= RF_DELETED;
  iSystem->GetIPak()->FSeek(m_handle, de->offsetHeader, SEEK_SET);
  if (m_eDT == eFSD_name)
  {
    iSystem->GetIPak()->FRead(&fde0, 1, sizeof(fde0), m_handle);
    strcpy(fde0.name, "$deleted$");
    iSystem->GetIPak()->FSeek(m_handle, de->offsetHeader, SEEK_SET);
    iSystem->GetIPak()->FWrite(&fde0, 1, sizeof(fde0), m_handle);
  }
  else
  if (m_eDT == eFSD_id)
  {
    iSystem->GetIPak()->FRead(&fde1, 1, sizeof(fde1), m_handle);
    fde1.ID = -1;
    iSystem->GetIPak()->FSeek(m_handle, de->offsetHeader, SEEK_SET);
    iSystem->GetIPak()->FWrite(&fde1, 1, sizeof(fde1), m_handle);
  }

  return m_dir.size();
}

int CResFile::mfFileDelete(int num)
{
  SDirEntry *de = mfGetEntry(num);
  if (!de)
  {
    sprintf (m_ermes, "CResFile::mfFileDelete - error file number in resource file <%s>", m_name);
    return -1;
  }
  return mfFileDelete(de);
}

int CResFile::mfFileDelete(char* name)
{
  return mfFileDelete(mfFileGetNum(name));
}

//======================================================================

void StripExtension (const char *in, char *out)
{
  ptrdiff_t len = strlen(in)-1;

  if(len<=1)
  {
    strcpy(out, in);
    return;
  }

  while (in[len])
  {
    if (in[len]=='.')
    {
      int n = len;
      while(in[n] != 0)
      {
        if (in[n] == '+')
        {
          strcpy(out, in);
          return;
        }
        n++;
      }
      break;
    }
    len--;
    if (!len)
    {
      strcpy(out, in);
      return;
    }
  }
  strncpy(out, in, len);
  out[len] = 0;
}

const char *GetExtension (const char *in)
{
  ptrdiff_t len = strlen(in)-1;
  while (len)
  {
    if (in[len]=='.')
      return &in[len];
    len--;
  }
  return NULL;
}

void AddExtension (char *path, char *extension)
{
  char    *src;
  src = path + strlen(path) - 1;

  while (*src != '/' && src != path)
  {
    if (*src == '.')
      return;                 // it has an extension
    src--;
  }

  strcat (path, extension);
}

void ConvertDOSToUnixName( char *dst, const char *src )
{
  while ( *src )
  {
    if ( *src == '\\' )
      *dst = '/';
    else
      *dst = *src;
    dst++; src++;
  }
  *dst = 0;
}

void ConvertUnixToDosName( char *dst, const char *src )
{
  while ( *src )
  {
    if ( *src == '/' )
      *dst = '\\';
    else
      *dst = *src;
    dst++; src++;
  }
  *dst = 0;
}

void UsePath (char *name, char *path, char *dst)
{
  char c;

  if (!path)
  {
    strcpy(dst, name);
    return;
  }

  strcpy(dst, path);
  if ( (c=path[strlen(path)-1]) != '/' && c!='\\')
    strcat(dst, "/");
  strcat(dst, name);
}
