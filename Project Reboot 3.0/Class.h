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
	UClass* GetSuperStruct() { return *(UClass**)(__int64(this) + Offsets::SuperStruct); } // idk if this is in UStruct
};

class UFunction : public UStruct
{
public:
	void*& GetFunc() { return *(void**)(__int64(this) + Offsets::Func); }
};
