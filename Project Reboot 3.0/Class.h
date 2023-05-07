#pragma once

#include "Object.h"

#include "addresses.h"
#include "UnrealString.h"
#include "Map.h"

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

class UEnum : public UField
{
public:
	int64 GetValue(const std::string& EnumMemberName)
	{
		auto Names = (TArray<TPair<FName, __int64>>*)(__int64(this) + sizeof(UField) + sizeof(FString));

		for (int i = 0; i < Names->Num(); i++)
		{
			auto& Pair = Names->At(i);
			auto& Name = Pair.Key();
			auto Value = Pair.Value();

			if (Name.ComparisonIndex.Value && Name.ToString().contains(EnumMemberName))
				return Value;
		}

		return -1;
	}
};