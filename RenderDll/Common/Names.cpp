/*=============================================================================
	Names.cpp : implementation of the CName class using hash tables.
	Copyright 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/


#include "RenderPCH.h"
#include "Names.int"


TArray<SNameEntry *> CName::mNames;
TArray<int> CName::mAvailable;
SNameEntry* CName::mNameHash[8192];
int CName::mDuplicate;

int CName::Size()
{
  INT_PTR nSize = 0;									//AMD Port
  nSize += mAvailable.GetMemoryUsage();
  for (int i=0; i<mNames.GetSize(); i++)
  {
    if (i < mNames.Num())
    {
      nSize += 4;
      if (mNames[i])
        nSize += mNames[i]->Size();
    }
    else
      nSize += 4;
  }
  nSize += sizeof(long) * 256;

  return nSize;
}

//===================================================================

SNameEntry* CreateNewNameEntry(const char *str, int num, int m, SNameEntry *sFNE)
{
  SNameEntry *lsFNE;

  lsFNE = (SNameEntry *)malloc( strlen(str)+sizeof(SNameEntry)-MAX_SNAME_LEN+1);
  lsFNE->mNumName = num;
  lsFNE->mFlags = m;
  lsFNE->mNext = sFNE;

  strcpy(lsFNE->mName, str);

  return lsFNE;
}

void DestroyNameEntry(SNameEntry* sFNE)
{
  free(sFNE);
}


CName::CName(const char* str, EFindName efn) 
{
  char tstr[MAX_SNAME_LEN];
  SNameEntry * sFNE;

  assert( str );

  if ( !mNames.Num() )
    mfInitTables();

  if ( *str == 0 )
    mNameIndex = 0;

  strncpy(tstr, str, MAX_SNAME_LEN);

  int i = 0;
  long hash = mfGetHash(tstr);
  sFNE = mNameHash[hash];
  while ( sFNE )
  {
    if ( !stricmp(tstr,sFNE->mName) )
    {
      mNameIndex = sFNE->mNumName;
      return;
    }
    sFNE = sFNE->mNext;
  }
  if ( efn == eFN_Find )
    mNameIndex = 0;
  else
  {
    if ( mAvailable.Num() )
    {
      mNameIndex = mAvailable[mAvailable.Num()-1];
      mAvailable._Remove(mAvailable.Num()-1, 1);
    }
    else
    {
      i = mNames.Num();
      mNames.AddIndex(1);
      mNameIndex = i;
    }
    sFNE = CreateNewNameEntry(str, mNameIndex, 0, mNameHash[hash] );
    mNameHash[hash] = sFNE;
    mNames[mNameIndex] = sFNE;
    if ( efn == eFN_Intrinsic )
      mNames[mNameIndex]->mFlags |= NF_Intrinsic;
  }
}

void CName::mfInitTables(void)
{
  int i;

  for ( i=0; i<8192; i++ )
  {
    mNameHash[i] = NULL;
  }

  i = 0;
  while(gIntrinsicNames[i].mName[0])
  {
    mfRegister(gIntrinsicNames[i]);  
    i++;
  }

}

void CName::mfRegister(SNameEntry& sFNE)
{
  long hash;
  int i, l;

  hash = mfGetHash(sFNE.mName);
  sFNE.mNext = mNameHash[hash];
  mNameHash[hash] = &sFNE;
  l = mNames.Num();
  while ( l <= sFNE.mNumName )
  {
    i = mNames.Num();
    mNames.AddIndex(1);
    mNames[i] = NULL;
    l++;
  }
  if ( mNames[sFNE.mNumName] != NULL )
    mDuplicate = sFNE.mNumName;

  mNames[sFNE.mNumName] = &sFNE;
}

void CName::mfDisplayHash(void)
{
  int i, l, m;
  SNameEntry* sFNE;

  l = m = 0;
  for ( i=0; i<8192; i++ )
  {
    sFNE = mNameHash[i];
    if ( sFNE )
      m++;
    while ( sFNE )
    {
      l++;
      sFNE = sFNE->mNext;
    }
  }
  iLog->Log( "Hash: %i names, %i/%i hash bins", l, m, 8192);
}

void CName::mfInitSubsystem(void)
{
  SNameEntry* sFNE;
  SNameEntry* sFNE1;
  int i;

  CName("None");

  assert( mNames.Num() );

  if ( mDuplicate )
  {
    Warning( 0,0,"WARNING: name %i was duplicated", mDuplicate);
    return;
  }

  for ( i=0; i<8192; i++ )
  {
    sFNE = mNameHash[i];
    while ( sFNE )
    {
      sFNE1 = sFNE->mNext;
      if ( sFNE1 )
      {
        if ( !stricmp(sFNE->mName, sFNE1->mName) )
          Warning( 0,0,"WARNING: Name '%s' was duplicated", sFNE->mName);
      }
      sFNE = sFNE->mNext;
    }
  }
}

void CName::mfExitSubsystem(void)
{
  SNameEntry* sFNE;
  int i;

  for ( i=0; i<mNames.Num(); i++ )
  {
    sFNE = mNames[i];
    if ( sFNE && !(sFNE->mFlags & NF_Intrinsic) )
      DestroyNameEntry(sFNE);
  }
  mNames.Free();
}

void CName::mfDeleteEntry(int num)
{
  SNameEntry* sFNE;
  SNameEntry** sFNE1;
  int i;
  long hash;

  sFNE = mNames[num];

  assert(sFNE);
  assert(!(sFNE->mFlags & NF_Intrinsic));

  hash = mfGetHash(sFNE->mName);
  sFNE1 = &mNameHash[hash];

  while ( *sFNE1 && *sFNE1!=sFNE  )
  {
    sFNE1 = &((*sFNE1)->mNext);
  }
  if ( !*sFNE1 )
  {
    Warning( 0,0,"WARNING: Unhashed name '%s'\n", sFNE->mName);
    return;
  }

  *sFNE1 = (*sFNE1)->mNext;
  mNames[num] = NULL;
  i = mAvailable.Num();
  mAvailable.AddIndex(1);
  mAvailable[i] = num;
  DestroyNameEntry(sFNE);
}

