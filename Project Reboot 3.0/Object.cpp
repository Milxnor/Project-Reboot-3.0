#include "Object.h"

#include "addresses.h"

#include "Class.h"
#include "KismetSystemLibrary.h"
#include "UObjectArray.h"

FName* getFNameOfProp(void* Property)
{
	FName* NamePrivate = nullptr;

	if (Engine_Version >= 425)
		NamePrivate = (FName*)(__int64(Property) + 0x28);
	else
		NamePrivate = &((UField*)Property)->NamePrivate;

	return NamePrivate;
};

void* UObject::GetProperty(const std::string& ChildName, bool bWarnIfNotFound) const
{
	for (auto CurrentClass = ClassPrivate; CurrentClass; CurrentClass = *(UClass**)(__int64(CurrentClass) + Offsets::SuperStruct))
	{
		void* Property = *(void**)(__int64(CurrentClass) + Offsets::Children);

		if (Property)
		{
			// LOG_INFO(LogDev, "Reading prop name..");

			std::string PropName = getFNameOfProp(Property)->ToString();

			// LOG_INFO(LogDev, "PropName: {}", PropName);

			if (PropName == ChildName)
			{
				return Property;
			}

			while (Property)
			{
				if (PropName == ChildName)
				{
					return Property;
				}

				Property = Engine_Version >= 425 ? *(void**)(__int64(Property) + 0x20) : ((UField*)Property)->Next;
				PropName = Property ? getFNameOfProp(Property)->ToString() : "";
			}
		}
	}

	if (bWarnIfNotFound)
		LOG_WARN(LogFinder, "Unable to find0{}", ChildName);

	return nullptr;
}

void* UObject::GetProperty(const std::string& ChildName, bool bWarnIfNotFound)
{
	for (auto CurrentClass = ClassPrivate; CurrentClass; CurrentClass = *(UClass**)(__int64(CurrentClass) + Offsets::SuperStruct))
	{
		void* Property = *(void**)(__int64(CurrentClass) + Offsets::Children);

		if (Property)
		{
			// LOG_INFO(LogDev, "Reading prop name..");

			std::string PropName = getFNameOfProp(Property)->ToString();

			// LOG_INFO(LogDev, "PropName: {}", PropName);

			if (PropName == ChildName)
			{
				return Property;
			}

			while (Property)
			{
				if (PropName == ChildName)
				{
					return Property;
				}

				Property = Engine_Version >= 425 ? *(void**)(__int64(Property) + 0x20) : ((UField*)Property)->Next;
				PropName = Property ? getFNameOfProp(Property)->ToString() : "";
			}
		}
	}

	if (bWarnIfNotFound)
		LOG_WARN(LogFinder, "Unable to find0{}", ChildName);

	return nullptr;
}

int UObject::GetOffset(const std::string& ChildName, bool bWarnIfNotFound)
{
	auto Property = GetProperty(ChildName, bWarnIfNotFound);

	if (!Property)
		return -1;

	return  *(int*)(__int64(Property) + Offsets::Offset_Internal);
}

int UObject::GetOffset(const std::string& ChildName, bool bWarnIfNotFound) const
{
	auto Property = GetProperty(ChildName, bWarnIfNotFound);

	if (!Property)
		return -1;

	return  *(int*)(__int64(Property) + Offsets::Offset_Internal);
}

bool UObject::ReadBitfieldValue(int Offset, uint8_t FieldMask)
{
	return ReadBitfield(this->GetPtr<PlaceholderBitfield>(Offset), FieldMask);
}

void UObject::SetBitfieldValue(int Offset, uint8_t FieldMask, bool NewValue)
{
	SetBitfield(this->GetPtr<PlaceholderBitfield>(Offset), FieldMask, NewValue);
}

std::string UObject::GetFullName()
{
	return ClassPrivate ? ClassPrivate->GetName() + " " + UKismetSystemLibrary::GetPathName(this).ToString() : "NoClassPrivate";
}

bool UObject::IsA(UClass* otherClass)
{
	UClass* super = ClassPrivate;

	while (super)
	{
		if (otherClass == super)
			return true;

		super = *(UClass**)(__int64(super) + Offsets::SuperStruct);
	}

	return false;
}

/* class UClass* UObject::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/CoreUObject.Object");
	return Class;
} */

void UObject::AddToRoot()
{
	auto Item = GetItemByIndex(InternalIndex);

	if (!Item)
	{
		LOG_INFO(LogDev, "Invalid item");
		return;
	}

	Item->SetRootSet();
}

bool UObject::IsValidLowLevel()
{
	if (this == nullptr)
	{
		// UE_LOG(LogUObjectBase, Warning, TEXT("NULL object"));
		return false;
	}
	if (IsBadReadPtr(this, 8)) // needed?
	{
		return false;
	}
	if (!ClassPrivate)
	{
		// UE_LOG(LogUObjectBase, Warning, TEXT("Object is not registered"));
		return false;
	}
	return ChunkedObjects ? ChunkedObjects->IsValid(this) : UnchunkedObjects ? UnchunkedObjects->IsValid(this) : false;
}

FORCEINLINE bool UObject::IsPendingKill() const
{
	return ChunkedObjects ? ChunkedObjects->GetItemByIndex(InternalIndex)->IsPendingKill() : UnchunkedObjects ? UnchunkedObjects->GetItemByIndex(InternalIndex)->IsPendingKill() : false;
}