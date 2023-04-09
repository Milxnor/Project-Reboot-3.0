#pragma once

#include <locale>

#include "Array.h"
#include "log.h"

class FString
{
public:
	TArray<TCHAR> Data;

public:
	std::string ToString() const
	{
		auto length = std::wcslen(Data.Data);
		std::string str(length, '\0');
		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data.Data, Data.Data + length, '?', &str[0]);

		return str;
	}

	void Free()
	{
		Data.Free();
	}

	bool IsValid()
	{
		return Data.Data;
	}

	void Set(const wchar_t* NewStr) // by fischsalat
	{
		if (!NewStr/* || std::wcslen(NewStr) == 0 */) return;

		Data.ArrayMax = Data.ArrayNum = *NewStr ? (int)std::wcslen(NewStr) + 1 : 0;

		if (Data.ArrayNum)
			Data.Data = const_cast<wchar_t*>(NewStr);
	}

	FString() {}

	FString(const wchar_t* str)
	{
		Set(str);
	}

	~FString()
	{
		if (Data.Data)
		{
			// LOG_INFO(LogDev, "Deconstructing FString!");
		}

		// Free();
	}
};