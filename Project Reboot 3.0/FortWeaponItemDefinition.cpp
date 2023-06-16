#include "FortWeaponItemDefinition.h"

#include "DataTable.h"
#include "SoftObjectPtr.h"

int UFortWeaponItemDefinition::GetClipSize()
{
	static auto WeaponStatHandleOffset = GetOffset("WeaponStatHandle");
	auto& WeaponStatHandle = Get<FDataTableRowHandle>(WeaponStatHandleOffset);

	auto Table = WeaponStatHandle.DataTable;

	if (!Table)
		return 0;

	auto& RowMap = Table->GetRowMap();

	void* Row = nullptr;

	for (int i = 0; i < RowMap.Pairs.Elements.Data.Num(); ++i)
	{
		auto& Pair = RowMap.Pairs.Elements.Data.at(i).ElementData.Value;

		if (Pair.Key() == WeaponStatHandle.RowName)
		{
			Row = Pair.Value();
			break;
		}
	}

	if (!Row)
		return 0;

	static auto ClipSizeOffset = FindOffsetStruct("/Script/FortniteGame.FortBaseWeaponStats", "ClipSize");
	return *(int*)(__int64(Row) + ClipSizeOffset);
}

UFortWorldItemDefinition* UFortWeaponItemDefinition::GetAmmoData()
{
	static auto AmmoDataOffset = GetOffset("AmmoData");
	auto AmmoData = GetPtr<TSoftObjectPtr<UFortWorldItemDefinition>>(AmmoDataOffset);
	return AmmoData->Get();
}

UClass* UFortWeaponItemDefinition::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortWeaponItemDefinition");
	return Class;
}