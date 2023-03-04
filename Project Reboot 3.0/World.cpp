#include "World.h"

#include "KismetStringLibrary.h"
#include "Actor.h"

#include "reboot.h"

void UWorld::Listen()
{
	auto GameNetDriverName = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");

	UNetDriver* NewNetDriver = nullptr;

	constexpr bool bUseBeacons = true;

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

	int Port = 7777;

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
	int LevelCollectionSize = 0x78;

	*(UNetDriver**)(__int64(LevelCollections.AtPtr(0, LevelCollectionSize)) + 0x10) = NewNetDriver;
	*(UNetDriver**)(__int64(LevelCollections.AtPtr(1, LevelCollectionSize)) + 0x10) = NewNetDriver;

	LOG_INFO(LogNet, "Listening on port {}!", Port);
}