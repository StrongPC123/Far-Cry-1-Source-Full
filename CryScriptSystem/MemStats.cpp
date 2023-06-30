#include "stdafx.h"
#include "ScriptSystem.h"
#include <crysizer.h>
extern "C" {
#define LUA_PRIVATE
#include "lua.h"

#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
}

#define calcarraysize(n,t) ((lu_mem)(n)*(lu_mem)sizeof(t))
#define calcclosuresize(nupvals)	((int)sizeof(Closure) + (int)sizeof(TObject)*((nupvals)-1))

extern "C" int gLuaAllocatedMemory;

int calcprotosize(Proto *f)
{
	int i=0;
	i+=calcarraysize(f->sizecode, Instruction);
	i+=calcarraysize(f->sizelocvars, struct LocVar);
	i+=calcarraysize(f->sizek, TObject);
	i+=calcarraysize(f->sizep, Proto *);
	i+=calcarraysize(f->sizelineinfo, int);
	i+=sizeof(Proto);
	return i;
}

int calctablesize(Hash *t) {
	int i=0;
	i+=calcarraysize(t->size, Node);
	i+=sizeof(Hash);
	return i;
}

void CScriptSystem::GetMemoryStatistics(ICrySizer *pSizer)
{
	//pSizer->AddObject( this,gLuaAllocatedMemory+sizeof(*this) );
	//return;

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////


	lua_StateStats lss;
	lua_StateStats *LSS=&lss;
	Proto *proto=m_pLS->G->rootproto;
	Closure *closure=m_pLS->G->rootcl;
	Hash *hash=m_pLS->G->roottable;
	Udata *udata=m_pLS->G->rootudata;
	TString *string=m_pLS->G->strt.hash[0];

	LSS->nProto=0;
	LSS->nProtoMem=0;
	LSS->nClosure=0;
	LSS->nClosureMem=0;
	LSS->nHash=0;
	LSS->nHashMem=0;
	LSS->nString=0;
	LSS->nStringMem=0;
	LSS->nUdata=0;
	LSS->nUdataMem=0;
#ifdef TRACE_TO_FILE
	FILE *f=fopen("protodump.txt","w+");
	if(!f)::OutputDebugString("opening 'protodump.txt' failed\n");
#endif
/////BYTECODE////////////////////////////////////////////
	{
		SIZER_SUBCOMPONENT_NAME(pSizer,"Bytecode");
		while(proto!=NULL)
		{
			LSS->nProto++;
			LSS->nProtoMem+=calcprotosize(proto);
#ifdef TRACE_TO_FILE
			if(f)if(!proto->lineDefined)
				fprintf(f,"%d,%s,noline\n",calcprotosize(proto),getstr(proto->source));
			else
				fprintf(f,"%d,%s,%d\n",calcprotosize(proto),getstr(proto->source),-proto->lineinfo[0]);
#endif
			proto=proto->next;
		}
		pSizer->AddObject(m_pLS->G->rootproto,LSS->nProtoMem);
	}
#ifdef TRACE_TO_FILE
	if(f)fclose(f);
#endif
	/////FUNCTIONS/////////////////////////////////////////
	{
		SIZER_SUBCOMPONENT_NAME(pSizer,"Functions");
		while(closure!=NULL)
		{
			LSS->nClosure++;
			LSS->nClosureMem+=calcclosuresize(closure->nupvalues);
			closure=closure->next;
		}
		pSizer->AddObject(m_pLS->G->rootcl,LSS->nClosureMem);
	}
	/////TABLES/////////////////////////////////////////
	{
		int maxsize=0;
		int size=0;
		while(hash!=NULL)
		{
			LSS->nHash++;
			size=calctablesize(hash);
			if(size>maxsize)maxsize=size;
			LSS->nHashMem+=size;
			hash=hash->next;
		}
		char ctemp[200]="Unknown";
		SIZER_SUBCOMPONENT_NAME(pSizer,ctemp);
		pSizer->AddObject(m_pLS->G->roottable,LSS->nHashMem);
	}
	/////USERDATA///////////////////////////////////////
	{
		SIZER_SUBCOMPONENT_NAME(pSizer,"User Data");
		while(udata!=NULL)
		{
			LSS->nUdata++;
			LSS->nUdataMem+=sizeudata(udata->uv.len);
			udata=udata->uv.next;
		}
		pSizer->AddObject(m_pLS->G->rootudata,LSS->nUdataMem);
	}
	/////STRINGS///////////////////////////////////////
	{
		SIZER_SUBCOMPONENT_NAME(pSizer,"Strings");
		for (int i=0; i<m_pLS->G->strt.size; i++) {  /* for each list */
			TString **p = &m_pLS->G->strt.hash[i];
			TString *curr;
			while ((curr = *p) != NULL)
			{
					LSS->nString++;
					if (string) // this can be NULL in Previewer
						LSS->nStringMem += sizestring(string->tsv.len);
					p = &curr->tsv.nexthash;
			}
		}
		pSizer->AddObject(m_pLS->G->strt.hash,LSS->nStringMem);
	}
	/////REGISTRY///////////////////////////////////////
	{
		SIZER_SUBCOMPONENT_NAME(pSizer,"Reference Registry");
		pSizer->AddObject(m_pLS->G->registry,calctablesize(m_pLS->G->registry));
		pSizer->AddObject(m_pLS->G->weakregistry,calctablesize(m_pLS->G->weakregistry));
		pSizer->AddObject(m_pLS->G->xregistry,calctablesize(m_pLS->G->xregistry));
	}
	{
		SIZER_SUBCOMPONENT_NAME(pSizer,"Global Table");
		pSizer->AddObject(m_pLS->gt,calctablesize(m_pLS->gt));
		
	}
	/*char sTemp[1000];
	::OutputDebugString("-----LUA STATS------\n");
	sprintf(sTemp,"Proto num=%d memsize=%d kb\n",LSS->nProto,LSS->nProtoMem/1024);
	::OutputDebugString(sTemp);
	sprintf(sTemp,"Closure num=%d memsize=%d kb\n",LSS->nClosure,LSS->nClosureMem/1024);
	::OutputDebugString(sTemp);
	sprintf(sTemp,"Hash num=%d memsize=%d kb\n",LSS->nHash,LSS->nHashMem/1024);
	::OutputDebugString(sTemp);
	sprintf(sTemp,"Udata num=%d memsize=%d kb\n",LSS->nUdata,LSS->nUdataMem/1024);
	::OutputDebugString(sTemp);
	sprintf(sTemp,"String num=%d memsize=%d kb\n",LSS->nString,LSS->nStringMem/1024);
	::OutputDebugString(sTemp);
	sprintf(sTemp,"registry table memsize=%d kb\n",calctablesize(m_pLS->G->registry)/1024);
	::OutputDebugString(sTemp);
	sprintf(sTemp,"weak registry table memsize=%d kb\n",calctablesize(m_pLS->G->weakregistry)/1024);
	::OutputDebugString(sTemp);
	::OutputDebugString("-----END LUA STATS------\n");*/
}