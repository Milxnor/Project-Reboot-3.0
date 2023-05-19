#pragma once

#include <unordered_map>

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

	bool operator!=(const FGuid& other)
	{
		return !(*this == other);
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

	static inline void (*ProcessEventOriginal)(const UObject*, UFunction*, void*);

	/* virtual */ void ProcessEvent(UFunction* Function, void* Parms = nullptr)
	{
		// LOG_INFO(LogDev, "PE: 0x{:x}", __int64(ProcessEventOriginal) - __int64(GetModuleHandleW(0)));
		ProcessEventOriginal(this, Function, Parms);
	}

	/* virtual */ void ProcessEvent(UFunction* Function, void* Parms = nullptr) const
	{
		// LOG_INFO(LogDev, "PE: 0x{:x}", __int64(ProcessEventOriginal) - __int64(GetModuleHandleW(0)));
		ProcessEventOriginal(this, Function, Parms);
	}

	std::string GetName() { return NamePrivate.ToString(); }
	std::string GetPathName();
	std::string GetFullName();
	UObject* GetOuter() const { return OuterPrivate; }
	FName GetFName() const { return NamePrivate; }

	class UPackage* GetOutermost() const;
	bool IsA(class UStruct* Other);
	class UFunction* FindFunction(const std::string& ShortFunctionName);

	void* GetProperty(const std::string& ChildName, bool bWarnIfNotFound = true);
	void* GetProperty(const std::string& ChildName, bool bWarnIfNotFound = true) const;
	int GetOffset(const std::string& ChildName, bool bWarnIfNotFound = true);
	int GetOffset(const std::string& ChildName, bool bWarnIfNotFound = true) const;

	template <typename T = UObject*>
	T& Get(int Offset) const { return *(T*)(__int64(this) + Offset); }

	void* GetInterfaceAddress(UClass* InterfaceClass);

	bool ReadBitfieldValue(int Offset, uint8_t FieldMask);
	bool ReadBitfieldValue(const std::string& ChildName, uint8_t FieldMask) { return ReadBitfieldValue(GetOffset(ChildName), FieldMask); }

	void SetBitfieldValue(int Offset, uint8_t FieldMask, bool NewValue);
	void SetBitfieldValue(const std::string& ChildName, uint8_t FieldMask, bool NewValue) { return SetBitfieldValue(GetOffset(ChildName), FieldMask, NewValue); }

	/* template <typename T = UObject*>
	T& GetCached(const std::string& ChildName)
	{
		// We need to find a better way to do this because if there is a member with the same name in a different class then it will return the wrong offset.
		static std::unordered_map<std::string, int32_t> SavedOffsets; // Name (formatted in {Member}) and Offset

		auto CachedName = // ClassPrivate->GetName() +
			ChildName;
		auto Offset = SavedOffsets.find(CachedName);

		if (Offset != SavedOffsets.end())
		{
			int off = Offset->second;

			return *(T*)(__int64(this) + off);
		}

		auto Offset = Get(ChildName);

		SavedOffsets.emplace(CachedName, Offset->second);

		return *(T*)(__int64(this) + Offset->second); 
	} */

	template <typename T = UObject*>
	T& Get(const std::string& ChildName) { return Get<T>(GetOffset(ChildName)); }

	template <typename T = UObject*>
	T* GetPtr(int Offset) { return (T*)(__int64(this) + Offset); }

	template <typename T = UObject*>
	T* GetPtr(const std::string& ChildName) { return GetPtr<T>(GetOffset(ChildName)); }

	void AddToRoot();
	bool IsValidLowLevel();
	FORCEINLINE bool IsPendingKill() const;

	// static class UClass* StaticClass();
};

/* struct FInternalUObjectBaseUtilityIsValidFlagsChecker
{
	FORCEINLINE static bool CheckObjectValidBasedOnItsFlags(const UObject* Test)
	{
		// Here we don't really check if the flags match but if the end result is the same
			checkSlow(GUObjectArray.IndexToObject(Test->InternalIndex)->HasAnyFlags(EInternalObjectFlags::PendingKill | EInternalObjectFlags::Garbage) == Test->HasAnyFlags(RF_InternalPendingKill | RF_InternalGarbage));
			return !Test->HasAnyFlags(RF_InternalPendingKill | RF_InternalGarbage);
	}
}; */

FORCEINLINE bool IsValidChecked(const UObject* Test)
{
	// if (!Test)
		// return false;

	return true; // FInternalUObjectBaseUtilityIsValidFlagsChecker::CheckObjectValidBasedOnItsFlags(Test);
}