/////////////////////////////////////////////////////////////////////////////
// Resource.h
//
// Represents a resource module.
//
// Copyright (c) 2010 - 2015 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "../Include/Common.h"

class DllExport Resource
{
	public:
		Resource(
			LPCTSTR ModulePath);
		~Resource(void);

		HMODULE GetResourceModule();
		TCHAR* GetString(
			UINT	ResourceId);
		int ShowMessageString(
			LPCTSTR Message,
			UINT uType = MB_OK);
		int ShowMessage(
			int TitleId,
			int	StringId);
		int ShowMessageError(
			int TitleId,
			LPCTSTR	Message,
			int		StringId);
		int ShowMessageYesNo(
			int TitleId,
			int	StringId);

	private:
		LPCTSTR m_ModulePath;
		HMODULE m_ResourceModule;
};
