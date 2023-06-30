#ifndef __CNAME_H__
#define __CNAME_H__

#include <TArray.h>

#define MAX_SNAME_LEN 256

// Name index.
typedef INT NAME_INDEX;

#define checkName ASSERT

enum EFindName
{
  eFN_Find    = 0,
  eFN_Add,
  eFN_Intrinsic
};


extern long gHashTable[];

struct SNameEntry
{
  int mNumName;
  unsigned int mFlags;
  SNameEntry* mNext;
  char mName[MAX_SNAME_LEN];

  size_t Size()
  {
    size_t nSize = sizeof(*this) - MAX_SNAME_LEN;
    nSize += strlen(mName) + 1;

    return nSize;
  }
};

#define NF_Intrinsic 0x1

class CName
{
private:
  int mNameIndex;

public:
  // constructors
  CName() {mNameIndex=0;}
  CName(int nIndex) {mNameIndex=nIndex;}
  CName(char const *, enum EFindName=eFN_Add);
  ~CName() {}

  CName& operator=(const CName& cFN)   { mNameIndex = cFN.mNameIndex; return *this; }
  const char* operator*(void) const    { return mNames[mNameIndex]->mName; }
  int operator!=(const CName& cFN) const  { return (cFN.mNameIndex != mNameIndex); }
  int operator==(const CName& cFN) const  { return (cFN.mNameIndex == mNameIndex); }

  const char* c_str() const { return mNames[mNameIndex]->mName; }
  unsigned long mfGetFlags(void) const   { return mNames[mNameIndex]->mFlags; }
  long mfGetHash(const char *str)
  {
    unsigned char ch;
    unsigned int hash = 0;
    int temp;

    while ( ch=*str++ )
    {
      ch = toupper(ch);
      temp = hash & 0xff;
      hash >>= 8;
      hash ^= gHashTable[ch ^ temp];
    }
    return (hash & 0x1fff);
  }
  NAME_INDEX GetIndex() const
  {
    return mNameIndex;
  }
  void mfClear() { mNameIndex=0; }
  void mfClearFlags(unsigned int flag)  { mNames[mNameIndex]->mFlags &= ~flag; }
  void mfSetFlags(unsigned int flag)    { mNames[mNameIndex]->mFlags |= flag; }
  int mfGetIndex(void) const             { return mNameIndex; }
  int mfIsValid(void)
  {
    if ( mNameIndex<0 || mNameIndex>=mNames.Num() || !mNames[mNameIndex] )
      return 0;
    return 1;
  }

  void mfInitTables(void);
  static void mfDisplayHash(void);
  static void mfExitSubsystem(void);
  static void mfInitSubsystem(void);
  static SNameEntry* mfGetEntry(int i) { return mNames[i]; }
  void mfRegister(SNameEntry&);
  static int mfGetMaxNames(void) { return mNames.Num(); }
  static int Size();
  void mfDeleteEntry(int);

  static TArray<SNameEntry *> mNames;
  static TArray<int> mAvailable;
  static SNameEntry* mNameHash[8192];
  static int mDuplicate;
};

//===========================================================


#endif // __CNAME_H__
