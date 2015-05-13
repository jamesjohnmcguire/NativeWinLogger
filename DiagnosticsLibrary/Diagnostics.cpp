///////////////////////////////////////////////////////////////////////
// Diagnostics.cpp - Class Implementation
//
// Class for handling and reporting diagnostic events.
//
// Copyright (c) 2006-2015 by James John McGuire
// All rights reserved.
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include <tchar.h>
#include <shlobj.h>
#include <assert.h>
#include "Diagnostics.h"
#include "version.h"
#include "../Utils/Utils.h"

///////////////////////////////////////////////////////////////////////
// DllMain
///////////////////////////////////////////////////////////////////////
BOOL
APIENTRY DllMain(
	HMODULE	hModule,
	DWORD	ul_reason_for_call,
	LPVOID	lpReserved)
{
	return TRUE;
}

VOID DbgPrintf(
	LPTSTR fmt,
	...
	)
{
	va_list marker;
	TCHAR szBuf[256];

	va_start(marker, fmt);
	wvsprintf(szBuf, fmt, marker);
	va_end(marker);

	OutputDebugString(szBuf);
	OutputDebugString(TEXT("\r\n"));
	_tprintf_s(_T("%s\r\n"), szBuf);
}

///////////////////////////////////////////////////////////////////////
// Diagnostics Class Definition
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// Ctors / Dtors
///////////////////////////////////////////////////////////////////////
Diagnostics::Diagnostics(void) :
	m_OutputLevel(0),
	m_LogFilePath(NULL)
{
	m_Version		= GetUnicodeString(VERSION_STRING);
	m_LogFilePath	= GetUserDataPath(_T("\\Zenware.log"));
}

Diagnostics::Diagnostics(
	TCHAR*	BaseFileName,
	TCHAR*	Version) :
		m_OutputLevel(0),
		m_LogFilePath(NULL)
{
	m_Version	= Version;

	if (NULL == BaseFileName)
	{
		BaseFileName	= _T("\\Zenware.log");
	}
	m_LogFilePath	= GetUserDataPath(BaseFileName);
}

Diagnostics::~Diagnostics(void)
{
	if (NULL != m_LogFilePath)
	{
		delete m_LogFilePath;
		m_LogFilePath = NULL;
	}
	if (NULL != m_Version)
	{
		delete m_Version;
		m_Version = NULL;
	}
}

///////////////////////////////////////////////////////////////////////
// SetDiagnosticOutput
// options
//	DIAGNOSTICS_NONE			no debugging information
//	DIAGNOSTICS_LOGFILE			write to logfile
//	DIAGNOSTICS_POPUPS			display popup (messagebox) information
//	DIAGNOSTICS_FROMREGISTRY	get debug level from registry
//								reg key is:
//								HKEY_LOCAL_MACHINE\SOFTWARE\\ZENWARE\Debug
//
//								possible values are hashed flags, so
//								0	= DIAGNOSTICS_NONE
//								1	= DIAGNOSTICS_LOGFILE
//								2	= DIAGNOSTICS_POPUPS
//								no value is equivalent of 0
//								other hashed values ignored
///////////////////////////////////////////////////////////////////////
bool
Diagnostics::SetDiagnosticOutput(
	int	OptionCode)
{
	bool	ReturnCode	= false;

	if (DIAGNOSTICS_FROMREGISTRY == OptionCode)
	{
		LONG	lRet			= -1;
		UINT	nDiagnostics	= GetOutputRegistryKey();

		if (0 != nDiagnostics)
		{
			m_OutputLevel	= nDiagnostics;
			ReturnCode		= true;
		}
		else
		{
			m_OutputLevel	= DIAGNOSTICS_NONE;
			ReturnCode		= false;
		}
	}
	else
	{
		m_OutputLevel	= OptionCode;
		ReturnCode		= true;
	}

	return ReturnCode;
}

