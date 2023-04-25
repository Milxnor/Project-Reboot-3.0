#pragma once

#include "log.h"
#include "inc.h"

/*
#ifdef PROD
#define UE_DEBUG_BREAK() ((void)0)
#else
#define UE_DEBUG_BREAK() ((void)(FWindowsPlatformMisc::IsDebuggerPresent() && (__debugbreak(), 1)))
#endif

#ifndef PROD
#define _DebugBreakAndPromptForRemote() \
	if (!FPlatformMisc::IsDebuggerPresent()) { FPlatformMisc::PromptForRemoteDebugging(false); } UE_DEBUG_BREAK();
#else
#define _DebugBreakAndPromptForRemote()
#endif 

#define CA_ASSUME( Expr ) // Todo move to Misc/CoreMiscDefines.h

namespace FDebug
{
	template <typename FmtType, typename... Types>
	static void LogAssertFailedMessage(const char* Expr, const char* File, int32 Line, const FmtType& Fmt, Types... Args)
	{
		// static_assert(TIsArrayOrRefOfType<FmtType, TCHAR>::Value, "Formatting string must be a TCHAR array.");
		// tatic_assert(TAnd<TIsValidVariadicFunctionArg<Types>...>::Value, "Invalid argument(s) passed to FDebug::LogAssertFailedMessage");

		LOG_ERROR(LogDev, "Assert failed message {} {}", File, Line);
		// LogAssertFailedMessageImpl(Expr, File, Line, Fmt, Args...);
	}

	void __cdecl AssertFailed(const char* Expr, const char* File, int32 Line, const wchar_t* Format = TEXT(""), ...)
	{
		LOG_ERROR(LogDev, "Assert failed {} {}", File, Line);

		/*
		if (GIsCriticalError)
		{
			return;
		}

		// This is not perfect because another thread might crash and be handled before this assert
		// but this static varible will report the crash as an assert. Given complexity of a thread
		// aware solution, this should be good enough. If crash reports are obviously wrong we can
		// look into fixing this.
		bHasAsserted = true;

		TCHAR DescriptionString[4096];
		GET_VARARGS(DescriptionString, ARRAY_COUNT(DescriptionString), ARRAY_COUNT(DescriptionString) - 1, Format, Format);

		TCHAR ErrorString[MAX_SPRINTF];
		FCString::Sprintf(ErrorString, TEXT("%s"), ANSI_TO_TCHAR(Expr));
		GError->Logf(TEXT("Assertion failed: %s") FILE_LINE_DESC TEXT("\n%s\n"), ErrorString, ANSI_TO_TCHAR(File), Line, DescriptionString);
		
	}
}
*/

// #define check(expr)				{ if(UNLIKELY(!(expr))) { FDebug::LogAssertFailedMessage( #expr, __FILE__, __LINE__, TEXT("") ); _DebugBreakAndPromptForRemote(); FDebug::AssertFailed( #expr, __FILE__, __LINE__ ); CA_ASSUME(false); } }
