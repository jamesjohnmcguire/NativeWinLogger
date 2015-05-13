/////////////////////////////////////////////////////////////////////////////
// Resource.cpp
//
// Represents a resource module.
//
// Copyright (c) 2010 - 2015 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "Utils.h"

Resource::Resource(
	LPCTSTR ModulePath)
{
	m_ModulePath = ModulePath;
}

Resource::~Resource(void)
{
}

/////////////////////////////////////////////////////////////////////////////
// GetResourceModule
/////////////////////////////////////////////////////////////////////////////
HMODULE
Resource::GetResourceModule()
{
	if (NULL == m_ResourceModule)
	{
		TCHAR*	ModuleName		= NULL;

		if (NULL != m_ModulePath)
		{
			m_ResourceModule = LoadLibrary( m_ModulePath );

			if (NULL != m_ResourceModule)
			{
				AfxSetResourceHandle(m_ResourceModule);

				delete ModuleName;
			}
		}
	}

	return m_ResourceModule;
}

/////////////////////////////////////////////////////////////////////////////
// GetString
//
// Gets an localized string from the resource table
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
TCHAR*
Resource::GetString(
	UINT	ResourceId)
{
	CString ResourceCstring;
	TCHAR*	ResourceString = NULL;

	if (NULL == m_ResourceModule)
	{
		m_ResourceModule = GetResourceModule();
	}

	if (NULL != m_ResourceModule)
	{
		//BOOL ResultCode = ResourceCstring.LoadString(Module, ResourceId, m_LanguageId);
		BOOL ResultCode = ResourceCstring.LoadString(m_ResourceModule, ResourceId);

		if (TRUE == ResultCode)
		{
			ResourceString	= GetStringCopy(ResourceCstring);
		}
	}

	return ResourceString;
}

/////////////////////////////////////////////////////////////////////////////
// ShowMessage
/////////////////////////////////////////////////////////////////////////////
int
Resource::ShowMessage(
	int TitleId,
	int	StringId)
{
	TCHAR*	Title	= GetString(TitleId);
	TCHAR*	Message	= GetString(StringId);
	int ReturnCode	= MessageBox(GetActiveWindow(), Message, Title, MB_OK);
	delete Title;
	delete Message;

	return ReturnCode;
}

/////////////////////////////////////////////////////////////////////////////
// ShowMessageError
/////////////////////////////////////////////////////////////////////////////
int
Resource::ShowMessageError(
	int TitleId,
	LPCTSTR	Message,
	int		StringId)
{
	TCHAR*	Title			= GetString(TitleId);
	TCHAR*	ErrorMessage	= GetString(StringId);
	CString CompleteMessage = Message;
	int ReturnCode = MessageBox(GetActiveWindow(),
		CompleteMessage + _T(": ") + ErrorMessage, Title, MB_OK);
	delete Title;
	delete ErrorMessage;

	return ReturnCode;
}

/////////////////////////////////////////////////////////////////////////////
// ShowMessage
/////////////////////////////////////////////////////////////////////////////
int
Resource::ShowMessageYesNo(
	int TitleId,
	int	StringId)
{
	TCHAR*	Title	= GetString(TitleId);
	TCHAR*	Message	= GetString(StringId);
	int ReturnCode	= MessageBox(GetActiveWindow(), Message, Title, MB_YESNO);
	delete Title;
	delete Message;

	return ReturnCode;
}
