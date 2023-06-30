#pragma once

#include <vector>
#include <imoviesystem.h>

///////////////////////////////////////////////////////////////////////////////
//
// Iterator.
// Iterator class allows iteration thru elements of container.
//
///////////////////////////////////////////////////////////////////////////////
class CSequenceIt : public ISequenceIt
{
public:
	CSequenceIt();
	virtual ~CSequenceIt();
	void Release();
	void	add( IAnimSequence* element );
	void	clear();
	bool	empty() const;
	int		count() const;
	IAnimSequence*	first();
	IAnimSequence*	next();
private:
	typedef std::vector<IAnimSequence*>	Elements;
	int	m_count;
	Elements::iterator	m_current;
	Elements	m_elements;
};