void
Diagnostics::Report(
	LPCTSTR Message)
{
	UINT	ActualOutputLevel	= m_OutputLevel;
	if (DIAGNOSTICS_LOGFILE & m_OutputLevel)
	{
		Write(Message);
	}

	if (DIAGNOSTICS_CONSOLE & m_OutputLevel)
	{
		_tprintf_s(_T("%s\r\n"), Message);
	}

	if (DIAGNOSTICS_POPUPS & m_OutputLevel)
	{
		MessageBox( GetActiveWindow(), Message, _T("Zenware"), MB_OK);
	}
}

void
Diagnostics::Report(
	const char* Message)
{
	LPCTSTR UnicodeMessage = GetUnicodeString(Message);

	if (NULL != UnicodeMessage)
	{
		Report(UnicodeMessage);
		delete UnicodeMessage;
	}
}

void
Diagnostics::ReportException(
	LPCTSTR Module,
	LPCTSTR Message)
{
	TCHAR*	ExceptionConst	= _T("Exception: ");
	UINT	ActualOutputLevel	= m_OutputLevel;

	if (NULL == Message)
	{
		Message	= _T("Undefined: ");
	}

	size_t	ErrorMessageLength	= (_tcslen(Module) +
									_tcslen(Message) +
									_tcslen(ExceptionConst) +
									_tcslen(_T(" ")) + 1)
									* sizeof(TCHAR);
	TCHAR*	ErrorMessage		= new TCHAR[ErrorMessageLength];

	if (NULL != ErrorMessage)
	{
		_stprintf_s(ErrorMessage, ErrorMessageLength, _T("%s%s %s"), ExceptionConst, Module, Message);

		ReportLastError();
		Report(ErrorMessage);
		OutputDebugString(ErrorMessage);
		delete ErrorMessage;
	}
}

///////////////////////////////////////////////////////////////////////
// ReportLastError
///////////////////////////////////////////////////////////////////////
bool
Diagnostics::ReportLastError(
	HMODULE	ModuleHandle /* = NULL */)
{
	bool	ReturnCode	= false;

	TCHAR*	ErrorMessageBuffer	= NULL;
	//LPVOID	ErrorMessageBuffer	= NULL;
	DWORD	ErrorCode			= GetLastError();
	DWORD	ResultCode;
	DWORD	FormatFlags			= FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;

	if (NULL != ModuleHandle)
	{
		FormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
	}

	ResultCode	= FormatMessage(FormatFlags,
								ModuleHandle,
								ErrorCode,
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
								(TCHAR*)&ErrorMessageBuffer,
								0,
								NULL);

	if (NULL != ErrorMessageBuffer)
	{
		size_t	ErrorMessageLength	= (_tcslen(ErrorMessageBuffer) + 13) * sizeof(TCHAR);
		TCHAR*	ErrorMessage		= new TCHAR[ErrorMessageLength];

		_stprintf_s(ErrorMessage, ErrorMessageLength, _T("%x: %s"), ErrorCode, ErrorMessageBuffer);

		//TRACE(_T("Error: %s\r\n"), ErrorMsg);
		OutputDebugString(ErrorMessage);
		Report(ErrorMessage);
		delete ErrorMessage;
		LocalFree(ErrorMessageBuffer);

		ReturnCode = true;
	}

	return ReturnCode;
}

void Diagnostics::SetRegistryOverride(bool RegistryOverride)
{
}

