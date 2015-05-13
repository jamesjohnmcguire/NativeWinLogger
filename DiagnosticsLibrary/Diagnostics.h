///////////////////////////////////////////////////////////////////////
// Diagnostics.h
//
// Class for handling and reporting diagnostic events.
//
// Copyright (c) 2006-2015 by James John McGuire
// All rights reserved.
///////////////////////////////////////////////////////////////////////
#pragma once

///////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////
#include "../Include/common.h"

///////////////////////////////////////////////////////////////////////
// defines
///////////////////////////////////////////////////////////////////////
const int DIAGNOSTICS_NONE			= 0;
const int DIAGNOSTICS_LOGFILE		= 1;
const int DIAGNOSTICS_POPUPS		= 2;
const int DIAGNOSTICS_CONSOLE		= 4;
const int DIAGNOSTICS_EVENTLOG		= 8;
const int DIAGNOSTICS_FROMREGISTRY	= 16;

VOID
DbgPrintf(LPTSTR fmt, ...);

///////////////////////////////////////////////////////////////////////
// Class: Diagnostics
///////////////////////////////////////////////////////////////////////
class DllExport Diagnostics
{
	public:
		// Properties

		// Methods
			Diagnostics(void);
			Diagnostics(
				TCHAR*	BaseFileName,
				TCHAR*	Version);
			~Diagnostics(void);

			void Report(
				LPCTSTR Message);
			void Report(
				const char* Message);
			bool ReportError(
				LPCTSTR Module,
				HRESULT ErrorCode);
			void ReportException(
				LPCTSTR Module,
				LPCTSTR Message);
			bool ReportGenericError(
				LPCTSTR ErrorString,
				long ErrorCode);
			bool ReportLastError(
				HMODULE	ModuleHandle = NULL);
			bool ReportString(
				LPCTSTR	InfoString,
				LPCTSTR String);
			bool ReportValue(
				LPCTSTR InfoString,
				ULONG_PTR Value);
			bool SetDiagnosticOutput(
				int	nOption);
			void SetRegistryOverride(
				bool RegistryOverride);

	private:
		// Properties
			UINT		m_OutputLevel;
			TCHAR*		m_LogFilePath;
			TCHAR*		m_Version;

		// Methods
			TCHAR* ConcatStrings(
				TCHAR*	FirstString,
				TCHAR*	SecondString);
			TCHAR* GetStringCopy(
				LPCTSTR	SourceString);
			DWORD GetOutputRegistryKey();
			TCHAR* GetUserDataPath(
				TCHAR*	FileName);
			void Write(
				LPCTSTR EventToReport);
			int GetMultiByteStringFromUnicodeString(
				LPCWSTR szUnicodeString,
				//wchar_t * szUnicodeString,
				char* szMultiByteString,
				int nMultiByteBufferSize,
				int nCodePage);

			char* GetMultiByteString(
				LPCWSTR szUnicodeString);

			int GetUnicodeStringFromMultiByteString(
				LPCSTR		szMultiByteString,
				wchar_t*	szUnicodeString,
				int			nUnicodeBufferSize,
				int			nCodePage);

			wchar_t*
			GetUnicodeString(
				LPCSTR	MultiByteString);
};
