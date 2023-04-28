#include "World.h"

#include "KismetStringLibrary.h"
#include "Actor.h"

#include "reboot.h"

AWorldSettings* UWorld::K2_GetWorldSettings()
{
	static auto fn = FindObject<UFunction>("/Script/Engine.World.K2_GetWorldSettings");
	AWorldSettings* WorldSettings;
	this->ProcessEvent(fn, &WorldSettings);
	return WorldSettings;
}

void UWorld::Listen()
{
	auto GameNetDriverName = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");

	UNetDriver* NewNetDriver = nullptr;

	constexpr bool bUseBeacons = true;

	int Port = 7777 - Globals::AmountOfListens + 1;

	if (bUseBeacons)
	{
		static auto BeaconClass = FindObject<UClass>(L"/Script/FortniteGame.FortOnlineBeaconHost");
		auto NewBeacon = GetWorld()->SpawnActor<AActor>(BeaconClass);

		if (!NewBeacon)
		{
			LOG_ERROR(LogNet, "Failed to spawn beacon!");
			return;
		}

		static bool (*InitHost)(UObject* Beacon) = decltype(InitHost)(Addresses::InitHost);
		static void (*PauseBeaconRequests)(UObject* Beacon, bool bPause) = decltype(PauseBeaconRequests)(Addresses::PauseBeaconRequests);

		NewBeacon->Get<int>("ListenPort") = Engine_Version < 426 ? Port - 1 : Port;

		InitHost(NewBeacon);
		PauseBeaconRequests(NewBeacon, false);

		NewNetDriver = NewBeacon->Get<UNetDriver*>("NetDriver");
	}
	else
	{
		NewNetDriver = GetEngine()->CreateNetDriver(GetWorld(), GameNetDriverName);
	}

	if (!NewNetDriver)
	{
		LOG_ERROR(LogNet, "Failed to create net driver!");
		return;
	}

	NewNetDriver->Get<FName>("NetDriverName") = GameNetDriverName;
	GetWorld()->Get("NetDriver") = NewNetDriver;

	FURL URL = FURL();
	URL.Port = Port - (Engine_Version >= 426);

	FString Error;

	if (!NewNetDriver->InitListen(GetWorld(), URL, false, Error))
	{
		LOG_ERROR(LogNet, "Failed to init listen!");
		return;
	}

	NewNetDriver->SetWorld(GetWorld());

	// LEVEL COLLECTIONS

	auto& LevelCollections = GetWorld()->Get<TArray<__int64>>("LevelCollections");
	int LevelCollectionSize = FindObject<UStruct>("/Script/Engine.LevelCollection")->GetPropertiesSize();

	*(UNetDriver**)(__int64(LevelCollections.AtPtr(0, LevelCollectionSize)) + 0x10) = NewNetDriver;
	*(UNetDriver**)(__int64(LevelCollections.AtPtr(1, LevelCollectionSize)) + 0x10) = NewNetDriver;

	LOG_INFO(LogNet, "Listening on port {}!", Port + Globals::AmountOfListens - 1);
}

AWorldSettings* UWorld::GetWorldSettings(const bool bCheckStreamingPersistent, const bool bChecked) const
{
	// checkSlow(!IsInActualRenderingThread());
	AWorldSettings* WorldSettings = nullptr;
	static auto PersistentLevelOffset = GetOffset("PersistentLevel");
	if (Get(PersistentLevelOffset))
	{
		WorldSettings = Get<ULevel*>(PersistentLevelOffset)->GetWorldSettings(bChecked);

		if (bCheckStreamingPersistent)
		{
			static auto StreamingLevelsOffset = GetOffset("StreamingLevels");
			auto& StreamingLevels = Get<TArray<UObject*>>(StreamingLevelsOffset);

			static auto LevelStreamingPersistentClass = FindObject<UClass>("/Script/Engine.LevelStreamingPersistent");

			if (StreamingLevels.Num() > 0 &&
				StreamingLevels.at(0) &&
				StreamingLevels.at(0)->IsA(LevelStreamingPersistentClass))
			{
				static auto LoadedLevelOffset = StreamingLevels.at(0)->GetOffset("LoadedLevel");
				ULevel* Level = StreamingLevels.at(0)->Get<ULevel*>(LoadedLevelOffset);
				if (Level != nullptr)
				{
					WorldSettings = Level->GetWorldSettings();
				}
			}
		}
	}
	return WorldSettings;
}