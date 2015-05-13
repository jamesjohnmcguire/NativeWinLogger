/////////////////////////////////////////////////////////////////////////////
// Registry.h
//
// Represents the  Windows registry.
//
// Copyright (c) 2008 - 2015 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#define USE_CRTDBG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <shlwapi.h>
#include "Registry.h"
#include "Utils.h"

/////////////////////////////////////////////////////////////////////////////
// Constructors / Destructors
/////////////////////////////////////////////////////////////////////////////
Registry::Registry(void)
{
}

Registry::~Registry(void)
{
}

/////////////////////////////////////////////////////////////////////////////
// Delete
/////////////////////////////////////////////////////////////////////////////
LONG
	Registry::Delete(
	HKEY	hKeyRoot,
	LPCTSTR	pszRegKeyPath,
	LPCTSTR	pszSubKey,
	bool	bOnlyIfEmpty)
{
	LONG	lRet	= -1;
	HKEY	hKey;

	lRet = RegOpenKeyEx(	hKeyRoot,
		pszRegKeyPath,
		0,
		KEY_READ,
		&hKey);

	if (ERROR_SUCCESS == lRet)
	{
		if (bOnlyIfEmpty)
			lRet	= RegDeleteKey(	hKey,
			pszSubKey );
		else
			lRet	= SHDeleteKey(	hKey,
			pszSubKey );

		RegCloseKey( hKey );
	}

	return lRet;
}

/////////////////////////////////////////////////////////////////////////////
// GetDwordValue
/////////////////////////////////////////////////////////////////////////////
DWORD Registry::GetDwordValue(HKEY KeyRoot, LPCTSTR KeyPath,
	LPCTSTR KeyVariable)
{
	LONG	successCode	= -1;
	HKEY	hKey;
	DWORD	RetunValue	= NULL;
	DWORD	BufferSize	= sizeof(DWORD);
	successCode = RegOpenKeyEx(KeyRoot, KeyPath, 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == successCode)
	{
		successCode = RegQueryValueEx(hKey, KeyVariable, 0, NULL,
			(LPBYTE)&RetunValue, &BufferSize);

		RegCloseKey( hKey );
	}

	return successCode;
}

/////////////////////////////////////////////////////////////////////////////
// GetSubKeyNames
/////////////////////////////////////////////////////////////////////////////
TCHAR** Registry::GetSubKeyNames(HKEY KeyRoot, TCHAR* RegKey, DWORD* KeyCount)
{
	LONG	lRet = -1;
	HKEY	hKey;
	DWORD	KeyValuesCount = 0;
	TCHAR**	KeyNames = NULL;

	lRet = RegOpenKeyEx(KeyRoot, RegKey, 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lRet)
	{
		lRet = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
			&KeyValuesCount, NULL, NULL, NULL, NULL);

		if ((ERROR_SUCCESS == lRet) && (KeyValuesCount > 0))
		{
			KeyNames = new TCHAR*[KeyValuesCount];

			if (NULL != KeyNames)
			{
				TCHAR	KeyName[MAX_KEY_LENGTH];
				DWORD	KeyNameLength = MAX_KEY_LENGTH;

				for (DWORD Index=0; Index < KeyValuesCount; Index++)
				{
					KeyName[0] = '\0';
					KeyNameLength = MAX_KEY_LENGTH;
					lRet = RegEnumValue(hKey,
						Index,
						KeyName,
						&KeyNameLength,
						NULL,
						NULL,
						NULL,
						NULL);

					if (ERROR_SUCCESS == lRet)
					{
						KeyNames[Index] = GetStringCopy(KeyName);
					}
				}
			}

			*KeyCount = KeyValuesCount;
		}
		RegCloseKey( hKey );
	}

	return KeyNames;
}

/////////////////////////////////////////////////////////////////////////////
// GetSubKeysCount
/////////////////////////////////////////////////////////////////////////////
DWORD Registry::GetSubKeysCount(HKEY KeyRoot, LPCTSTR RegKey)
{
	LONG	lRet	= -1;
	HKEY	hKey;
	DWORD	KeyValuesCount	= 0;

	lRet = RegOpenKeyEx(KeyRoot, RegKey, 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lRet)
	{
		lRet	= RegQueryInfoKey(	hKey, NULL, NULL, NULL, NULL, NULL, NULL,
			&KeyValuesCount, NULL, NULL, NULL, NULL);

		if (ERROR_SUCCESS != lRet)
		{
			KeyValuesCount = 0;
		}

		RegCloseKey( hKey );
	}

	return KeyValuesCount;
}

