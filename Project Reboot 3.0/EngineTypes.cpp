#include "EngineTypes.h"

#include "reboot.h"

UStruct* FHitResult::GetStruct()
{
	static auto Struct = FindObject<UStruct>("/Script/Engine.HitResult");
	return Struct;
}

int FHitResult::GetStructSize()
{
	return GetStruct()->GetPropertiesSize();
}