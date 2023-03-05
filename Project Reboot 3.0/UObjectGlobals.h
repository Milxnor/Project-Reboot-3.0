#pragma once

#include "Object.h"

#define ANY_PACKAGE (UObject*)-1

extern inline UObject* (*StaticFindObjectOriginal)(UClass* Class, UObject* InOuter, const TCHAR* Name, bool ExactClass) = nullptr;

template <typename T = UObject>
static inline T* StaticFindObject(UClass* Class, UObject* InOuter, const TCHAR* Name, bool ExactClass = false)
{
	// LOG_INFO(LogDev, "StaticFindObjectOriginal: {}", __int64(StaticFindObjectOriginal));
	return (T*)StaticFindObjectOriginal(Class, InOuter, Name, ExactClass);
}