// 
// Reference linked smart pointer template
//
// Created by Petar
// cleaned up and added operator* by Petar 27 Feb 2003
// added derived class that releases the underlying pointer
#pragma once


template <class PtrClass>
class pointer_container
{


protected:
	mutable PtrClass *m_pContent;

	mutable const pointer_container *m_pNext;
	mutable const pointer_container *m_pPrevious;

	inline void init(PtrClass *initial_content)
	{
		m_pContent = initial_content;
		m_pNext=this;
		m_pPrevious=this;
	}

	inline void insert(const pointer_container &other) const
	{
		m_pNext = &other;
		m_pPrevious = other.m_pPrevious;
		other.m_pPrevious->m_pNext = this;
		other.m_pPrevious = this;
	}

	inline void remove() const
	{
		m_pNext->m_pPrevious = m_pPrevious;
		m_pPrevious->m_pNext = m_pNext;
	}

	inline void cleanup() const
	{
		remove();
		if (m_pNext == this)
		{
			delete m_pContent;
			m_pContent = 0;	
		}
	}

public:

	pointer_container(PtrClass *initial_content=0)
	{
		init(initial_content);
	}

	// copy constructor
	pointer_container(const pointer_container &copy)
	{
		init(0);
		operator=(copy);
	}

	~pointer_container()
	{
		cleanup();
	}

	inline const pointer_container &operator=(const pointer_container &copy)
	{
		cleanup();
		m_pContent = copy.m_pContent;
		if (copy.m_pContent)
			insert(copy);
		return *this;
	}

	inline PtrClass &operator*() { return *m_pContent;}
	inline PtrClass *operator->() { return m_pContent;}
	inline bool		operator==(const pointer_container &other)
	{
		return m_pContent == other.m_pContent;
	}
	inline bool operator!=(const pointer_container &other)
	{
		return !operator==(other);
	}
	inline operator PtrClass* () const
	{	
		return m_pContent;
	}
	inline bool operator!()
	{
		return !m_pContent;
	}

};

