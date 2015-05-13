/////////////////////////////////////////////////////////////////////////////
// Utils.cpp
//
// String and other common functions.
//
// Copyright (c) 2007 - 2015 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#ifndef INSTALLHELPER_EXPORTS
#define _CRTDBG_MAP_ALLOC
#endif
#include <string>
#include <crtdbg.h>
#include <stdlib.h>
#include <shlobj.h>
#include "Utils.h"
#include "Registry.h"

#if defined _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

#ifdef INSTALLHELPER_EXPORTS
#include <tchar.h>
#else
/////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////

#ifdef BUILD_JAPAN
static WORD g_LanguageId = LANG_JAPANESE;
#else
static WORD g_LanguageId = LANG_ENGLISH;
#endif

#define DATE_FORMAT _T("%02d/%02d %d %02d:%02d")

static HMODULE g_ResourceModule	= NULL;

CWinApp theApp;

static ULONG alternativeCodePages[] = {CodePageUtf8, CodePageUsAscii, 0};

BOOL CWinApp::InitInstance()
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// ConcatStreams
// Caller is responsible for 'delete'ing the return stream.
/////////////////////////////////////////////////////////////////////////////
BYTE* ConcatStreams(BYTE* firstStream, DWORD firstStreamLength,
	BYTE* SecondStream, DWORD SecondStreamLength)
{
	BYTE* ConcatStream	= NULL;

	// if both null, nothing to do
	if ((NULL != firstStream) || (NULL != SecondStream))
	{
		try
		{
			// even though we shouldn't assume these are strings, we want to insure
			// the buffer is zero-terminated.
			// there are some situations, where a memory blob is cast as a string.
			DWORD	ConcatStreamLength =
				firstStreamLength + SecondStreamLength + 2;

			ConcatStream = new BYTE[ConcatStreamLength];

			SecureZeroMemory(ConcatStream, ConcatStreamLength);
			errno_t ErrorCode = memcpy_s(ConcatStream, ConcatStreamLength,
				firstStream, firstStreamLength);
			if ((ErrorCode == 0) || (ErrorCode == EINVAL))
			{
				BYTE* SecondPart = ConcatStream + firstStreamLength;

				ErrorCode = memcpy_s(SecondPart,
					ConcatStreamLength - firstStreamLength,
					SecondStream,
					SecondStreamLength);

				if ((ErrorCode != 0) && (ErrorCode != EINVAL))
				{
					SetLastError(ErrorCode);
					throw;
				}
			}
			else
			{
				SetLastError(ErrorCode);
				throw;
			}
		}
		catch(TCHAR* Exception)
		{
			// log
			OutputDebugString(Exception);
			throw;
		}
	}

	return ConcatStream;
}

/////////////////////////////////////////////////////////////////////////////
// ConcatStringsA
//
// Ascii version
//
// Caller is responsible for 'delete'ing the string.
/////////////////////////////////////////////////////////////////////////////
char*
	ConcatStringsA(
	const char*	FirstString,
	const char*	SecondString)
{
	char*	ConcatString		= NULL;

	if ((NULL != FirstString) || (NULL != SecondString))
	{
		try
		{
			if (NULL == FirstString)
			{
				ConcatString = GetStringCopyA(SecondString);
			}
			else if (NULL == SecondString)
			{
				ConcatString = GetStringCopyA(FirstString);
			}
			else
			{
				UINT	ConcatStringLength = (UINT)strlen(FirstString) +
					(UINT)strlen(SecondString) +
					1;

				ConcatString = new char[ConcatStringLength];

				strcpy_s(ConcatString, ConcatStringLength, FirstString);
				strcat_s(ConcatString, ConcatStringLength, SecondString);
			}
		}
		catch(TCHAR* Exception)
		{
			// log
			OutputDebugString(Exception);
			throw;
		}
	}

	return ConcatString;
}

