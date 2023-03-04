#include "FortItem.h"

void UFortItem::SetOwningControllerForTemporaryItem(UObject* Controller)
{
	static auto SOCFTIFn = FindObject<UFunction>("/Script/FortniteGame.FortItem.SetOwningControllerForTemporaryItem");
	this->ProcessEvent(SOCFTIFn, &Controller);
}