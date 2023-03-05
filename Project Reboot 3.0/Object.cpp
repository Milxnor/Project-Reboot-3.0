#include "Object.h"

#include "addresses.h"

#include "Class.h"
#include "KismetSystemLibrary.h"

int UObject::GetOffset(const std::string& ChildName, bool bWarnIfNotFound)
{
	auto getFNameOfProp = [](void* Property) -> FName*
	{
		FName* NamePrivate = nullptr;

		if (Engine_Version >= 425)
			NamePrivate = (FName*)(__int64(Property) + 0x28);
		else
			NamePrivate = &((UField*)Property)->NamePrivate;

		return NamePrivate;
	};

	for (auto CurrentClass = ClassPrivate; CurrentClass; CurrentClass = *(UClass**)(__int64(CurrentClass) + Offsets::SuperStruct))
	{
		void* Property = *(void**)(__int64(CurrentClass) + Offsets::Children);

		if (Property)
		{
			std::string PropName = getFNameOfProp(Property)->ToString();

			// LOG_INFO(LogDev, "PropName: {}", PropName);

			if (PropName == ChildName)
			{
				return *(int*)(__int64(Property) + Offsets::Offset_Internal);
			}

			while (Property)
			{
				if (PropName == ChildName)
				{
					return *(int*)(__int64(Property) + Offsets::Offset_Internal);
				}

				Property = Engine_Version >= 425 ? *(void**)(__int64(Property) + 0x20) : ((UField*)Property)->Next;
				PropName = Property ? getFNameOfProp(Property)->ToString() : "";
			}
		}
	}

	if (bWarnIfNotFound)
		LOG_WARN(LogFinder, "Unable to find0{}", ChildName);

	return 0;
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