/////////////////////////////////////////////////////////////////////////////
// ConcatStringsVA
//
// Ascii version
//
// Caller is responsible for 'delete'ing the string.
/////////////////////////////////////////////////////////////////////////////
char* __cdecl
	ConcatStringsVA(
	const char*	FirstString,
	const char*	SecondString,
	...)
{
	char*	NewString	= NULL;
	va_list	Arguments;

	va_start( Arguments, SecondString );

	const char*	OldString	= NewString = ConcatStringsA(FirstString, SecondString);

	const char*	NextArg	= va_arg( Arguments, const char*);

	while (NULL != NextArg)
	{
		NewString	= ConcatStringsA(OldString, NextArg);

		delete OldString;
		OldString	= NewString;

		NextArg	= va_arg( Arguments, const char*);
	}

	va_end( Arguments );

	return	NewString;
}

WORD GetLanguageId()
{
	return g_LanguageId;
}

void
	SetLanguageId(
	WORD	LangId,
	HMODULE ResourceModule )
{
	g_LanguageId = LangId;
	g_ResourceModule = ResourceModule;
}

/////////////////////////////////////////////////////////////////////////////
// AllowConversionFlags
//
// Some code pages don't support the converion flags
/////////////////////////////////////////////////////////////////////////////
bool
	AllowConversionFlags(
	ULONG CodePage)
{
	bool ReturnCode = true;

	switch(CodePage)
	{
	case 50220:
	case 50221:
	case 50222:
	case 50225:
	case 50227:
	case 50229:
	case 52936:
	case 54936:
	case 57002:
	case 57003:
	case 57004:
	case 57005:
	case 57006:
	case 57007:
	case 57008:
	case 57009:
	case 57010:
	case 57011:
	case 65000:
	case 65001:
	case 42:
		{
			ReturnCode = false;
			break;
		}
	default:
		{
			ReturnCode = true;
			break;
		}
	}

	return ReturnCode;
}

/////////////////////////////////////////////////////////////////////////////
// GetMultiByteStringFromUnicodeString()
//
// Returns:		int			number of characters written. 0 means error
// Parameters:
//	wchar_t *	szUnicodeString			(IN)	Unicode input string
//	char*		szMultiByteString		(OUT)	Multibyte output string
//	int			nMultiByteBufferSize	(IN)	Multibyte buffer size (chars)
//	ULONG		nCodePage				(IN)	Code page used to perform conversion
//												Default = -1 (Get local code page).
//
// Purpose:		Gets a MultiByte string from a Unicode string
// Exceptions:	None.
/////////////////////////////////////////////////////////////////////////////
DllExport char *
	GetMultiByteStringFromUnicodeString(
	LPCWSTR	UnicodeString,
	ULONG	CodePage)
{
	BOOL	bUsedDefChar	= FALSE;
	int		nCharsWritten = 0;
	char*	MultiByteString = NULL;
	try
	{
		if (UnicodeString)
		{
			// If no code page specified, take default for system
			if (-1 == CodePage)
			{
				CodePage = GetACP();
			}

			int MultiByteBufferSize = WideCharToMultiByte( CodePage,
				0,
				UnicodeString,
				-1,
				NULL,
				0,
				NULL,
				NULL);

			MultiByteString = new char[MultiByteBufferSize];

			// Zero out buffer first
			memset((void*)MultiByteString, '\0', MultiByteBufferSize);

			DWORD Flags	= WC_COMPOSITECHECK | WC_SEPCHARS;

			bool UseFlags	= AllowConversionFlags(CodePage);

			if (false == UseFlags)
			{
				Flags	= 0;
			}

			// If writing to UTF8, flags, default char and boolean flag must be NULL
			nCharsWritten = WideCharToMultiByte(CodePage,
				Flags,
				UnicodeString,
				-1,
				MultiByteString,
				MultiByteBufferSize,
				NULL,
				NULL);

			// If no chars were written and the buffer is not 0, error!
			if (nCharsWritten == 0 && MultiByteBufferSize > 0)
			{
				TCHAR	ErrorMessage[256];
				_stprintf_s(ErrorMessage, _T("\r\nError in WideCharToMultiByte: %d\r\n"), GetLastError());
				ATLTRACE(ErrorMessage);
				MultiByteString[0]	= '\0';
			}
		}

		// Now fix nCharsWritten
		if (nCharsWritten > 0)
		{
			nCharsWritten--;
		}
		else
		{
			//MultiByteString[0]	= '\0';
		}
	}
	catch(...)
	{
		ATLTRACE(_T("Controlled exception in WideCharToMultiByte!\n"));
	}

	return MultiByteString;
}

