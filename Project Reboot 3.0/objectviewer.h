#pragma once

#include "reboot.h"
#include <fstream>

static inline bool IsPropertyA(void* Property, UClass* Class)
{
	if (Fortnite_Version < 12.10)
	{
		if (((UField*)Property)->IsA(Class))
			return true;
	}
	else
	{
		// TODO
	}

	return false;
}

namespace ObjectViewer
{
	void DumpContentsToFile(UObject* Object, const std::string& FileName = "", bool bExcludeUnhandled = false);

	static inline void DumpContentsToFile(const std::string& ObjectName, const std::string& FileName = "", bool bExcludeUnhandled = false) { return DumpContentsToFile(FindObject(ObjectName), FileName, bExcludeUnhandled); }
}