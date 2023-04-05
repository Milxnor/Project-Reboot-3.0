#pragma once

#include "inc.h"

struct FGenericPlatformTime
{
	static FORCEINLINE uint32 Cycles()
	{
        struct timeval tv{};
        FILETIME ft;
        ULARGE_INTEGER uli{};

        GetSystemTimeAsFileTime(&ft);  // Get current time
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;

        // Convert to microseconds
        uli.QuadPart /= 10;
        uli.QuadPart -= 11644473600000000ULL;

        tv.tv_sec = (long)(uli.QuadPart / 1000000);
        tv.tv_usec = (long)(uli.QuadPart % 1000000);
        return (uint32)((((uint64)tv.tv_sec) * 1000000ULL) + (((uint64)tv.tv_usec)));
	}
};