#pragma once

#include "memcury.h"
#include "inc.h"

#include "hooking.h"

static inline uintptr_t FindBytes(Memcury::Scanner& Scanner, const std::vector<uint8_t>& Bytes, int Count = 255, int SkipBytes = 0, bool bGoUp = false, int Skip = 0, const bool bPrint = false)
{
	if (!Scanner.Get())
	{
		return 0;
	}

	for (int i = 0 + SkipBytes; i < Count + SkipBytes; i++) // we should subtract from skip if goup
	{
		auto CurrentByte = *(Memcury::ASM::MNEMONIC*)(bGoUp ? Scanner.Get() - i : Scanner.Get() + i);

		// if (bPrint)
			// std::cout << "CurrentByte: " << std::hex << (int)CurrentByte << '\n';

		if (CurrentByte == Bytes[0])
		{
			bool Found = true;
			for (int j = 1; j < Bytes.size(); j++)
			{
				if (*(Memcury::ASM::MNEMONIC*)(bGoUp ? Scanner.Get() - i + j : Scanner.Get() + i + j) != Bytes[j])
				{
					Found = false;
					break;
				}
			}
			if (Found)
			{
				if (Skip > 0)
				{
					Skip--;
					continue;
				}

				return bGoUp ? Scanner.Get() - i : Scanner.Get() + i;
			}
		}

		// std::cout << std::format("CurrentByte: 0x{:x}\n", (uint8_t)CurrentByte);
	}

	return -1;// Scanner.Get();
}

static inline uint64 FindStaticFindObject()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"Illegal call to StaticFindObject() while serializing object data!");
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 255, 0, true); // Addr.ScanFor(bytes, false).Get();
}

static inline uint64 FindProcessEvent()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"AccessNoneNoContext");
	return FindBytes(Addr, { 0x40, 0x55 }, 2000); // Addr.ScanFor({ 0x40, 0x55 }).Get();
}

static inline uint64 FindObjectArray()
{

}

static inline uint64 FindCreateNetDriver()
{
	return 0;
}

static inline uint64 FindKickPlayer()
{
	if (Engine_Version >= 423 || Engine_Version <= 425)
		return Memcury::Scanner::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 49 8B F0 48 8B DA 48 85 D2").Get();

	// return 0;

	// return Memcury::Scanner::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 49 8B F0 48 8B DA 48 85 D2").Get(); // 12.41

	uint64 Ret = 0;

	auto Addr = Memcury::Scanner::FindStringRef(L"Validation Failure: %s. kicking %s", false);

	if (Addr.Get())
	{
		Ret = Addr.Get() ? FindBytes(Addr, { 0x40, 0x55 }, 1000, 0, true) : Ret;

		if (!Ret)
			Ret = Addr.Get() ? FindBytes(Addr, { 0x40, 0x53 }, 2000, 0, true) : Ret;
	}

	if (Ret)
		return Ret;

	auto Addr2 = Memcury::Scanner::FindStringRef(L"Failed to kick player"); // L"KickPlayer %s Reason %s"
	Ret = Addr2.Get() ? FindBytes(Addr2, { 0x48, 0x89, 0x5C }, 2000, 0, true) : Ret; // s12??
	// Ret = Addr2.Get() ? FindBytes(Addr2, { 0x48, 0x8B, 0xC4 }, 2000, 0, true) : Ret;

	if (Ret)
		return Ret;

	/* auto Addr3 = Memcury::Scanner::FindStringRef(L"Game already ended.");
	Ret = Addr3.Get() ? FindBytes(Addr3, { 0x48, 0x89, 0x5C }, 2000, 0, true) : Ret;

	if (Ret)
		return Ret; */

	Ret = Memcury::Scanner::FindPattern("40 53 41 56 48 81 EC ? ? ? ? 48 8B 01 48 8B DA 4C 8B F1 FF 90").Get();

	return Ret;
}

static inline uint64 FindInitHost()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"BeaconPort=");
	return FindBytes(Addr, { 0x48, 0x8B, 0xC4 }, 1000, 0, true);
}

