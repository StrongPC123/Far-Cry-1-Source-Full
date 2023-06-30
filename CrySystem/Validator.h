
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code (c) Crytek 2001-2004
// 
//	File: Validator.h	
// 
//	History:
//	-Feb 09,2004:Created 
//
//////////////////////////////////////////////////////////////////////

//#ifndef VALIDATOR_H
//#define VALIDATOR_H

#if _MSC_VER > 1000
# pragma once
#endif

//////////////////////////////////////////////////////////////////////////
// Default validator implementation.
//////////////////////////////////////////////////////////////////////////
struct SDefaultValidator : public IValidator
{
	CSystem *m_pSystem;
	SDefaultValidator( CSystem *system ) : m_pSystem(system) {};
	virtual void Report( SValidatorRecord &record )
	{
		if (record.text)
		{
			if (record.severity == VALIDATOR_ERROR)
				m_pSystem->GetILog()->Log( "\001ERROR: %s",record.text );
			else if (record.severity == VALIDATOR_WARNING)
        m_pSystem->GetILog()->Log( "\002WARNING: %s",record.text );
			else
				m_pSystem->GetILog()->Log( "%s",record.text );

			if ( (record.text[0] == '!')  || (m_pSystem->m_sysWarnings && m_pSystem->m_sysWarnings->GetIVal() != 0))
			{
#ifdef WIN32
				string strMessage = record.text;
				strMessage += "\n---------------------------------------------\nAbort - terminate application\nRetry - continue running the application\nIgnore - don't show this message box any more";
				switch (::MessageBox( NULL, strMessage.c_str(), "CryEngine Warning", MB_ABORTRETRYIGNORE|MB_DEFBUTTON2|MB_ICONWARNING|MB_SYSTEMMODAL ))
				{
				case IDABORT:
					m_pSystem->GetIConsole()->Exit ("User abort requested during showing the warning box with the following message: %s", record.text );
					break;
				case IDRETRY:
					break;
				case IDIGNORE:
					m_pSystem->m_sysWarnings->Set(0);
					break;
				}
#endif
			}
		}
	}
};

//#endif