#include "lua.h"
#include "lauxlib.h"

int g_vectortag=0;

float *newvector(lua_State *L)
{
	float *v=(float *)lua_newuserdata(L, sizeof(float)*3);
	int nparams=lua_gettop(L);
	lua_settag(L,g_vectortag);
	if(nparams>0)
	{
		v[0]=lua_tonumber(L,1);
		v[1]=lua_tonumber(L,2);
		v[2]=lua_tonumber(L,3);
	}
	else{
		v[0]=v[1]=v[2]=0.0f;
	}
	return v;
}

int vector_set(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		const char* idx=luaL_check_string(L,2);
		if(idx)
		{
			switch(idx[0])
			{
			case 'x':case 'r':
				v[0]=luaL_check_number(L,3);
				return 0;
			case 'y':case 'g':
				v[1]=luaL_check_number(L,3);
				return 0;
			case 'z':case 'b':
				v[2]=luaL_check_number(L,3);
				return 0;
			default:
				break;
			}
		}
	}
	return 0;
}

int vector_get(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		const char* idx=luaL_check_string(L,2);
		if(idx)
		{
			switch(idx[0])
			{
			case 'x':case 'r':
				lua_pushnumber(L,v[0]);
				return 1;
			case 'y':case 'g':
				lua_pushnumber(L,v[1]);
				return 1;
			case 'z':case 'b':
				lua_pushnumber(L,v[2]);
				return 1;
			default:
				return 0;
				break;
			}
		}
	}
	return 0;
}

int vector_mul(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		if(vl_isvector(L,2))
		{
			float *v2=(float *)lua_touserdata(L,2);
			float res=v[0]*v2[0] + v[1]*v2[1] + v[2]*v2[2];
			lua_pushnumber(L,res);
			return 1;
		}
		else if(lua_isnumber(L,2))
		{
			float f=lua_tonumber(L,2);
			float *newv=newvector(L);
			newv[0]=v[0]*f;
			newv[1]=v[1]*f;
			newv[2]=v[2]*f;
			return 1;

		}else lua_error(L,"mutiplying a vector with an invalid type");
	}
	return 0;
}

int vector_add(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		if(vl_isvector(L,2))
		{
			float *v2=(float *)lua_touserdata(L,2);
			float *newv=newvector(L);
			newv[0]=v[0]+v2[0];
			newv[1]=v[1]+v2[1];
			newv[2]=v[2]+v2[2];
			return 1;
		}
		else lua_error(L,"adding a vector with an invalid type");
	}
	return 0;
}

int vector_div(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		if(lua_isnumber(L,2))
		{
			float f=lua_tonumber(L,2);
			float *newv=newvector(L);
			newv[0]=v[0]/f;
			newv[1]=v[1]/f;
			newv[2]=v[2]/f;
			return 1;

		}
		else lua_error(L,"dividing a vector with an invalid type");
	}
	return 0;
}

int vector_sub(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		if(vl_isvector(L,2))
		{
			float *v2=(float *)lua_touserdata(L,2);
			float *newv=newvector(L);
			newv[0]=v[0]-v2[0];
			newv[1]=v[1]-v2[1];
			newv[2]=v[2]-v2[2];
			return 1;
		}
		else if(lua_isnumber(L,2))
		{
			float f=lua_tonumber(L,2);
			float *newv=newvector(L);
			newv[0]=v[0]-f;
			newv[1]=v[1]-f;
			newv[2]=v[2]-f;
			return 1;

		}
		else lua_error(L,"subtracting a vector with an invalid type");
	}
	return 0;
}

int vector_unm(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		float *newv=newvector(L);
		newv[0]=-v[0];
		newv[1]=-v[1];
		newv[2]=-v[2];
		return 1;
	}
	return 0;
}

int vector_pow(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		if(vl_isvector(L,2))
		{
			float *v2=(float *)lua_touserdata(L,2);
			float *newv=newvector(L);
			newv[0]=v[1]*v2[2]-v[2]*v2[1];
			newv[1]=v[2]*v2[0]-v[0]*v2[2];
			newv[2]=v[0]*v2[1]-v[1]*v2[0];
			return 1;
		}
		else lua_error(L,"cross product between vector and an invalid type");
	}
	return 0;
}

int vl_newvector(lua_State *L)
{
	newvector(L);
	return 1;
}

int vl_isvector(lua_State *L,int index)
{
	return lua_tag(L,index)==g_vectortag;
}

int vl_initvectorlib(lua_State *L)
{
	g_vectortag=lua_newtype(L,"vector",LUA_TUSERDATA);

	lua_pushcclosure(L,vector_set, 0);
	lua_settagmethod(L, g_vectortag, "settable");
	lua_pushcclosure(L,vector_get, 0);
	lua_settagmethod(L, g_vectortag, "gettable");
	lua_pushcclosure(L,vector_mul, 0);
	lua_settagmethod(L, g_vectortag, "mul");
	lua_pushcclosure(L,vector_div, 0);
	lua_settagmethod(L, g_vectortag, "div");
	lua_pushcclosure(L,vector_add, 0);
	lua_settagmethod(L, g_vectortag, "add");
	lua_pushcclosure(L,vector_sub, 0);
	lua_settagmethod(L, g_vectortag, "sub");
	lua_pushcclosure(L,vector_unm, 0);
	lua_settagmethod(L, g_vectortag, "unm");
	lua_pushcclosure(L,vector_pow, 0);
	lua_settagmethod(L, g_vectortag, "pow");
	lua_pushcclosure(L,vl_newvector, 0);
	lua_setglobal(L,"vector");
	return 0;
}