static inline uint64 FindPauseBeaconRequests()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"All Beacon Requests Resumed.");
	return FindBytes(Addr, { 0x40, 0x53 }, 1000, 0, true);
}

static inline uint64 FindGetPlayerViewpoint()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"APlayerController::GetPlayerViewPoint: out_Location, ViewTarget=%s");
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true, 1);
}

static inline uint64 FindSpawnActor()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"SpawnActor failed because no class was specified");
	return FindBytes(Addr, { 0x4C, 0x8B, 0xDC }, 3000, 0, true);
}

static inline uint64 FindSetWorld()
{
	if (Engine_Version < 426)
		return Memcury::Scanner::FindStringRef(L"AOnlineBeaconHost::InitHost failed")
			.ScanFor({ 0x48, 0x8B, 0xD0, 0xE8 }, false)
			.RelativeOffset(4)
			.Get(); // THANKS ENDER

	int SetWorldIndex = 0;

	int Fortnite_Season = std::floor(Fortnite_Version);

	if (Fortnite_Season == 13)
		SetWorldIndex = 0x70;
	else if (Fortnite_Season == 14 || Fortnite_Version <= 15.2)
		SetWorldIndex = 0x71;
	else if (Fortnite_Version >= 15.3 && Fortnite_Season < 18) // i havent tested 15.2
		SetWorldIndex = 0x72;
	else if (Fortnite_Season == 18)
		SetWorldIndex = 0x73;
	else if (Fortnite_Season >= 19 && Fortnite_Season < 21)
		SetWorldIndex = 0x7A;

	// static auto DefaultNetDriver = FindObject("/Script/Engine.Default__NetDriver");
	return SetWorldIndex;
}

static inline uint64 FindInitListen() 
{
	auto Addr = Memcury::Scanner::FindStringRef(L"%s IpNetDriver listening on port %i");
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true, 1);
}

static inline uint64 FindOnDamageServer()
{
	auto Addr = FindFunctionCall(L"OnDamageServer", { 0x40, 0x55 });
	return Addr;
}

static inline uint64 FindStaticLoadObject()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_LoadObject").ScanFor({ 0x4C, 0x89, 0x4C }, false);
	return Addr.Get();
}

static inline uint64 FindNoMCP()
{
	if (Fortnite_Version == 4)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 83 A7 ? ? ? ? ? 83 E0 01").RelativeOffset(1).Get();

	if (Engine_Version == 421 || Engine_Version == 422)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 84 C0 75 CE").RelativeOffset(1).Get();

	if (Engine_Version == 423)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 84 C0 75 C0").RelativeOffset(1).Get();

	if (Engine_Version == 425)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 84 C0 75 C1").RelativeOffset(1).Get();

	if (Engine_Version == 426)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 84 C0 75 10 84 DB").RelativeOffset(1).Get();

	// return (uintptr_t)GetModuleHandleW(0) + 0x1791CF0; // 11.01
	return 0;
	// return (uintptr_t)GetModuleHandleW(0) + 0x161d600; // 10.40
}

static inline uint64 FindCollectGarbage()
{
	return 0;

	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_CollectGarbageInternal");
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true, 1);
}

static inline uint64 FindActorGetNetMode()
{
	auto AActorGetNetmode = Memcury::Scanner::FindStringRef(L"ClientPlayerLeft %s")
		.ScanFor({ 0x48, 0x8B, 0xCF, 0xE8 })
		.RelativeOffset(4)
		.Get();

	return AActorGetNetmode;
}

static inline uint64 FindTickFlush()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_NetTickFlush");
	return FindBytes(Addr, { 0x4C, 0x8B }, 1000, 0, true);
}

static inline uint64 FindGIsServer()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering");
	auto Addy = FindBytes(Addr, { 0xC6, 0x05 }, 50, 0, true, 1);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x44, 0x88 }, 50, 0, true, 1);

	return Memcury::Scanner(Addy).RelativeOffset(2).Get();
}

static inline uint64 FindChangeGameSessionId()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"Changing GameSessionId from '%s' to '%s'");
	return FindBytes(Addr, { 0x40, 0x55 }, 2000, 0, true);
}

