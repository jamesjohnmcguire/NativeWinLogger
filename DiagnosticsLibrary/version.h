/////////////////////////////////////////////////////////////////////////////
// $Id: version.h 22 2015-03-25 13:00:52Z JamesMc $
//
// Copyright (c) 2006-2015 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////
// version.h
#include "version.ver"
#define		_STR(x) #x
#define		STR(x) _STR(x)
#define		VERSION_NUMBER			VERSION_MAJOR,VERSION_MINOR,VERSION_BUILD,VERSION_QFE
#define		VERSION_STRING			STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_BUILD) "." STR(VERSION_QFE)
#define		VERSION_COMPANY			""
#define		VERSION_COPYRIGHT		"Copyright © 2010 by James John McGuire"
#define		VERSION_TRADEMARK		""
#define		VERSION_BUILD_DATE_TIME	VERSION_BUILD_DATE " - " VERSION_BUILD_TIME
