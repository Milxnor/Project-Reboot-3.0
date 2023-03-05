#include "FortItemDefinition.h"

UFortItem* UFortItemDefinition::CreateTemporaryItemInstanceBP(int Count, int Level)
{
	static auto CreateTemporaryItemInstanceBPFunction = FindObject<UFunction>(L"/Script/FortniteGame.FortItemDefinition.CreateTemporaryItemInstanceBP");
	struct { int Count; int Level; UFortItem* ReturnValue; } CreateTemporaryItemInstanceBP_Params{ Count, 1 };

	ProcessEvent(CreateTemporaryItemInstanceBPFunction, &CreateTemporaryItemInstanceBP_Params);

	return CreateTemporaryItemInstanceBP_Params.ReturnValue;
}

float UFortItemDefinition::GetMaxStackSize()
{
	static auto MaxStackSizeOffset = this->GetOffset("MaxStackSize");

	bool bIsScalableFloat = false;

	if (!bIsScalableFloat)
		return Get<int>(MaxStackSizeOffset);

	return 0;
}