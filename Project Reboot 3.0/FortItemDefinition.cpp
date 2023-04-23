#include "FortItemDefinition.h"
#include "CurveTable.h"
#include "DataTable.h"

UFortItem* UFortItemDefinition::CreateTemporaryItemInstanceBP(int Count, int Level)
{
	static auto CreateTemporaryItemInstanceBPFunction = FindObject<UFunction>(L"/Script/FortniteGame.FortItemDefinition.CreateTemporaryItemInstanceBP");
	struct { int Count; int Level; UFortItem* ReturnValue; } CreateTemporaryItemInstanceBP_Params{ Count, Level };

	ProcessEvent(CreateTemporaryItemInstanceBPFunction, &CreateTemporaryItemInstanceBP_Params);

	return CreateTemporaryItemInstanceBP_Params.ReturnValue;
}

float UFortItemDefinition::GetMaxStackSize()
{
	static auto MaxStackSizeOffset = this->GetOffset("MaxStackSize");

	bool bIsScalableFloat = Fortnite_Version >= 12; // idk

	if (!bIsScalableFloat)
		return Get<int>(MaxStackSizeOffset);

    struct FScalableFloat
    {
    public:
        float                                        Value;                                             // 0x0(0x4)(Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
        uint8                                        Pad_3BF0[0x4];                                     // Fixing Size After Last Property  [ Dumper-7 ]
        FCurveTableRowHandle                  Curve;                                             // 0x8(0x10)(Edit, BlueprintVisible, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
    };

    static auto AthenaGameData = FindObject<UDataTable>("/Game/Athena/Balance/DataTables/AthenaGameData.AthenaGameData");

    auto& ScalableFloat = Get<FScalableFloat>(MaxStackSizeOffset);
    auto& RowMap = AthenaGameData->GetRowMap();

    if (ScalableFloat.Curve.RowName.ComparisonIndex.Value == 0)
        return ScalableFloat.Value;

	FSimpleCurve* Curve = nullptr;

	for (auto& Pair : RowMap)
	{
		if (Pair.Key() == ScalableFloat.Curve.RowName)
		{
			Curve = (FSimpleCurve*)Pair.Value();
			break;
		}
	}

	if (!Curve)
		return 1;

    return Curve->GetKeys().at(0).Value;
}