char*
	GetMultiByteString(
	LPCTSTR UnicodeString)
{
	// if this is not a Unicode build, just use the string itself.
	// this function is then essentially a NOP, but its stamped all over the place.
#ifndef UNICODE
	return UnicodeString;
#else
	char* MultiByteString	= NULL;

	if (NULL != UnicodeString)
	{
		MultiByteString = GetMultiByteStringFromUnicodeString(	UnicodeString,
			-1);
	}

	return MultiByteString;
#endif
}

char*
	GetUtfMultiByteString(
	LPCTSTR UnicodeString)
{
	// if this is not a Unicode build, just use the string itself.
	// this function is then essentially a NOP, but its stamped all over the place.
#ifndef UNICODE
	return UnicodeString;
#else
	char* MultiByteString	= NULL;

	if (NULL != UnicodeString)
	{
		MultiByteString = GetMultiByteStringFromUnicodeString(	UnicodeString,
			CP_UTF8);
	}

	return MultiByteString;
#endif
}

char*
	GetMultiByteString(
	ULONG	CodePage,
	LPCTSTR UnicodeString)
{
	// if this is not a Unicode build, just use the string itself.
	// this function is then essentially a NOP, but its stamped all over the place.
#ifndef UNICODE
	return UnicodeString;
#else
	char* MultiByteString	= NULL;

	if (NULL != UnicodeString) {
		MultiByteString = GetMultiByteStringFromUnicodeString(	UnicodeString,
			CodePage);
	}

	return MultiByteString;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// GetUnicodeStringFromMultiByteString
//
// Returns:		int			num. of chars written (0 means error)
// Parameters:
//	char *		szMultiByteString	(IN)		Multi-byte input string
//	wchar_t*	szUnicodeString		(OUT)		Unicode outputstring
//	int			nUnicodeBufferSize	(IN)		Size of Unicode output buffer in chars(IN)
//	ULONG		nCodePage			(IN)		Code page used to perform conversion
//												Default = -1 (Get local code page).
//
// Purpose:		Gets a Unicode string from a MultiByte string.
/////////////////////////////////////////////////////////////////////////////
int
	GetUnicodeStringFromMultiByteString(
	LPCSTR		szMultiByteString,
	wchar_t*	szUnicodeString,
	int			nUnicodeBufferSize,
	ULONG		nCodePage)
{
	bool	bOK = true;
	int		nCharsWritten = 0;

	if (szUnicodeString && szMultiByteString)
	{
		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		DWORD Flags	= MB_PRECOMPOSED;

		bool UseFlags	= AllowConversionFlags(nCodePage);

		if (false == UseFlags)
		{
			Flags = 0;
		}

		try
		{
			// Zero out buffer first. NB: nUnicodeBufferSize is NUMBER OF CHARS, NOT BYTES!
			memset((void*)szUnicodeString, '\0', sizeof(wchar_t) *
				nUnicodeBufferSize);

			// When converting to UTF8, don't set any flags (see Q175392).
			for (int codePageIndex = 0;;) {
				nCharsWritten = MultiByteToWideChar(nCodePage,
					Flags,
					szMultiByteString,
					-1,
					szUnicodeString,
					nUnicodeBufferSize+1);

				if (nCharsWritten > 0 || alternativeCodePages[codePageIndex] == 0)
					break;

				//If there a problem converting to a wide char with the given code page.
				nCodePage = alternativeCodePages[codePageIndex++];
			}
		}
		catch(CException *ex)
		{
			CString cError;
			LPTSTR pszError = cError.GetBuffer();
			ex->GetErrorMessage(pszError, 3);
			_tprintf(_T("Exception: %s\r\n"), pszError);
			cError.ReleaseBuffer();
			TRACE(_T("Controlled exception in MultiByteToWideChar!\n"));
		}
	}

	ASSERT(nCharsWritten > 0);

	// Now fix nCharsWritten
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}
	else
	{
		GetLastErrorInfo(NULL);
	}

	return nCharsWritten;
}

