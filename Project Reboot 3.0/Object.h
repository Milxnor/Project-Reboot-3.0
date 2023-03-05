#pragma once

#include "ObjectMacros.h"
#include "NameTypes.h"

#include "addresses.h"

class UClass;
class UFunction;

struct FGuid
{
	unsigned int A;
	unsigned int B;
	unsigned int C;
	unsigned int D;

	bool operator==(const FGuid& other)
	{
		return A == other.A && B == other.B && C == other.C && D == other.D;
	}
};

class UObject
{
public:
	void** VFTable;
	/*EObjectFlags */ int32 ObjectFlags;
	int32 InternalIndex;
	UClass* ClassPrivate;
	FName NamePrivate;
	UObject* OuterPrivate;

	static inline void (*ProcessEventOriginal)(UObject*, UFunction*, void*);

	/* virtual */ void ProcessEvent(UFunction* Function, void* Parms = nullptr)
	{
		// LOG_INFO(LogDev, "PE: 0x{:x}", __int64(ProcessEventOriginal) - __int64(GetModuleHandleW(0)));
		ProcessEventOriginal(this, Function, Parms);
	}

	std::string GetName() { return NamePrivate.ToString(); }
	std::string GetFullName();

	bool IsA(UClass* Other);

	int GetOffset(const std::string& ChildName, bool bWarnIfNotFound = true);

	template <typename T = UObject*>
	T& Get(int Offset) { return *(T*)(__int64(this) + Offset); }

	template <typename T = UObject*>
	T& GetCached(const std::string& ChildName)
	{
		// We need to find a better way to do this because if there is a member with the same name in a different class then it will return the wrong offset.
		static std::unordered_map<std::string, int32_t> SavedOffsets; // Name (formatted in {Member}) and Offset

		auto CachedName = /* ClassPrivate->GetName() + */ ChildName;
		auto Offset = SavedOffsets.find(CachedName);

		if (Offset != SavedOffsets.end())
		{
			int off = Offset->second;

			return *(T*)(__int64(this) + off);
		}

		auto Offset = Get(ChildName);

		SavedOffsets.emplace(CachedName, Offset->second);

		return *(T*)(__int64(this) + Offset->second); 
	}

	template <typename T = UObject*>
	T& Get(const std::string& ChildName) { return Get<T>(GetOffset(ChildName)); }

	template <typename T = UObject*>
	T* GetPtr(int Offset) { return (T*)(__int64(this) + Offset); }

	template <typename T = UObject*>
	T* GetPtr(const std::string& ChildName) { return GetPtr<T>(GetOffset(ChildName)); }
};