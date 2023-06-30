#ifndef _SERVERSNOOPER_H_
#define _SERVERSNOOPER_H_

#include <INetwork.h>
class CServerSnooper :
	public IServerSnooper
{
public:
	CServerSnooper(void);
	virtual ~CServerSnooper(void);
	bool Init(IServerSnooperSink *pSink);
public:
//IServerSnooper
	void SearchForLANServers(unsigned int nTime);
	void Update(unsigned int nTime);
	void Release(){delete this;}
protected:
	void ProcessPacket(CStream &stmPacket,CIPAddress &ip);
private:
	CDatagramSocket m_socket;
	unsigned int m_nStartTime;
	unsigned int m_nCurrentTime;
	IServerSnooperSink *m_pSink;
};

#endif //_SERVERSNOOPER_H_