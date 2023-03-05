#pragma once

#include "Object.h"

#include "addresses.h"

struct UField : UObject
{
	UField* Next;
	// void* pad; void* pad2;
};

class UStruct : public UField
{
public:
	int GetPropertiesSize();
};

class UClass : public UStruct
{
public:
	UObject* CreateDefaultObject();
};

class UFunction : public UStruct
{
public:
	void* GetFunc() { return *(void**)(__int64(this) + Offsets::Func); }
};
