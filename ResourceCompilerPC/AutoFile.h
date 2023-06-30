
class CAutoFile
{
	FILE* m_f;
public:
	CAutoFile (const char* szName, const char* szMode)
	{
		m_f = fopen (szName, szMode);
	}
	~CAutoFile ()
	{
		if (m_f)
			fclose (m_f);
	}

	long GetSize()
	{
		if (fseek (m_f, 0, SEEK_END))
			return -1;

		long nSize = ftell (m_f);

		if (fseek(m_f, 0, SEEK_SET))
			return -1;

		return nSize;
	}

	bool Read (void* pData, unsigned nSize)
	{
		return (1 == fread (pData, nSize, 1, m_f));
	}

	bool isEof()
	{
		return feof(m_f)?true:false;
	}

	operator FILE*() {return m_f;}
};
