#include "objectviewer.h"

void ObjectViewer::DumpContentsToFile(UObject* Object, const std::string& FileName, bool bExcludeUnhandled)
{
	if (!Object->IsValidLowLevel())
	{
		LOG_ERROR(LogObjectViewer, "Invalid object passed into DumpContentsToFile!");
		return;
	}

	static auto ClassClass = FindObject<UClass>(L"/Script/CoreUObject.Class");

	if (Object->IsA(ClassClass))
	{
		LOG_ERROR(LogObjectViewer, "Object passed into DumpContentsToFile was a class!");
		return;
	}

	static auto BytePropertyClass = FindObject<UClass>(L"/Script/CoreUObject.ByteProperty");
	static auto ObjectPropertyClass = FindObject<UClass>(L"/Script/CoreUObject.ObjectProperty");
	static auto ClassPropertyClass = FindObject<UClass>(L"/Script/CoreUObject.ClassProperty");
	static auto DoublePropertyClass = FindObject<UClass>(L"/Script/CoreUObject.DoubleProperty");
	static auto FloatPropertyClass = FindObject<UClass>(L"/Script/CoreUObject.FloatProperty");
	static auto Int8PropertyClass = FindObject<UClass>(L"/Script/CoreUObject.Int8Property");
	static auto EnumPropertyClass = FindObject<UClass>(L"/Script/CoreUObject.EnumProperty");
	static auto ArrayPropertyClass = FindObject<UClass>(L"/Script/CoreUObject.ArrayProperty");
	static auto Int64PropertyClass = FindObject<UClass>(L"/Script/CoreUObject.Int64Property");
	static auto UInt16PropertyClass = FindObject<UClass>(L"/Script/CoreUObject.UInt16Property");
	static auto BoolPropertyClass = FindObject<UClass>(L"/Script/CoreUObject.BoolProperty");
	static auto NamePropertyClass = FindObject<UClass>(L"/Script/CoreUObject.NameProperty");
	static auto UInt32PropertyClass = FindObject<UClass>(L"/Script/CoreUObject.UInt32Property");
	static auto FunctionClass = FindObject<UClass>(L"/Script/CoreUObject.Function");
	static auto IntPropertyClass = FindObject<UClass>(L"/Script/CoreUObject.IntProperty");
	static auto UInt64PropertyClass = FindObject<UClass>(L"/Script/CoreUObject.UInt64Property");
	static auto StrPropertyClass = FindObject<UClass>(L"/Script/CoreUObject.StrProperty");
	static auto SoftObjectPropertyClass = FindObject<UClass>(L"/Script/CoreUObject.SoftObjectProperty");

	std::ofstream Stream(FileName);

	if (!FileName.empty() && !Stream.is_open())
	{
		LOG_ERROR(LogObjectViewer, "Failed to open file {}!", FileName);
		return;
	}

	auto log = [&](const std::string& str) {
		if (FileName.empty())
		{
			LOG_INFO(LogObjectViewer, "{}", str);
		}
		else
		{
			Stream << str;
		}
	};

	for (auto CurrentClass = Object->ClassPrivate; CurrentClass; CurrentClass = (UClass*)CurrentClass->GetSuperStruct())
	{
		void* Property = *(void**)(__int64(CurrentClass) + Offsets::Children);

		while (Property)
		{
			std::string PropertyName = GetFNameOfProp(Property)->ToString();
			int Offset = *(int*)(__int64(Property) + Offsets::Offset_Internal);

			// log(std::format("Handling prop {}\n", PropertyName));

			if (Offsets::PropertyClass)
			{
				if (IsPropertyA(Property, ObjectPropertyClass))
				{
					auto PropertyClass = *(UClass**)(__int64(Property) + Offsets::PropertyClass);

					if (PropertyClass->IsValidLowLevel())
						log(std::format("{} Object: {}\n", PropertyName, PropertyClass->GetFullName()));
				}

				/*
				else if (IsPropertyA(Property, SoftObjectPropertyClass))
				{
					auto PropertyClass = *(UClass**)(__int64(Property) + Offsets::PropertyClass);

					if (PropertyClass->IsValidLowLevel())
					{
						auto SoftObjectPtr = *(TSoftObjectPtr<UObject>*)(__int64(Object) + Offset);
						auto SoftObjectPtrObject = SoftObjectPtr.Get(PropertyClass);
						log(std::format("{} SoftObjectPtr (type: {}): {}\n", PropertyName, PropertyClass->GetName(), SoftObjectPtrObject ? SoftObjectPtrObject->GetPathName() : "BadRead"));
					}
				}
				*/
			}

			if (IsPropertyA(Property, BytePropertyClass))
			{
				log(std::format("uint8 {} = {}\n", PropertyName, *(uint8*)(__int64(Object) + Offset)));
			}
			else if (IsPropertyA(Property, DoublePropertyClass))
			{
				log(std::format("double {} = {}\n", PropertyName, *(double*)(__int64(Object) + Offset)));
			}
			else if (IsPropertyA(Property, UInt16PropertyClass))
			{
				log(std::format("uint16 {} = {}\n", PropertyName, *(uint16*)(__int64(Object) + Offset)));
			}
			else if (IsPropertyA(Property, Int8PropertyClass))
			{
				log(std::format("int8 {} = {}\n", PropertyName, *(int8*)(__int64(Object) + Offset)));
			}
			else if (IsPropertyA(Property, NamePropertyClass))
			{
				log(std::format("FName {} = {}\n", PropertyName, (*(FName*)(__int64(Object) + Offset)).ToString()));
			}
			else if (IsPropertyA(Property, StrPropertyClass))
			{
				auto string = (FString*)(__int64(Object) + Offset);

				log(std::format("FString {} = {}\n", PropertyName, string->Data.Data ? string->ToString() : ""));
			}
			else if (IsPropertyA(Property, FloatPropertyClass))
			{
				log(std::format("float {} = {}\n", PropertyName, *(float*)(__int64(Object) + Offset)));
			}
			else if (IsPropertyA(Property, BoolPropertyClass))
			{
				auto FieldMask = GetFieldMask(Property);

				log(std::format("bool {} = {}\n", PropertyName, ReadBitfield((PlaceholderBitfield*)(__int64(Object) + Offset), FieldMask)));
			}
			else if (IsPropertyA(Property, IntPropertyClass))
			{
				log(std::format("int32 {} = {}\n", PropertyName, *(int*)(__int64(Object) + Offset)));
			}
			else if (IsPropertyA(Property, UInt32PropertyClass))
			{
				log(std::format("uint32 {} = {}\n", PropertyName, *(uint32*)(__int64(Object) + Offset)));
			}
			else if (IsPropertyA(Property, UInt64PropertyClass))
			{
				log(std::format("uint64 {} = {}\n", PropertyName, *(uint64*)(__int64(Object) + Offset)));
			}
			else if (IsPropertyA(Property, Int64PropertyClass))
			{
				log(std::format("int64 {} = {}\n", PropertyName, *(int64*)(__int64(Object) + Offset)));
			}
			else if (IsPropertyA(Property, ArrayPropertyClass))
			{
				log(std::format("{} Array\n", PropertyName));
			}
			/*
			else if (IsPropertyA(Property, EnumPropertyClass))
			{
				using UNumericProperty = UObject;

				auto EnumValueIg = *(uint8*)(__int64(Property) + Offset);
				auto UnderlyingType = *(UNumericProperty**)(__int64(Property) + Offsets::UnderlyingType);
				// log(std::format("{} Enum: {}\n", PropertyName, (int)EnumValueIg));
				log(std::format("{} Enum\n", PropertyName));
			}
			*/
			else if (IsPropertyA(Property, FunctionClass))
			{

			}
			else if (!bExcludeUnhandled)
			{
				// log(std::format("{}: {}\n", PropertyName, "UNHANDLED");
				log(std::format("{}: {} {}\n", PropertyName, "UNHANDLED", ""/*, ((UObject*)Property)->GetName()*/));
			}

			Property = GetNext(Property);
		}
	}

	return;
}