static inline uint64 FindGIsClient()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering");
	auto Addy = FindBytes(Addr, { 0xC6, 0x05 }, 50, 0, true, 2);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x44, 0x88 }, 50, 0, true, 2);

	return Memcury::Scanner(Addy).RelativeOffset(2).Get();
}

static inline uint64 FindGetNetMode()
{
	/* return Memcury::Scanner::FindStringRef(L" (client %d)")
		.ScanFor({ 0x48, 0x8B, 0xC8, 0xE8 }, false)
		.RelativeOffset(4)
		.Get(); // credit ender */

	auto Addr = Memcury::Scanner::FindStringRef(L"PREPHYSBONES");
	auto BeginningFunction = Memcury::Scanner(FindBytes(Addr, { 0x40, 0x55 }, 1000, 0, true));
	auto CallToFunc = Memcury::Scanner(FindBytes(BeginningFunction, { 0xE8 }));

	return CallToFunc.RelativeOffset(1).Get();

	// return (uintptr_t)GetModuleHandleW(0) + 0x34d2140;
}

static inline uint64 FindRealloc()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"a.Budget.BudgetMs", false);

	if (!Addr.Get())
	{
		return Memcury::Scanner::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 48 8B F1 41 8B D8 48 8B 0D ? ? ? ?").Get(); // 4.16-4.20
	}

	auto BeginningFunction = Memcury::Scanner(FindBytes(Addr, { 0x40, 0x53 }, 1000, 0, true));
	auto CallToFunc = Memcury::Scanner(FindBytes(BeginningFunction, { 0xE8 }));

	return CallToFunc.RelativeOffset(1).Get();

	// return Memcury::Scanner::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 48 8B F1 41 8B D8 48 8B 0D ? ? ? ?").Get();
}

static inline uint64 FindPickTeam()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%d]", false);

	if (!Addr.Get())
		Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%s]");

	return FindBytes(Addr, { 0x40, 0x55 }, 1000, 0, true);
}

static inline uint64 FindInternalTryActivateAbility()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"InternalTryActivateAbility called with invalid Handle! ASC: %s. AvatarActor: %s");
	return FindBytes(Addr, { 0x4C, 0x89, 0x4C }, 1000, 0, true);
}

static inline uint64 FindGiveAbility()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!");
	auto /* callToGiveAbility */ realGiveAbility = Memcury::Scanner(FindBytes(Addr, { 0xE8 }, 1000, 0)).RelativeOffset(1).Get();
	// auto realGiveAbility = ((callToGiveAbility + 1 + 4) + *(int*)(callToGiveAbility + 1));
	return realGiveAbility;
}

static inline uint64 FindGiveAbilityAndActivateOnce()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!");

	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 1000, 0, true);
}

static inline uint64 FindCantBuild()
{	
	auto add = Memcury::Scanner::FindPattern("48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 41 56 48 83 EC ? 49 8B E9 4D 8B F0", false).Get();

	if (!add)
		add = Memcury::Scanner::FindPattern("48 89 54 24 ? 55 56 41 56 48 83 EC 50").Get(); // 4.20

	return add;

	auto CreateBuildingActorAddr = Memcury::Scanner(GetFunctionIdxOrPtr(FindObject<UFunction>("/Script/FortniteGame.FortAIController.CreateBuildingActor")));
	auto LikeHuh = Memcury::Scanner(FindBytes(CreateBuildingActorAddr, { 0x40, 0x88 }, 3000));
	auto callaa = Memcury::Scanner(FindBytes(LikeHuh, { 0xE8 }));

	return callaa.RelativeOffset(1).Get();
}

static inline uint64 FindReplaceBuildingActor()
{
	auto StringRef = Memcury::Scanner::FindStringRef(L"STAT_Fort_BuildingSMActorReplaceBuildingActor");

	if (!StringRef.Get()) // we are on a version where stats dont exist
	{
		// todo hardcode sigs

		return 0;
	}

	return FindBytes(StringRef, { 0x4C, 0x8B }, 1000, 0, true);
}