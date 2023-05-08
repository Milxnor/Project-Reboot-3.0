#pragma once

#include "Object.h"
#include "Package.h"

#define ANY_PACKAGE (UObject*)-1

extern inline UObject* (*StaticFindObjectOriginal)(UClass* Class, UObject* InOuter, const TCHAR* Name, bool ExactClass) = nullptr;

template <typename T = UObject>
static inline T* StaticFindObject(UClass* Class, UObject* InOuter, const TCHAR* Name, bool ExactClass = false)
{
	// LOG_INFO(LogDev, "StaticFindObjectOriginal: {}", __int64(StaticFindObjectOriginal));
	return (T*)StaticFindObjectOriginal(Class, InOuter, Name, ExactClass);
}

static inline UPackage* GetTransientPackage()
{
	static auto TransientPackage = StaticFindObject<UPackage>(nullptr, nullptr, L"/Engine/Transient");
	return TransientPackage;
}