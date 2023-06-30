#include "stdafx.h"
#include "sequenceit.h"

CSequenceIt::CSequenceIt()
{
	m_count = 0;
	m_current = m_elements.end();
}

CSequenceIt::~CSequenceIt()
{
}

void CSequenceIt::Release()
{
	delete this;
}
	
void CSequenceIt::add( IAnimSequence* element )
{
	m_elements.push_back( element );
	m_count++;
	m_current = m_elements.begin();
}

void CSequenceIt::clear()
{
	m_elements.clear();
	m_count = 0;
	m_current = m_elements.end();
}

bool CSequenceIt::empty() const
{
	return m_count == 0;
};

int CSequenceIt::count() const
{
	return m_count;
};
	
IAnimSequence* CSequenceIt::first()
{
	m_current = m_elements.begin();
	if (m_current != m_elements.end()) {
		return *m_current;
	}
	return 0;
}

IAnimSequence* CSequenceIt::next()
{
	if (m_current != m_elements.end()) {
		if (++m_current != m_elements.end()) {
			return *m_current;
		}
	}
	return 0;
}