////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   errorreport.cpp
//  Version:     v1.00
//  Created:     30/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ErrorReport.h"
#include "ErrorReportDialog.h"

#include "Objects\BaseObject.h"
#include "Material\Material.h"

//////////////////////////////////////////////////////////////////////////
CString CErrorRecord::GetErrorText()
{
	CString str = error;
	str.TrimRight();

	if (!file.IsEmpty())
	{
		str += CString(",File=\"") + file + "\"";
	}
	else
	{
	}
	if (pMaterial)
	{
		str += CString(",Material=<") + pMaterial->GetFullName() + ">";
	}
	if (pObject)
	{
		str += CString(",Object=\"") + pObject->GetName() + "\"";
	}
	return str;
}

//////////////////////////////////////////////////////////////////////////
// CError Report.
//////////////////////////////////////////////////////////////////////////
CErrorReport::CErrorReport()
{
	m_errors.reserve( 100 );
	m_bImmidiateMode = true;
	m_pObject = 0;
	m_pMaterial = 0;
	m_pParticle = 0;
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::ReportError( CErrorRecord &err )
{
	if (m_bImmidiateMode)
	{
		if (err.module == VALIDATOR_MODULE_EDITOR)
			Warning( err.error );
		else
		{
			if (err.severity == VALIDATOR_ERROR)
				CLogFile::FormatLine( "ERROR: %s",err.error );
			else
				CLogFile::FormatLine( "WARNING: %s",err.error );

			// Show dialog if first character of warning is !.
			if (!err.error.IsEmpty() && err.error[0] == '!')
			{
				Warning( err.error );
			}
		}
	}
	else
	{
		if (err.pObject == NULL && m_pObject)
		{
			err.pObject = m_pObject;
		}
		else if (err.pMaterial == NULL && m_pMaterial)
		{
			err.pMaterial = m_pMaterial;
		}
		else if (err.pParticle == NULL && m_pParticle)
		{
			err.pParticle = m_pParticle;
		}
		m_errors.push_back( err );
	}
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::Clear()
{
	m_errors.clear();
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::Display()
{
	if (m_errors.empty())
	{
		SetImmidiateMode( true );
		return;
	}

	// Log all errors.
	CLogFile::WriteLine( "========================= Errors =========================" );
	for (int i = 0; i < m_errors.size(); i++)
	{
		CErrorRecord &err = m_errors[i];
		CString str = err.GetErrorText();
		CLogFile::FormatLine( "%d) %s",i,(const char*)str );
	}
	CLogFile::WriteLine( "========================= End Errors =========================" );

	CErrorReportDialog::Open( this );
	SetImmidiateMode( true );
}

//////////////////////////////////////////////////////////////////////////
bool CErrorReport::IsEmpty() const
{
	return m_errors.empty();
}

//////////////////////////////////////////////////////////////////////////
CErrorRecord& CErrorReport::GetError( int i )
{
	assert( i >= 0 && i < m_errors.size() );
	return m_errors[i];
};

//////////////////////////////////////////////////////////////////////////
void CErrorReport::SetImmidiateMode( bool bEnable )
{
	if (bEnable != m_bImmidiateMode)
	{
		Clear();
		m_bImmidiateMode = bEnable;
	}
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::Report( SValidatorRecord &record )
{
	CErrorRecord err;
	if (record.text)
		err.error = record.text;
	if (record.description)
		err.description = record.description;
	if (record.file)
		err.file = record.file;
	else
		err.file = m_currentFilename;
	err.severity = (CErrorRecord::ESeverity)record.severity;

	err.flags = 0;
	if (record.flags & VALIDATOR_FLAG_FILE)
		err.flags |= CErrorRecord::FLAG_NOFILE;
	if (record.flags & VALIDATOR_FLAG_TEXTURE)
		err.flags |= CErrorRecord::FLAG_TEXTURE;
	if (record.flags & VALIDATOR_FLAG_SCRIPT)
		err.flags |= CErrorRecord::FLAG_SCRIPT;
	if (record.flags & VALIDATOR_FLAG_AI)
		err.flags |= CErrorRecord::FLAG_AI;

	err.module = record.module;
	err.pObject = m_pObject;
	err.pMaterial = m_pMaterial;
	err.pParticle = m_pParticle;

	ReportError( err );
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::SetCurrentValidatorObject( CBaseObject *pObject )
{
	m_pObject = pObject;
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::SetCurrentValidatorMaterial( CMaterial *pMtl )
{
	m_pMaterial = pMtl;
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::SetCurrentValidatorParticle( CParticleItem *pParticle )
{
	m_pParticle = pParticle;
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::SetCurrentFile( const CString &file )
{
	m_currentFilename = file;
}

//////////////////////////////////////////////////////////////////////////
CErrorsRecorder::CErrorsRecorder()
{
	GetIEditor()->GetErrorReport()->SetImmidiateMode(false);
}

//////////////////////////////////////////////////////////////////////////
CErrorsRecorder::~CErrorsRecorder()
{
	GetIEditor()->GetErrorReport()->Display();
}