/////////////////////////////////////////////////////////////////////////////
// GetUnicodeString
/////////////////////////////////////////////////////////////////////////////
wchar_t*
	GetUnicodeString(
	LPCSTR	MultiByteString)
{
	wchar_t*	UnicodeString	= NULL;

	if (NULL != MultiByteString)
	{
		size_t	StringLength = strlen(MultiByteString) + 1;
		UnicodeString = new wchar_t[StringLength];
		GetUnicodeStringFromMultiByteString(MultiByteString, UnicodeString, (int)StringLength, -1);
	}

	return UnicodeString;
}

/////////////////////////////////////////////////////////////////////////////
// GetUnicodeString
/////////////////////////////////////////////////////////////////////////////
wchar_t*
	GetUnicodeString(
	ULONG	CodePage,
	LPCSTR	MultiByteString)
{
	wchar_t*	UnicodeString	= NULL;

	if (NULL != MultiByteString)
	{
		size_t	StringLength = strlen(MultiByteString) + 1;
		UnicodeString = new wchar_t[StringLength];
		GetUnicodeStringFromMultiByteString(MultiByteString, UnicodeString, (int)StringLength, CodePage);
	}

	return UnicodeString;
}

char*
	GetStringCopyA(
	const char*	SourceString)
{
	char* StringCopy = NULL;

	if (NULL != SourceString)
	{
		BOOL BadString	= IsBadStringPtrA(SourceString,(UINT_PTR)-1);

		if (FALSE == BadString)
		{
			size_t	SourceStringLength = strlen(SourceString) + 1;

			StringCopy = new char[SourceStringLength];

			if (NULL != StringCopy)
			{
				strcpy_s(StringCopy, SourceStringLength, SourceString);
			}
		}
	}
	return StringCopy;
}

/////////////////////////////////////////////////////////////////////////////
// GetStringCopyAmount
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	GetStringCopyAmount(
	LPCTSTR	SourceString,
	size_t	Amount)
{
	TCHAR* StringCopy = NULL;

	if (NULL != SourceString)
	{
		BOOL BadString	= IsBadStringPtr(SourceString,(UINT_PTR)-1);

		if (FALSE == BadString)
		{
			size_t	SourceStringLength = (_tcslen(SourceString) + 1) * sizeof(TCHAR);

			StringCopy = new TCHAR[Amount+1];

			if (NULL != StringCopy)
			{
				_tcsncpy_s(	StringCopy,
					Amount+1,
					SourceString,
					Amount);
			}
		}
	}

	return StringCopy;
}

/////////////////////////////////////////////////////////////////////////////
// GetStringCopyAmountA
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
char*
	GetStringCopyAmountA(
	const char*	SourceString,
	size_t		Amount)
{
	char* StringCopy = NULL;

	if (NULL != SourceString)
	{
		BOOL BadString	= IsBadStringPtrA(SourceString,(UINT_PTR)-1);

		if (FALSE == BadString)
		{
			size_t	SourceStringLength = (strlen(SourceString) + 1);

			StringCopy = new char[Amount+1];

			if (NULL != StringCopy)
			{
				strncpy_s(	StringCopy,
					Amount+1,
					SourceString,
					Amount);
			}
		}
	}

	return StringCopy;
}

/////////////////////////////////////////////////////////////////////////////
// IsUtf8BomMark
/////////////////////////////////////////////////////////////////////////////
bool
	IsUtf8BomMark(
	BYTE ByteInQuestion)
{
	bool	Utf8BomMark	= false;

	if (0xEF == ByteInQuestion)
	{
		Utf8BomMark	= true;
	}

	return Utf8BomMark;
}

/////////////////////////////////////////////////////////////////////////////
// GetBaseFileName
//
// delete returned buffer after use.
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	GetBaseFileName(
	LPCTSTR	FileName)
{
	TCHAR*	BaseFileName	= NULL;

	if (NULL != FileName)
	{
		LPCTSTR	Found	= _tcsrchr(FileName, _T('\\'));

		if (NULL != Found)
		{
			// get past the slash
			Found++;

			BaseFileName	= GetStringCopy(Found);
		}
		else
		{
			BaseFileName	= GetStringCopy(FileName);
		}
	}

	return BaseFileName;
}