///////////////////////////////////////////////////////////////////////
// Write
///////////////////////////////////////////////////////////////////////
void
Diagnostics::Write(
	LPCTSTR EventToReport)
{
	HANDLE		FileHandle				= INVALID_HANDLE_VALUE;
	SYSTEMTIME	CurrentTime;
	TCHAR		CurrentTimeString[128];

	GetLocalTime(&CurrentTime);

	_stprintf_s(CurrentTimeString,
				_T(" %04d/%02d/%02d::%02d:%02d:%02d "),
				CurrentTime.wYear,
				CurrentTime.wMonth,
				CurrentTime.wDay,
				CurrentTime.wHour,
				CurrentTime.wMinute,
				CurrentTime.wSecond);

	if (NULL != m_LogFilePath)
	{
		FileHandle	= CreateFile(m_LogFilePath,
								GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								0,
								OPEN_ALWAYS,
								0,
								0);
	}

	if (INVALID_HANDLE_VALUE != FileHandle)
	{
		DWORD	BytesRead	= 0;

		TCHAR*	TotalMessage = ConcatStringsV(m_Version, CurrentTimeString, EventToReport, _T("\r\n"), NULL);

		if (NULL != TotalMessage)
		{
			size_t	BufferSize		= _tcslen(TotalMessage);

			char*	AnsiTotalMessage	= GetMultiByteString(TotalMessage);

			if (NULL != AnsiTotalMessage)
			{
				SetFilePointer(FileHandle, 0, NULL, FILE_END);

				WriteFile(FileHandle, AnsiTotalMessage, (DWORD)BufferSize, &BytesRead, NULL);

				CloseHandle(FileHandle);

				delete AnsiTotalMessage;
			}

			delete TotalMessage;
		}
	}
}

bool Diagnostics::ReportError(LPCTSTR Module, HRESULT ErrorCode)
{
	bool	ReturnCode	= false;

	TCHAR*	ErrorMessageBuffer	= NULL;
	DWORD	ResultCode;
	DWORD	FormatFlags			= FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;

	ResultCode	= FormatMessage(FormatFlags,
								NULL,
								ErrorCode,
								//MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
								(TCHAR*)&ErrorMessageBuffer,
								0,
								NULL);

	if (NULL != ErrorMessageBuffer)
	{
		size_t	ErrorMessageLength	= (_tcslen(ErrorMessageBuffer) + 13) * sizeof(TCHAR);
		TCHAR*	ErrorMessage		= new TCHAR[ErrorMessageLength];

		_stprintf_s(ErrorMessage, ErrorMessageLength, _T("%x: %s"), ErrorCode, ErrorMessageBuffer);

		//TRACE(_T("Error: %s\r\n"), ErrorMsg);
		OutputDebugString(ErrorMessage);
		Report(ErrorMessage);
		delete ErrorMessage;
		LocalFree(ErrorMessageBuffer);

		ReturnCode = true;
	}

	return ReturnCode;
}

bool
Diagnostics::ReportGenericError(
	LPCTSTR	ErrorString,
	long	ErrorCode)
{
	bool	ReturnCode	= false;

	if (NULL != ErrorString)
	{
		size_t	ErrorMessageLength	= (_tcslen(ErrorString) + 13) * sizeof(TCHAR);
		TCHAR*	ErrorMessage		= new TCHAR[ErrorMessageLength];

		if (NULL != ErrorMessage)
		{
			_stprintf_s(ErrorMessage, ErrorMessageLength, _T("%x: %s"), ErrorCode, ErrorString);

			OutputDebugString(ErrorMessage);
			Report(ErrorMessage);
			delete ErrorMessage;

			ReturnCode = true;
		}
	}

	return ReturnCode;
}

bool
Diagnostics::ReportString(
	LPCTSTR	InfoString,
	LPCTSTR String)
{
	LPCTSTR	TotalString	= ConcatStringsV(InfoString, _T(": "), String);

	Report(TotalString);

	return true;
}

bool
Diagnostics::ReportValue(
	LPCTSTR	InfoString,
	ULONG_PTR	Value)
{
	bool	ReturnCode	= false;

	if (NULL != InfoString)
	{
		size_t	InfoMessageLength	= (_tcslen(InfoString) + 13) * sizeof(TCHAR);
		TCHAR*	InfoMessage			= new TCHAR[InfoMessageLength];

		if (NULL != InfoMessage)
		{
			_stprintf_s(InfoMessage, InfoMessageLength, _T("0x%08X: %s"), Value, InfoString);

			Report(InfoMessage);
			delete InfoMessage;

			ReturnCode = true;
		}
	}

	return ReturnCode;
}

