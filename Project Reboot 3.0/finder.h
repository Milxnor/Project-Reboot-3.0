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
	// return Memcury::Scanner::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 49 8B F0 48 8B DA 48 85 D2").Get(); // 12.41

	auto Addr = Memcury::Scanner::FindStringRef(L"Validation Failure: %s. kicking %s", false);

	if (Addr.Get())
	{
		return FindBytes(Addr, { 0x40, 0x55 }, 2000, 0, true);
	}

	auto Addr2 = Memcury::Scanner::FindStringRef(L"KickPlayer %s Reason %s");
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true);
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
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true);
}

static inline uint64 FindSpawnActor()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"SpawnActor failed because no class was specified");
	return FindBytes(Addr, { 0x4C, 0x8B, 0xDC }, 3000, 0, true);
}

static inline uint64 FindSetWorld()
{
	return Memcury::Scanner::FindStringRef(L"AOnlineBeaconHost::InitHost failed")
		.ScanFor({ 0x48, 0x8B, 0xD0, 0xE8 }, false)
		.RelativeOffset(4)
		.Get(); // THANKS ENDER

	// return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8B EC 48 83 EC 30 48 8B 99").Get(); // s12 i think
	return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 99 ? ? ? ? 48 8B F2 48 8B F9 48 85 DB 0F 84 ? ? ? ? 48 8B 97").Get();
}

static inline uint64 FindInitListen() 
{
	auto Addr = Memcury::Scanner::FindStringRef(L"%s IpNetDriver listening on port %i");
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true, 1);
}

static inline uint64 FindNoMCP()
{
	// return (uintptr_t)GetModuleHandleW(0) + 0x1791CF0; // 11.01
	return 0;
	return (uintptr_t)GetModuleHandleW(0) + 0x161d600;
}

static inline uint64 FindCollectGarbage()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_CollectGarbageInternal");
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true, 1);
}

static inline uint64 FindTickFlush()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_NetTickFlush");
	return FindBytes(Addr, { 0x4C, 0x8B }, 1000, 0, true);
}

static inline uint64 FindGetNetMode()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"PREPHYSBONES");
	auto BeginningFunction = Memcury::Scanner(FindBytes(Addr, { 0x40, 0x55 }, 1000, 0, true));
	auto CallToFunc = Memcury::Scanner(FindBytes(BeginningFunction, { 0xE8 }));

	return CallToFunc.RelativeOffset(1).Get();

	// return (uintptr_t)GetModuleHandleW(0) + 0x34d2140;
}

static inline uint64 FindRealloc()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"a.Budget.BudgetMs");
	auto BeginningFunction = Memcury::Scanner(FindBytes(Addr, { 0x40, 0x53 }, 1000, 0, true));
	auto CallToFunc = Memcury::Scanner(FindBytes(BeginningFunction, { 0xE8 }));

	return CallToFunc.RelativeOffset(1).Get();

	// return Memcury::Scanner::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 48 8B F1 41 8B D8 48 8B 0D ? ? ? ?").Get();
}

static inline uint64 FindPickTeam()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%d]");
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

static inline uint64 FindCantBuild()
{
	return Memcury::Scanner::FindPattern("48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 41 56 48 83 EC ? 49 8B E9 4D 8B F0").Get();

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