/////////////////////////////////////////////////////////////////////////////
// GetUserDataFilePath
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	GetUserDataPath(
	TCHAR*	FileName)
{
	TCHAR*	DataFilePath	= NULL;
	TCHAR*	UserDataPath	= new TCHAR[MAX_PATH];

	HRESULT	ResultCode	= SHGetFolderPath(	NULL,
		CSIDL_PERSONAL|CSIDL_FLAG_CREATE,
		NULL,
		SHGFP_TYPE_CURRENT,
		UserDataPath);

	if(FAILED(ResultCode))
	{
	}
	else
	{
		if (FileName[0] != '\\')
		{
			TCHAR*	TempUserDataPath = ConcatStrings(UserDataPath, _T("\\"));
			delete[] UserDataPath;
			UserDataPath	= TempUserDataPath;
		}

		DataFilePath	= ConcatStrings(UserDataPath, FileName);

		if (NULL != UserDataPath)
		{
			delete[] UserDataPath;
		}
	}

	return DataFilePath;
}

/////////////////////////////////////////////////////////////////////////////
// GetWindowsDllPath
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
LPCTSTR
	GetWindowsDllPath(
	TCHAR*	FileName)
{
	TCHAR*	DataFilePath	= NULL;
	TCHAR*	UserDataPath	= new TCHAR[MAX_PATH];

	HRESULT	ResultCode	= SHGetFolderPath(	NULL,
		CSIDL_WINDOWS|CSIDL_FLAG_CREATE,
		NULL,
		SHGFP_TYPE_CURRENT,
		UserDataPath);

	if(FAILED(ResultCode))
	{
	}
	else
	{
		if (FileName[0] != '\\')
		{
			TCHAR*	TempUserDataPath = ConcatStrings(UserDataPath, _T("\\"));
			delete[] UserDataPath;
			UserDataPath	= TempUserDataPath;
		}

		DataFilePath	= ConcatStrings(UserDataPath, FileName);

		if (NULL != UserDataPath)
		{
			delete[] UserDataPath;
		}
	}

	return DataFilePath;
}

/////////////////////////////////////////////////////////////////////////////
// GetFileNameTempPath
/////////////////////////////////////////////////////////////////////////////
LPCTSTR	GetFileNameTempPath(
	LPCTSTR	FileName)
{
	TCHAR*	FileNameTempPath		= NULL;
	TCHAR*	FriendlyFileNameBase	= GetBaseFileName(FileName);

	if (NULL != FriendlyFileNameBase)
	{
		TCHAR	TempPath[MAX_PATH]	= {0};

		GetTempPath(MAX_PATH, TempPath);

		FileNameTempPath = ConcatStrings(TempPath, FriendlyFileNameBase);
	}

	return FileNameTempPath;
}

/////////////////////////////////////////////////////////////////////////////
// ShowMessageString
/////////////////////////////////////////////////////////////////////////////
int
	ShowMessageString(
	LPCTSTR Title,
	LPCTSTR Message,
	UINT uType)
{
	int ReturnCode	= MessageBox(GetActiveWindow(), Message, Title, uType);

	return ReturnCode;
}

TCHAR*	StringSprintfInt(
	TCHAR*	OriginalString,
	int	Number)
{
	TCHAR* StringCopy = NULL;

	if (NULL != OriginalString)
	{
		BOOL BadString	= IsBadStringPtr(OriginalString,(UINT_PTR)-1);

		if (FALSE == BadString)
		{
			size_t	SourceStringLength = (_tcslen(OriginalString) + 1) * sizeof(TCHAR) + 16;

			StringCopy = new TCHAR[SourceStringLength];

			if (NULL != StringCopy)
			{
				_stprintf_s(StringCopy, SourceStringLength, OriginalString, Number);
			}
		}
	}

	return StringCopy;
}

/////////////////////////////////////////////////////////////////////////////
// GetTempFilePathName
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	GetTempFilePathName(
	LPCTSTR	ApplicationName)
{
	TCHAR*	TempFileName	= new TCHAR[MAX_PATH];

	if (NULL != TempFileName)
	{
		TCHAR	TempPath[MAX_PATH] = {0};

		GetTempPath(MAX_PATH, TempPath);

		GetTempFileName(TempPath, ApplicationName, 0, TempFileName);
	}

	return TempFileName;
}