DWORD
Diagnostics::GetOutputRegistryKey()
{
	DWORD	ReturnValue	= 0;
	LONG	lRet		= -1;
	HKEY	hKey;
	DWORD	BufferSize	= sizeof(DWORD);

	lRet = RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
							_T("SOFTWARE\\Zenware"),
							0,
							KEY_READ,
							&hKey);

	if (ERROR_SUCCESS == lRet)
	{
		lRet	= RegQueryValueEx(	hKey,
									_T("Diagnostics"),
									0,
									NULL,
									(LPBYTE)&ReturnValue,
									&BufferSize);

		RegCloseKey( hKey );
	}

	return ReturnValue;
}


///////////////////////////////////////////////////////////////////////
// GetUserDataFilePath
//
// delete after use.
///////////////////////////////////////////////////////////////////////
TCHAR*
Diagnostics::GetUserDataPath(
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
			delete UserDataPath;
			UserDataPath	= TempUserDataPath;
		}
		DataFilePath	= ConcatStrings(UserDataPath, FileName);

		if (NULL != UserDataPath)
		{
			delete UserDataPath;
		}
	}

	return DataFilePath;
}

///////////////////////////////////////////////////////////////////////
// ConcatStrings
// Caller is responsible for 'delete'ing the string.
///////////////////////////////////////////////////////////////////////
TCHAR*
Diagnostics::ConcatStrings(
	TCHAR*	FirstString,
	TCHAR*	SecondString)
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


///////////////////////////////////////////////////////////////////////
// GetStringCopy
//
// delete after use.
///////////////////////////////////////////////////////////////////////
TCHAR*
Diagnostics::GetStringCopy(
	LPCTSTR	SourceString)
{
	TCHAR* StringCopy =  NULL;

	if (NULL != SourceString)
	{
		BOOL BadString	= IsBadStringPtr(SourceString,(UINT_PTR)-1);

		if (FALSE == BadString)
		{
			size_t	SourceStringLength = (_tcslen(SourceString) + 1) * sizeof(TCHAR);

			StringCopy =  new TCHAR[SourceStringLength];

			if (NULL != StringCopy)
			{
				_tcscpy_s(StringCopy, SourceStringLength, SourceString);
			}
		}
	}

	return StringCopy;
}

//TCHAR* __cdecl
//Diagnostics::ConcatStringsV(
//	LPCTSTR	FirstString,
//	LPCTSTR	SecondString,
//	...)
//{
//	TCHAR*	NewString	= NULL;
//	va_list	Arguments;
//
//	va_start( Arguments, SecondString );
//
//	TCHAR*	OldString	= NewString = ConcatStrings(FirstString, SecondString);
//
//	TCHAR*	NextArg	= va_arg( Arguments, TCHAR*);
//
//	while (NULL != NextArg)
//	{
//		NewString	= ConcatStrings(OldString, NextArg);
//
//		delete OldString;
//		OldString	= NewString;
//
//		NextArg	= va_arg( Arguments, TCHAR*);
//	}
//
//	va_end( Arguments );
//
//	return	NewString;
//}

