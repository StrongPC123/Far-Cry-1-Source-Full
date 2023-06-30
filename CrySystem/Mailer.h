////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Mailer.h
//  Version:     v1.00
//  Created:     5/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Send mail.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Mailer_h__
#define __Mailer_h__

#if _MSC_VER > 1000
#pragma once
#endif

class CMailer
{
public:
	static bool SendMessage(const char* _subject,             // E-Mail Subject
						              const char* _messageBody,         // Message Text
													const std::vector<const char*> &_recipients,  // All Recipients' Addresses
						              const std::vector<const char*> &_attachments, // All File Attachments
						              bool bShowDialog);   // Whether to allow editing by user
};

#endif // __Mailer_h__
