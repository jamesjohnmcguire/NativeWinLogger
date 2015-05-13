/////////////////////////////////////////////////////////////////////////////
// Registry.h
//
// Represents the  Windows registry.
//
// Copyright (c) 2008 - 2015 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "Common.h"

#define MAX_KEY_LENGTH 255

/////////////////////////////////////////////////////////////////////////////
// Registry Class Definition
/////////////////////////////////////////////////////////////////////////////
class DllExport Registry
{
	public:
		// Properties

		// Methods
			Registry(void);
			~Registry(void);

			LONG Delete(
				HKEY	hKeyRoot,
				LPCTSTR	pszRegKeyPath,
				LPCTSTR	pszSubKey,
				bool	bOnlyIfEmpty);
			DWORD GetDwordValue(
				HKEY	KeyRoot,
				LPCTSTR	KeyPath,
				LPCTSTR	KeyVariable);
			TCHAR** GetSubKeyNames(
				HKEY KeyRoot,
				TCHAR* RegKey,
				DWORD*	KeyCount);
			DWORD GetSubKeysCount(
				HKEY KeyRoot,
				LPCTSTR RegKey);
			TCHAR* GetValue(
				HKEY	KeyRoot,
				LPCTSTR	RegKeyPath,
				LPCTSTR	RegKeyVar);
			LONG GetValue(
				HKEY	hKeyRoot,
				LPCTSTR	pszRegKeyPath,
				LPCTSTR	pszRegKeyVar,
				LPTSTR	pszValue,
				DWORD	dwValueBufSize);
			LONG SetValue(
				HKEY	hKeyRoot,
				LPCTSTR	pszRegKeyPath,
				LPCTSTR	pszRegKeyVar,
				LPCTSTR	pszValue);
			LONG SetValue(
				HKEY	KeyRoot,
				LPCTSTR	RegKeyPath,
				LPCTSTR	RegKeyVar,
				DWORD	Value);
};
