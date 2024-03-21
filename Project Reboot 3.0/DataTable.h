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
		static auto Class = FindObject<UClass>(L"/Script/Engine.DataTable");
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
};
