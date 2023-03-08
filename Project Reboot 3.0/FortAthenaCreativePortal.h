#pragma once

#include "BuildingActor.h"

struct FCreativeLoadedLinkData
{
	
};

class AFortAthenaCreativePortal : public ABuildingActor // ABuildingGameplayActor
{
public:
	FCreativeLoadedLinkData* GetIslandInfo()
	{
		static auto CreativeLoadedLinkDataStruct = FindObject<UStruct>("/Script/FortniteGame.CreativeLoadedLinkData");

		if (!CreativeLoadedLinkDataStruct)
			return nullptr;

		static auto IslandInfoOffset = GetOffset("IslandInfo");
		return GetPtr<FCreativeLoadedLinkData>(IslandInfoOffset);
	}

	void* GetOwningPlayer()
	{
		static auto OwningPlayerOffset = GetOffset("OwningPlayer");
		return GetPtr<void>(OwningPlayerOffset);
	}

	bool& GetPortalOpen()
	{
		static auto bPortalOpenOffset = GetOffset("bPortalOpen");
		return Get<bool>(bPortalOpenOffset);
	}

	bool& GetUserInitiatedLoad()
	{
		static auto bUserInitiatedLoadOffset = GetOffset("bUserInitiatedLoad");
		return Get<bool>(bUserInitiatedLoadOffset);
	}

	bool& GetInErrorState()
	{
		static auto bInErrorStateOffset = GetOffset("bInErrorState");
		return Get<bool>(bInErrorStateOffset);
	}

	AFortVolume*& GetLinkedVolume()
	{
		static auto LinkedVolumeOffset = GetOffset("LinkedVolume");
		return Get<AFortVolume*>(LinkedVolumeOffset);
	}

	FString& GetCreatorName()
	{
		auto IslandInfo = GetIslandInfo();

		if (!IslandInfo)
		{
			return *(FString*)0;
		}

		static auto CreatorNameOffset = FindOffsetStruct("/Script/FortniteGame.CreativeLoadedLinkData", "CreatorName");
		return *(FString*)(__int64(IslandInfo) + CreatorNameOffset);
	}
};