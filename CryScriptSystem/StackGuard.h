#ifndef _STACKGUARD_H_
#define _STACKGUARD_H_
struct _StackGuard
{
	_StackGuard(lua_State *p)
	{
		m_pLS=p;
		m_nTop=lua_gettop(m_pLS);
	}
	~_StackGuard()
	{
		lua_settop(m_pLS,m_nTop);
	}
private:
	int m_nTop;
	lua_State *m_pLS;
};

#define _GUARD_STACK(ls)  _StackGuard __guard__(ls);

#endif _STACKGUARD_H_