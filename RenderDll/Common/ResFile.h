#ifndef __RESFILE_H__
#define __RESFILE_H__

#define IDRESHEADER		(('K'<<24)+('C'<<16)+('P'<<8)+'C')
#define RESVERSION  1

enum EResId
{
  eRI_UNKNOWN   = -1,
  eRI_BIN       = 0,
  eRI_IMAGE     = 1,
  eRI_MIPTEX    = 2,
  eRI_SOUND     = 3,
  eRI_MODEL     = 4,
  eRI_PALETTE   = 5
};

enum EArc
{
  eARC_NONE     = 0,
  eARC_RLE      = 1,
  eARC_LZSS     = 2
};



// Resource header
struct SFileResHeader
{
  uint hid;
  int  ver;
  uint optimize_mode;
  int num_files;
#ifdef PS2
  int ofs_first;
#else  
  long ofs_first;
#endif  
};

enum EFS_DirType
{
  eFSD_name,
  eFSD_id,
};

// Each file entry in resource
struct SFileDirEntry_0
{
  char name[256-6*4];
  int size;
  int offset;
  int offnext;
  int offprev;
  EResId eid;      // eRI_
  EArc earc;       // eARC_
};

// Each file entry in resource
struct SFileDirEntry_1
{
  int ID;
  int size;
  int offset;
  int offnext;
  int offprev;
  EResId eid;      // eRI_
  EArc earc;       // eARC_
};

struct SCacheUser
{
  void *data;
};

// Intrinsic file entry
struct SDirEntry
{
  uint ID;
  int size;
#ifdef PS2  
  long offset;
  long offsetHeader;
  long curseek;
#else
  int offset;
  int offsetHeader;
  int curseek;
#endif
  EResId eid;      // eRI_
  EArc earc;       // eARC_
  SCacheUser user;
  uint flags;

  SDirEntry()
  {
    memset(this, 0, sizeof(SDirEntry));
  }
  const char *Name()
  {
    return CName(ID).c_str();
  }
};

typedef std::map<uint,SDirEntry*> ResFilesMap;
typedef ResFilesMap::iterator ResFilesMapItor;

// Resource access types
#define RA_READ   1
#define RA_WRITE  2
#define RA_CREATE 4


// Resource files flags
#define RF_NOTSAVED 1
#define RF_DELETED 2
#define RF_INTRINSIC 4
#define RF_CACHED 8
#define RF_BIG 0x10
#define RF_TEMPDATA 0x20

// Resource optimize flags
#define RO_HEADERS_IN_BEGIN 1
#define RO_HEADERS_IN_END 2
#define RO_HEADER_FILE 4
#define RO_SORT_ALPHA_ASC 8
#define RO_SORT_ALPHA_DESC 0x10

#define MAX_OPEN_RESFILES 40

class CResFile
{
private:
  char m_name[1024];
  char *m_szAccess;
  FILE* m_handle;
  EFS_DirType m_eDT;
  ResFilesMap m_dir;
  int m_typeaccess;
  uint m_optimize;
  char m_ermes[1024];
  int m_version;
  int m_holesSize;

  static CResFile m_Root;
  static int m_nNumOpenResources;
  CResFile *m_Next;
  CResFile *m_Prev;

  bool mfActivate(bool bFirstTime);
  void mfDeactivate();

  _inline void Relink(CResFile* Before)
  {
    if (m_Next && m_Prev)
    {
      m_Next->m_Prev = m_Prev;
      m_Prev->m_Next = m_Next;
    }
    m_Next = Before->m_Next;
    Before->m_Next->m_Prev = this;
    Before->m_Next = this;
    m_Prev = Before;
  }
  _inline void Unlink()
  {
    if (!m_Next || !m_Prev)
      return;
    m_Next->m_Prev = m_Prev;
    m_Prev->m_Next = m_Next;
    m_Next = m_Prev = NULL;
  }
  _inline void Link(CResFile* Before)
  {
    if (m_Next || m_Prev)
      return;
    m_Next = Before->m_Next;
    Before->m_Next->m_Prev = this;
    Before->m_Next = this;
    m_Prev = Before;
  }

public:
  CResFile(const char* name, EFS_DirType eDT);
  CResFile();
  ~CResFile();

  char* mfGetError(void);
  void mfSetError(const char *er);
  const char *mfGetFileName() {return m_name;}
  bool mfGetDir(TArray<SDirEntry *>& Dir);

  bool mfOpen(int type);
  bool mfClose();
  bool mfFlush();
  bool mfOptimize(uint type);

  int mfGetNumFiles() { return m_dir.size(); }

  int mfFileGetNum(const char* name);
  
  int mfFileRead(SDirEntry *de);
  int mfFileRead(const char* name);
  int mfFileRead(int num);

  int mfFileRead(SDirEntry *de, void* data);
  int mfFileRead(int num, void* data);
  int mfFileRead(const char* name, void* data);
  
  void* mfFileRead2(SDirEntry *de, int size);
  void* mfFileRead2(int num, int size);
  void* mfFileRead2(const char* name, int size);

  void  mfFileRead2(SDirEntry *de, int size, void *buf);
  void  mfFileRead2(int num, int size, void *buf);
  
  void* mfFileGetBuf(SDirEntry *de);
  void* mfFileGetBuf(int num);
  void* mfFileGetBuf(char* name);

  int mfFileSeek(SDirEntry *de, int offs, int type);
  int mfFileSeek(int num, int offs, int type);
  int mfFileSeek(char* name, int offs, int type);

  int mfFileLength(SDirEntry *de);
  int mfFileLength(int num);
  int mfFileLength(char* name);

  int mfFileAdd(SDirEntry* de);

  int mfFileDelete(SDirEntry *de);
  int mfFileDelete(int num);
  int mfFileDelete(char* name);

  int mfFileExist(int ID);
  int mfFileExist(const char* name);

  SDirEntry *mfGetEntry(int num);

  FILE *mfGetHandle() { return m_handle; }
  int mfGetResourceSize();
  int mfGetHolesSize();
};

#endif //  __RESFILE_H__