/////////////////////////////////////////////////////////////////////////////
// GetLastErrorInfo
/////////////////////////////////////////////////////////////////////////////
bool
	GetLastErrorInfo(
	HMODULE	ModuleHandle)
{
	bool	bRet		= false;

	LPVOID	pMsgBuffer = NULL;
	DWORD	dwErr		= GetLastError();
	DWORD	dwRet;
	DWORD	FormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
//	LPTSTR buffer = reinterpret_cast<LPTSTR>(&pMsgBuffer);
	LPTSTR buffer = NULL;

	if (NULL != ModuleHandle)
	{
		FormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
	}

	dwRet	= FormatMessage(FormatFlags,
		ModuleHandle,
		dwErr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&buffer,
		0,
		NULL);

	if (NULL != buffer)
	{
		OutputDebugString(buffer);
		//TRACE(_T("Error: %s\r\n"), buffer);
		_tprintf(_T("\tError: %s"), buffer);
		LocalFree(pMsgBuffer);
	}

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// ShowTestResult
/////////////////////////////////////////////////////////////////////////////
void
	ShowTestResult(
	TCHAR * TestName,
	long TestResult)
{
	if (TestResult > 0)
	{
		_tprintf(_T("**%s: succeeded\r\n"), TestName);
	}
	else
	{
		_tprintf(_T("--%s: failed\r\n"), TestName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// ShowConditionResult
/////////////////////////////////////////////////////////////////////////////
void
	ShowConditionResult(
	TCHAR * TestName,
	long TestResult,
	long ExpectedResult)
{
	if (TestResult == ExpectedResult)
	{
		_tprintf(_T("%s: succeeded\r\n"), TestName);
	}
	else
	{
		_tprintf(_T("%s: failed\r\n"), TestName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// ShowConditionResultNot
/////////////////////////////////////////////////////////////////////////////
void
	ShowConditionResultNot(
	TCHAR * TestName,
	long TestResult,
	long ExpectedResult)
{
	if (TestResult != ExpectedResult)
	{
		_tprintf(_T("%s: succeeded\r\n"), TestName);
	}
	else
	{
		_tprintf(_T("%s: failed\r\n"), TestName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// ShowTestResultNot
/////////////////////////////////////////////////////////////////////////////
void
	ShowConditionResultNot(
	TCHAR*	TestName,
	void*	TestResult,
	void*	ExpectedResult)
{
	if (TestResult != ExpectedResult)
	{
		_tprintf(_T("%s: succeeded\r\n"), TestName);
	}
	else
	{
		_tprintf(_T("%s: failed\r\n"), TestName);
	}
}

#endif

/////////////////////////////////////////////////////////////////////////////
// ConcatStrings
// Caller is responsible for 'delete'ing the string.
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	ConcatStrings(
	LPCTSTR	FirstString,
	LPCTSTR	SecondString)
{
	TCHAR*	ConcatString		= NULL;

	if ((NULL != FirstString) || (NULL != SecondString))
	{
		try
		{
			if (NULL == FirstString)
			{
				ConcatString = GetStringCopy(SecondString);
			}
			else if (NULL == SecondString)
			{
				ConcatString = GetStringCopy(FirstString);
			}
			else
			{
				UINT	ConcatStringLength = (UINT)_tcslen(FirstString) +
					(UINT)_tcslen(SecondString) +
					1;

				ConcatString = new TCHAR[ConcatStringLength];

				_tcscpy_s(ConcatString, ConcatStringLength, FirstString);
				_tcscat_s(ConcatString, ConcatStringLength, SecondString);
			}
		}
		catch(TCHAR* Exception)
		{
			// log
			OutputDebugString(Exception);
			throw;
		}
	}
	return ConcatString;
}

/////////////////////////////////////////////////////////////////////////////
// GetStringCopy
//
// delete after use.
/////////////////////////////////////////////////////////////////////////////
TCHAR*
	GetStringCopy(
	LPCTSTR	SourceString)
{
	TCHAR* StringCopy = NULL;

	if (NULL != SourceString)
	{
		BOOL BadString	= IsBadStringPtr(SourceString,(UINT_PTR)-1);

		if (FALSE == BadString)
		{
			size_t	SourceStringLength = (_tcslen(SourceString) + 1) * sizeof(TCHAR);

			StringCopy = new TCHAR[SourceStringLength];

			if (NULL != StringCopy)
			{
				_tcscpy_s(StringCopy, SourceStringLength, SourceString);
			}
		}
	}

	return StringCopy;
}

TCHAR* __cdecl
	ConcatStringsV(
	LPCTSTR	FirstString,
	LPCTSTR	SecondString,
	...)
{
	TCHAR*	NewString	= NULL;
	va_list	Arguments;

	va_start( Arguments, SecondString );

	TCHAR*	OldString	= NewString = ConcatStrings(FirstString, SecondString);

	TCHAR*	NextArg	= va_arg( Arguments, TCHAR*);

	while (NULL != NextArg)
	{
		NewString	= ConcatStrings(OldString, NextArg);

		delete OldString;
		OldString	= NewString;

		NextArg	= va_arg( Arguments, TCHAR*);
	}

	va_end( Arguments );

	return	NewString;
}

bool
	IsEmailValid(const TCHAR* Address)
{
	int				Count = 0;
	const TCHAR*	c;
	const TCHAR*	Domain;
	static TCHAR*	Rfc822Specials = _T("()<>@,;:\\\"[]");

	// first we validate the length of the name and domain (name@domain)
	// name must be no longer than 64 chars and domain must be no longer than 255 chars
	const TCHAR* t = wcschr(Address, '@');
	if (!t)
		return false;
	size_t len = (size_t)(t - Address);
	if (len > 64)
		return false;
	len = wcslen(t);
	if (len > 256)
		return false;

	// then we validate the name portion (name@domain)
	for (c = Address; *c; c++)
	{
		if (*c == '\"' &&
			(c == Address || *(c - 1) == '.' || *(c - 1) == '\"'))
		{
			while (*++c)
			{
				if (*c == '\"')
					break;
				if (*c == '\\' && (*++c == ' '))
					continue;
				if (*c <= ' ' || *c >= 127)
					return false;
			}
			if (!*c++)
				return false;
			if (*c == '@')
				break;
			if (*c != '.')
				return false;
			continue;
		}
		if (*c == '@')
			break;
		if (*c <= ' ' || *c >= 127)
			return false;
		if (_tcschr(Rfc822Specials, *c))
			return false;
	}
	if (c == Address || *(c - 1) == '.')
		return false;

	// then we validate the domain portion (name@domain)
	if (!*(Domain = ++c))
		return false;
	do
	{
		if (*c == '.')
		{
			if (c == Domain || *(c - 1) == '.')
				return false;
			Count++;
		}
		if (*c <= ' ' || *c >= 127)
			return false;
		if (_tcschr(Rfc822Specials, *c))
			return false;
	}
	while (*++c);

	return true;
}
#ifndef INSTALLHELPER_EXPORTS

int GetDigitsFromString(char *str, int index)
{
	return (str[index] - '0') * 10 + (str[index + 1] - '0');
}

void GetDateFromYYMMDDHHMMSS(char *str, TCHAR* formattedDate, size_t len)
{
	struct tm tm;
	int year = GetDigitsFromString(str, 0);
	if (year < 50)
	{
		year += 100;
	}

	tm.tm_year	= year;
	tm.tm_mon	= GetDigitsFromString(str, 2) - 1;
	tm.tm_mday	= GetDigitsFromString(str, 4);
	tm.tm_hour	= GetDigitsFromString(str, 6);
	tm.tm_min	= GetDigitsFromString(str, 8);
	tm.tm_sec	= 0;

	time_t t = str[strlen(str) - 1] == 'Z' ? _mkgmtime(&tm) : mktime(&tm);
	localtime_s(&tm, &t);

	swprintf_s(formattedDate, len, DATE_FORMAT, tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
}

void GetDateFromTime_t(time_t* time_t, TCHAR* formattedDate, size_t len)
{
	struct tm tm;
	gmtime_s(&tm, time_t);

	//wcsftime(formattedDate, len, _T("%m/%d %Y %H:%M"), &tm);
	swprintf_s(formattedDate, len, DATE_FORMAT, tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
}

#endif