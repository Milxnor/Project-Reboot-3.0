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

bool FHitResult::IsBlockingHit()
{
	// return true;
	static auto bBlockingHitOffset = FindOffsetStruct("/Script/Engine.HitResult", "bBlockingHit");
	static auto bBlockingHitFieldMask = GetFieldMask(FindPropertyStruct("/Script/Engine.HitResult", "bBlockingHit"));
	return ReadBitfield((PlaceholderBitfield*)(__int64(this) + bBlockingHitOffset), bBlockingHitFieldMask);
}

FVector& FHitResult::GetLocation()
{
	static auto LocationOffset = FindOffsetStruct("/Script/Engine.HitResult", "Location");
	return *(FVector*)(__int64(this) + LocationOffset);
}

void FHitResult::CopyFromHitResult(FHitResult* Other)
{
	this->GetLocation() = Other->GetLocation();
}