/////////////////////////////////////////////////////////////////////////////
// GetValue
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	Registry::GetValue(
	HKEY KeyRoot,
	LPCTSTR RegKeyPath,
	LPCTSTR RegKeyVar)
{
	LONG	ReturnCode	= -1;
	HKEY	hKey;
	TCHAR*	StringValue	= NULL;
	DWORD	BufferSize	= 0;
	ReturnCode = RegOpenKeyEx(	KeyRoot,
		RegKeyPath,
		0,
		KEY_READ,
		&hKey);

	if (ERROR_SUCCESS == ReturnCode)
	{
		ReturnCode	= RegQueryValueEx(	hKey,
			RegKeyVar,
			0,
			NULL,
			NULL,
			&BufferSize);

		if (ERROR_SUCCESS == ReturnCode)
		{
			StringValue = new TCHAR[BufferSize+1];

			ReturnCode	= RegQueryValueEx(	hKey,
				RegKeyVar,
				0,
				NULL,
				(LPBYTE)StringValue,
				&BufferSize);
		}

		RegCloseKey( hKey );
	}

	return StringValue;
}

/////////////////////////////////////////////////////////////////////////////
// GetValue
/////////////////////////////////////////////////////////////////////////////
LONG
	Registry::GetValue(
	HKEY	hKeyRoot,
	LPCTSTR	pszRegKeyPath,
	LPCTSTR	pszRegKeyVar,
	LPTSTR	pszValue,
	DWORD	dwValueBufSize)
{
	LONG	lRet	= -1;
	HKEY	hKey;

	if (pszValue)
	{
		lRet = RegOpenKeyEx(	hKeyRoot,
			pszRegKeyPath,
			0,
			KEY_READ,
			&hKey);

		if (ERROR_SUCCESS == lRet)
		{
			lRet	= RegQueryValueEx(	hKey,
				pszRegKeyVar,
				0,
				NULL,
				(LPBYTE)pszValue,
				&dwValueBufSize );

			RegCloseKey( hKey );
		}
	}

	return lRet;
}

/////////////////////////////////////////////////////////////////////////////
// SetValue
/////////////////////////////////////////////////////////////////////////////
LONG
	Registry::SetValue(
	HKEY	hKeyRoot,
	LPCTSTR	pszRegKeyPath,
	LPCTSTR	pszRegKeyVar,
	LPCTSTR	pszValue)
{
	LONG	lRet	= -1;
	HKEY	hKey;
	DWORD	dwKeyOut;

	if (pszValue)
	{
		lRet = RegCreateKeyEx(	hKeyRoot,
			pszRegKeyPath,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&hKey,
			&dwKeyOut);

		/*
		lRet = RegOpenKeyEx(	hKeyRoot,
		pszRegKeyPath,
		0,
		KEY_READ | KEY_SET_VALUE,
		&hKey);
		*/
		if (ERROR_SUCCESS == lRet)
		{
			//ShowDebug( _T("SetRegKeyValue-RegCreateKeyEx = ERROR_SUCCESS") );
			lRet	= RegSetValueEx(	hKey,
				pszRegKeyVar,
				0,
				REG_SZ,
				(LPBYTE)pszValue,
				(DWORD)(_tcslen(pszValue) + 1) * sizeof(TCHAR));

			//if (ERROR_SUCCESS == lRet)
			//	ShowDebug( _T("SetRegKeyValue-RegSetValueEx = ERROR_SUCCESS") );
			//else
			//	ShowDebug( _T("SetRegKeyValue-RegSetValueEx != ERROR_SUCCESS") );
			RegCloseKey( hKey );
		}
		//else
		//	ShowDebug( _T("SetRegKeyValue-RegCreateKeyEx != ERROR_SUCCESS") );
	}

	return lRet;
}

/////////////////////////////////////////////////////////////////////////////
// SetValue
/////////////////////////////////////////////////////////////////////////////
LONG
	Registry::SetValue(
	HKEY KeyRoot,
	LPCTSTR RegKeyPath,
	LPCTSTR RegKeyVar,
	DWORD Value)
{
	LONG	lRet	= -1;
	HKEY	hKey;
	DWORD	dwKeyOut;

	lRet = RegCreateKeyEx(	KeyRoot,
		RegKeyPath,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		&dwKeyOut);

	if (ERROR_SUCCESS == lRet)
	{
		lRet	= RegSetValueEx(hKey,
			RegKeyVar,
			0,
			REG_DWORD,
			(LPBYTE)&Value,
			(DWORD)sizeof(DWORD));

		RegCloseKey( hKey );
	}

	return lRet;
}