///////////////////////////////////////////////////////////////////////
// GetMultiByteStringFromUnicodeString()
//
// Returns:		int			number of characters written. 0 means error
// Parameters:
//	wchar_t *	szUnicodeString			(IN)	Unicode input string
//	char*		szMultiByteString		(OUT)	Multibyte output string
//	int			nMultiByteBufferSize	(IN)	Multibyte buffer size (chars)
//	UINT		nCodePage				(IN)	Code page used to perform conversion
//												Default = -1 (Get local code page).
//
// Purpose:		Gets a MultiByte string from a Unicode string
// Exceptions:	None.
///////////////////////////////////////////////////////////////////////
int
Diagnostics::GetMultiByteStringFromUnicodeString(
	LPCWSTR	UnicodeString,
	char*	MultiByteString,
	int		MultiByteBufferSize,
	int		CodePage)
{
	BOOL	bUsedDefChar	= FALSE;
	int		nCharsWritten = 0;

	try
	{
		if (0 < MultiByteBufferSize)
		{
			if (UnicodeString && MultiByteString) 
			{
				// Zero out buffer first
				memset((void*)MultiByteString, '\0', MultiByteBufferSize);
			
				// If no code page specified, take default for system
				if (-1 == CodePage)
				{
					CodePage = GetACP();
				}

				DWORD	Flags	= 0;
				//DEBUGDEBUG
				CodePage	= 932;
				if (CP_UTF8 != CodePage)
				{
					Flags	= WC_COMPOSITECHECK | WC_SEPCHARS;
					//Flags	= WC_COMPOSITECHECK;
				}

				// If writing to UTF8, flags, default char and boolean flag must be NULL
				nCharsWritten = WideCharToMultiByte((UINT)CodePage,
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
				}
			}
		} 

		// Now fix nCharsWritten 
		if (nCharsWritten > 0)
		{
			nCharsWritten--;
		}
	}
	catch(...) 
	{
	}
	
	return nCharsWritten;
}

char*
Diagnostics::GetMultiByteString(
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
		size_t	StringLength	= _tcslen(UnicodeString) * 2 + 2;
		MultiByteString			= new char[StringLength];

		int		ResultCode = GetMultiByteStringFromUnicodeString(UnicodeString,
															MultiByteString,
															(int)StringLength,
															-1);
	}

	return MultiByteString;
#endif
}


///////////////////////////////////////////////////////////////////////
// GetUnicodeStringFromMultiByteString
//
// Returns:		int			num. of chars written (0 means error)
// Parameters:
//	char *		szMultiByteString	(IN)		Multi-byte input string
//	wchar_t*	szUnicodeString		(OUT)		Unicode outputstring
//	int			nUnicodeBufferSize	(IN)		Size of Unicode output buffer in chars(IN)
//	UINT		nCodePage			(IN)		Code page used to perform conversion
//												Default = -1 (Get local code page).
//
// Purpose:		Gets a Unicode string from a MultiByte string.
///////////////////////////////////////////////////////////////////////
int
Diagnostics::GetUnicodeStringFromMultiByteString(
	LPCSTR		szMultiByteString,
	wchar_t*	szUnicodeString,
	int			nUnicodeBufferSize,
	int			nCodePage)
{
	bool		bOK = true;
	int		nCharsWritten = 0;
		
	if (szUnicodeString && szMultiByteString)
	{
		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		try 
		{
			// Zero out buffer first. NB: nUnicodeBufferSize is NUMBER OF CHARS, NOT BYTES!
			memset((void*)szUnicodeString, '\0', sizeof(wchar_t) *
				nUnicodeBufferSize);

			// When converting to UTF8, don't set any flags (see Q175392).
			nCharsWritten = MultiByteToWideChar(	(UINT)nCodePage,
													(nCodePage==CP_UTF8 ? 0:MB_PRECOMPOSED), // Flags
													szMultiByteString,
													-1,
													szUnicodeString,
													//0);
													nUnicodeBufferSize+1);

			//if (nCharsWritten < 1)
			//	GetLastErrorInfo();

		}
		catch(...)
		{
		}
	}

	// Now fix nCharsWritten
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}
	
	assert(nCharsWritten > 0);
	return nCharsWritten;
}

///////////////////////////////////////////////////////////////////////
// GetUnicodeString
///////////////////////////////////////////////////////////////////////
wchar_t*
Diagnostics::GetUnicodeString(
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
