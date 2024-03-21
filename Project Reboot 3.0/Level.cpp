#include "Level.h"
#include "reboot.h"

UWorld*& ULevel::GetOwningWorld()
{
	static auto OwningWorldOffset = GetOffset("OwningWorld");
	return Get<UWorld*>(OwningWorldOffset);
}

bool ULevel::HasVisibilityChangeRequestPending() // T(REP)
{
	// I believe implementation on this changes depending on the version

	auto OwningWorld = GetOwningWorld();

	if (!OwningWorld)
		return false;

	static auto CurrentLevelPendingVisibilityOffset = OwningWorld->GetOffset("CurrentLevelPendingVisibility");
	auto CurrentLevelPendingVisibility = OwningWorld->Get<ULevel*>(CurrentLevelPendingVisibilityOffset);

	static auto CurrentLevelPendingInvisibilityOffset= OwningWorld->GetOffset("CurrentLevelPendingInvisibility");
	auto CurrentLevelPendingInvisibility = OwningWorld->Get<ULevel*>(CurrentLevelPendingInvisibilityOffset);

	return this == CurrentLevelPendingVisibility || this == CurrentLevelPendingInvisibility;
}

bool ULevel::IsAssociatingLevel() // T(REP)
{
	return false;
}

AWorldSettings* ULevel::GetWorldSettings(bool bChecked) const
{
	if (bChecked)
	{
		// checkf(WorldSettings != nullptr, TEXT("%s"), *GetPathName());
	}

	static auto WorldSettingsOffset = GetOffset("WorldSettings");
	return Get<AWorldSettings*>(WorldSettingsOffset);
}

UClass* ULevel::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.Level");
	return Class;
}