/////////////////////////////////////////////////////////////////////////////
// FileWrapper.cpp - Class Implementation
//
// Encapsulation class for reading and writing files.
//
// Copyright (c) 2008 - 2015 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "FileWrapper.h"
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "utils.h"

#if defined _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

/////////////////////////////////////////////////////////////////////////////
// Constructors / Destructors
/////////////////////////////////////////////////////////////////////////////
FileWrapper::FileWrapper(
	LPCTSTR FileName)
{
	m_FileName	= GetStringCopy(FileName);
}

FileWrapper::FileWrapper(
	LPCSTR FileName)
{
	m_FileName	= GetUnicodeString(FileName);
}

FileWrapper::~FileWrapper(void)
{
	if (NULL != m_FileName)
	{
		delete m_FileName;
		m_FileName = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Append
/////////////////////////////////////////////////////////////////////////////
bool
	FileWrapper::Append(
	BYTE*	Contents,
	DWORD	ContentsLength)
{
	bool	ReturnCode	= false;

	if ((NULL != m_FileName) && (NULL != Contents))
	{
		HANDLE	FileHandle	= CreateFile(m_FileName,
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_ALWAYS,
			0,
			0);

		if (INVALID_HANDLE_VALUE != FileHandle)
		{
			DWORD	BytesRead	= 0;

			SetFilePointer(FileHandle, 0, NULL, FILE_END);

			BOOL	ResultCode	= WriteFile(FileHandle, Contents, ContentsLength, &BytesRead, NULL);

			CloseHandle(FileHandle);

			if (FALSE != ResultCode)
			{
				ReturnCode	= true;
			}
		}
	}

	return ReturnCode;
}

/////////////////////////////////////////////////////////////////////////////
// Create
/////////////////////////////////////////////////////////////////////////////
bool
	FileWrapper::Create(
	BYTE*	Contents,
	DWORD	ContentsLength)
{
	bool	ReturnCode	= false;

	if ((NULL != m_FileName) && (NULL != Contents))
	{
		HANDLE	FileHandle	= CreateFile(m_FileName,
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			CREATE_ALWAYS,
			0,
			0);

		if (INVALID_HANDLE_VALUE != FileHandle)
		{
			DWORD	BytesRead	= 0;

			BOOL	ResultCode	= WriteFile(FileHandle, Contents, ContentsLength, &BytesRead, NULL);

			CloseHandle(FileHandle);

			if (FALSE != ResultCode)
			{
				ReturnCode	= true;
			}
		}
	}

	return ReturnCode;
}

/////////////////////////////////////////////////////////////////////////////
// Read
/////////////////////////////////////////////////////////////////////////////
BYTE*
	FileWrapper::Read(
	DWORD*	ContentsLength)
{
	BYTE*	InputContents = NULL;

	if (NULL != m_FileName)
	{
		HANDLE	FileHandle	= CreateFile(m_FileName,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);

		if (INVALID_HANDLE_VALUE != FileHandle)
		{
			DWORD	ContentsSize	= GetFileSize(FileHandle, NULL) + 2;

			if (0 < ContentsSize )
			{
				InputContents = new BYTE[ContentsSize];

				if (NULL != InputContents)
				{
					DWORD	BytesRead	= 0;

					SecureZeroMemory(InputContents, ContentsSize);

					BOOL	ResultCode	= ReadFile(FileHandle, InputContents, ContentsSize, &BytesRead, NULL) ;

					CloseHandle(FileHandle);

					if (FALSE != ResultCode)
					{
						*ContentsLength	= ContentsSize;
					}
				}
			}
		}
	}

	return InputContents;
}

/////////////////////////////////////////////////////////////////////////////
// ReadText
/////////////////////////////////////////////////////////////////////////////
char*
	FileWrapper::ReadText(void)
{
	char*	InputContents = NULL;

	if (NULL != m_FileName)
	{
		HANDLE	FileHandle	= CreateFile(m_FileName,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);

		if (INVALID_HANDLE_VALUE != FileHandle)
		{
			DWORD	ContentsSize	= GetFileSize(FileHandle, NULL) + 1;

			if (0 < ContentsSize )
			{
				InputContents = new char[ContentsSize];

				if (NULL != InputContents)
				{
					DWORD	BytesRead	= 0;

					SecureZeroMemory(InputContents, ContentsSize);

					BOOL result = ReadFile(FileHandle, InputContents, ContentsSize, &BytesRead, NULL) ;

					if (result == FALSE)
					{
					}

					CloseHandle(FileHandle);
				}
			}
		}
	}

	return InputContents;
}

/////////////////////////////////////////////////////////////////////////////
// ReadUnicodeText
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	FileWrapper::ReadUnicodeText(void)
{
	TCHAR*	InputContents = NULL;

	if (NULL != m_FileName)
	{
		HANDLE	FileHandle	= CreateFile(m_FileName,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);

		if (INVALID_HANDLE_VALUE != FileHandle)
		{
			DWORD	ContentsSize	= GetFileSize(FileHandle, NULL) / sizeof(TCHAR) + 2;

			if (0 < ContentsSize )
			{
				InputContents = new TCHAR[ContentsSize];

				if (NULL != InputContents)
				{
					DWORD	BytesRead	= 0;

					SecureZeroMemory(InputContents, ContentsSize);

					BOOL result = ReadFile(FileHandle, InputContents, ContentsSize, &BytesRead, NULL) ;

					if (result == FALSE)
					{
					}

					CloseHandle(FileHandle);
				}
			}
		}
	}

	return InputContents;
}