#pragma once

#include "Object.h"
#include "reboot.h"

#include "Map.h"

struct FTableRowBase
{
	unsigned char                                      UnknownData00[0x8]; // this is actually structural padding
};

class UDataTable : public UObject
{
public:
	template <typename RowDataType = uint8_t>
	TMap<FName, RowDataType*>& GetRowMap()
	{
		static auto RowStructOffset = FindOffsetStruct("/Script/Engine.DataTable", "RowStruct");

		return *(TMap<FName, RowDataType*>*)(__int64(this) + (RowStructOffset + sizeof(UObject*))); // because after rowstruct is rowmap
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/Engine.DataTable");
		return Class;
	}
};

struct FDataTableRowHandle
{
	UDataTable* DataTable;                                                // 0x0000(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FName                                       RowName;                                                  // 0x0008(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

template <typename StructType = uint8>
struct RowNameAndRowData
{
	FName RowName;
	StructType* RowData;
};

struct FDataTableCategoryHandle
{
	UDataTable* DataTable;                                                // 0x0000(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FName                                       ColumnName;                                               // 0x0008(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FName                                       RowContents;                                              // 0x0010(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	template <class T>
	void GetRows(std::vector<T*>& OutRows, const FString& ContextString) const
	{
		OutRows.clear();

		if (DataTable == nullptr)
		{
			if (RowContents.ComparisonIndex.Value != 0)
			{
				// UE_LOG(LogDataTable, Warning, TEXT("FDataTableCategoryHandle::FindRow : No DataTable for row %s (%s)."), *RowContents.ToString(), *ContextString);
			}

			return;
		}

		if (ColumnName.ComparisonIndex.Value == 0)
		{
			if (RowContents.ComparisonIndex.Value != 0)
			{
				// UE_LOG(LogDataTable, Warning, TEXT("FDataTableCategoryHandle::FindRow : No Column selected for row %s (%s)."), *RowContents.ToString(), *ContextString);
			}

			return;
		}

		return;

		// unreal trippin

		/*
		// Find the property that matches the desired column (ColumnName)
		UProperty* Property = DataTable->FindTableProperty(ColumnName);
		if (Property == nullptr)
		{
			return;
		}

		// check each row to see if the value in the Property element is the one we're looking for (RowContents). If it is, add the row to OutRows
		FString RowContentsAsString = RowContents.ToString();

		for (auto RowIt = DataTable->RowMap.CreateConstIterator(); RowIt; ++RowIt)
		{
			uint8* RowData = RowIt.Value();

			FString PropertyValue(TEXT(""));

			Property->ExportText_InContainer(0, PropertyValue, RowData, RowData, nullptr, PPF_None);

			if (RowContentsAsString == PropertyValue)
			{
				OutRows.Add((T*)RowData);
			}
		}
		*/

		return;
	}

};
