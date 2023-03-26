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

	auto Base = __int64(GetModuleHandleW(0));

	for (int i = 0 + SkipBytes; i < Count + SkipBytes; i++) // we should subtract from skip if goup
	{
		auto CurrentByte = *(Memcury::ASM::MNEMONIC*)(bGoUp ? Scanner.Get() - i : Scanner.Get() + i);

		if (bPrint)
			LOG_INFO(LogFinder, "[{}] CurrentByte: 0x{:x} (0x{:x})", i, (int)CurrentByte, (bGoUp ? Scanner.Get() - i : Scanner.Get() + i) - Base);

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

/* static inline uintptr_t FindBytesArray(Memcury::Scanner& Scanner, const std::vector<std::vector<uint8_t>>& Bytes, int Count = 255, int SkipBytes = 0, bool bGoUp = false, int Skip = 0, const bool bPrint = false)
{
	for (auto& ByteArray : Bytes)
	{
		auto Res = FindBytes(Scanner, ByteArray, Count, SkipBytes, false, Skip, bPrint);

		if (Res)
			return Res;
	}

	return 0;
} */

static inline uint64 FindStaticFindObject(int StringSkip = 1)
{
	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 33 F6 4C 8B E1 45 0F B6 E9 49 8B F8 41 8B C6").Get();

	if (Engine_Version >= 427) // ok so like the func is split up in ida idfk what to do about it
	{
		if (Fortnite_Version < 18)
			return Memcury::Scanner::FindPattern("40 55 53 57 41 54 41 55 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85").Get();
		else
			return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 60 45 33 ED 45 8A F9 44 38 2D ? ? ? ? 49 8B F8 48 8B").Get();
	}

	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("4C 8B DC 57 48 81 EC ? ? ? ? 80 3D ? ? ? ? ? 49 89 6B F0 49 89 73 E8").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"Illegal call to StaticFindObject() while serializing object data!", true, StringSkip, Engine_Version >= 427);
	auto Final = FindBytes(Addr, { 0x48, 0x89, 0x5C }, 255, 0, true, 0, false); // Addr.ScanFor(bytes, false).Get();

	return Final;
}

static inline uint64 FindProcessEvent()
{
	if (Fortnite_Version < 14)
	{
		auto Addr = Memcury::Scanner::FindStringRef(L"AccessNoneNoContext");
		return FindBytes(Addr, { 0x40, 0x55 }, 2000); // Addr.ScanFor({ 0x40, 0x55 }).Get();
	}

	auto Addr = Memcury::Scanner::FindStringRef(L"UMeshNetworkComponent::ProcessEvent: Invalid mesh network node type: %s", true, 0, Engine_Version >= 500);
	return Memcury::Scanner(FindBytes(Addr, { 0xE8 }, 2000, 0, false, Engine_Version < 500 ? 1 : 3)).RelativeOffset(1).Get(); // Addr.ScanFor({ 0x40, 0x55 }).Get();
}

static inline uint64 FindObjectArray()
{
	if (Engine_Version >= 421)
		return Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1").RelativeOffset(3).Get();

	auto cc = Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 14 C8 EB 03 49 8B D6 8B 42 08 C1 E8 1D A8 01 0F 85 ? ? ? ? F7 86 ? ? ? ? ? ? ? ?", false);
	auto addr = cc.Get() ? cc.RelativeOffset(3).Get() : 0; // 4.16
	
	if (!addr)
	{
		if (Engine_Version >= 416 || Engine_Version <= 420)
		{
			auto aa = Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 1C C8 81 4B ? ? ? ? ? 49 63 76 30", false);
			addr = aa.Get() ? aa.RelativeOffset(3).Get() : 0;

			if (!addr)
				addr = Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 1C C8 81 4B ? ? ? ? ? 49 63 76 30", false).RelativeOffset(3).Get();
		}
	}

	return addr;
}

static inline uint64 FindCreateNetDriver()
{
	return 0;
}

static inline uint64 FindKickPlayer()
{
	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("40 53 56 48 81 EC ? ? ? ? 48 8B DA 48 8B F1 E8 ? ? ? ? 48 8B 06 48 8B CE").Get();

	if (std::floor(Fortnite_Version) == 18)
		return Memcury::Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 4C 89 60 20 55 41 56 41 57 48 8B EC 48 83 EC 60 48 83 65 ? ? 4C 8B F2 83 65 E8 00 4C 8B E1 83 65 EC").Get();

	if (Engine_Version >= 423 || Engine_Version <= 425) // && instead of || ??
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
	if (Engine_Version == 427) // idk im dumb
	{
		auto addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B F1 4C 8D 05").Get();
	
		if (!addr) // s18
			addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 56 41 57 48 8D 68 A1 48 81 EC ? ? ? ? 48 8B F1 4C 8D 35 ? ? ? ? 4D").Get();

		return addr;
	}

	auto Addr = Memcury::Scanner::FindStringRef(L"BeaconPort=");
	return FindBytes(Addr, (Engine_Version == 427 ? std::vector<uint8_t>{ 0x48, 0x8B, 0x5C } : std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 }), 1000, 0, true);
}

static inline uint64 FindPauseBeaconRequests()
{
	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 ED 48 8B F1 84 D2 74 27 80 3D").Get();

	if (Engine_Version == 427)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 33 F6 48 8B F9 84 D2 74").Get();

	// todo try 40 53 48 83 EC 30 48 8B ? 84 D2 74 ? 80 3D for S1-S15

	if (Engine_Version == 426)
		return Memcury::Scanner::FindPattern("40 57 48 83 EC 30 48 8B F9 84 D2 74 62 80 3D").Get();

	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 30 48 8B D9 84 D2 74 68 80 3D ? ? ? ? ? 72 2C 48 8B 05 ? ? ? ? 4C 8D 44").Get();

	if (Engine_Version == 419)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 30 48 8B D9 84 D2 74 68 80 3D ? ? ? ? ? 72").Get(); // i supposed this is just because its getitng wrong string ref

	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 30 48 8B D9 84 D2 74 6F 80 3D ? ? ? ? ? 72 33 48 8B 05").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"All Beacon Requests Resumed.");
	return FindBytes(Addr, { 0x40, 0x53 }, 1000, 0, true);
}

static inline uint64 FindOnRep_ZiplineState()
{
	if (Fortnite_Version < 7)
		return 0;

	static auto Addrr = Memcury::Scanner::FindStringRef(L"ZIPLINES!! Role(%s)  AFortPlayerPawn::OnRep_ZiplineState ZiplineState.bIsZiplining=%d", false).Get();

	if (!Addrr)
		Addrr = Memcury::Scanner::FindStringRef(L"ZIPLINES!! GetLocalRole()(%s)  AFortPlayerPawn::OnRep_ZiplineState ZiplineState.bIsZiplining=%d").Get();

	// L"%s LocalRole[%s] ZiplineState.bIsZiplining[%d]" for 18.40???

	if (!Addrr)
		return 0;

	for (int i = 0; i < 400; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x53)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}
	}
	
	return 0;
}

static inline uint64 FindGetMaxTickRate() // Uengine::getmaxtickrate
{
	// TODO switch to index maybe?

	/* auto GetMaxTickRateIndex = *Memcury::Scanner::FindStringRef(L"GETMAXTICKRATE")
        .ScanFor({ 0x4D, 0x8B, 0xC7, 0xE8 })
        .RelativeOffset(4)
        .ScanFor({ 0xFF, 0x90 })
        .AbsoluteOffset(2)
        .GetAs<int*>() / 8;

    LOG_INFO(LogHook, "GetMaxTickRateIndex {}", GetMaxTickRateIndex); */

	auto stringRef = Memcury::Scanner::FindStringRef(L"Hitching by request!");

	if (!stringRef.Get())
		return 0;

	return FindBytes(stringRef, Fortnite_Version <= 2.5 ? std::vector<uint8_t>{ 0x40, 0x53 } : std::vector<uint8_t>{ 0x48, 0x89, 0x5C }, 1000, 0, true);
}

static inline uint64 FindGetPlayerViewpoint()
{
	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 41 56 41 57 48 8B EC 48 83 EC 50").Get(); // idk why finder doesnt work and cba to debug

	auto Addrr = Memcury::Scanner::FindStringRef(L"APlayerController::GetPlayerViewPoint: out_Location, ViewTarget=%s").Get();

	LOG_INFO(LogDev, "GetPlayerViewpoint StringRef: 0x{:x}", __int64(Addrr) - __int64(GetModuleHandleW(0)));

	for (int i = 0; i < 1500; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x55)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
		{
			return Addrr - i;
		}
	}

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x74)
		{
			return Addrr - i;
		}
	}

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}
	}

	return 0;
}

static inline uint64 FindSpawnActor()
{

	if (Engine_Version >= 427)
	{
		auto stat = Memcury::Scanner::FindStringRef(L"STAT_SpawnActorTime");
		return FindBytes(stat, { 0x48, 0x8B, 0xC4 }, 3000, 0, true);
	}

	auto Addr = Memcury::Scanner::FindStringRef(L"SpawnActor failed because no class was specified");

	if (Engine_Version >= 416 && Fortnite_Version <= 2.5)
		return FindBytes(Addr, { 0x40, 0x55 }, 3000, 0, true);

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
	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("4C 8B DC 49 89 5B 10 49 89 73 18 57 48 83 EC 50 48 8B BC 24 ?? ?? ?? ?? 49 8B F0 48 8B").Get();

	if (Engine_Version >= 427)
		return Memcury::Scanner::FindPattern("4C 8B DC 49 89 5B 08 49 89 73 10 57 48 83 EC 50 48 8B BC 24 ? ? ? ? 49 8B F0 48 8B 01 48 8B").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"%s IpNetDriver listening on port %i");
	return FindBytes(Addr, Engine_Version < 427 ? std::vector<uint8_t>{ 0x48, 0x89, 0x5C } : std::vector<uint8_t>{ 0x4C, 0x8B, 0xDC }, 2000, 0, true, 1);
}

static inline uint64 FindOnDamageServer()
{
	auto Addr = FindFunctionCall(L"OnDamageServer", { 0x40, 0x55 });
	return Addr;
}

static inline uint64 FindStaticLoadObject()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"STAT_LoadObject", false).Get();

	if (!Addrr)
	{
		auto StrRef2 = Memcury::Scanner::FindStringRef(L"Calling StaticLoadObject during PostLoad may result in hitches during streaming.");
		return FindBytes(StrRef2, { 0x40, 0x55 }, 1000, 0, true);
	}

	for (int i = 0; i < 400; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x4C && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x4C)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
		{
			return Addrr - i;
		}
	}

	return 0;
}

static inline uint64 FindSpecConstructor()
{
	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("80 61 29 F8 48 8B 44 24 ?").Get(); // 3.5

	if (Engine_Version == 421)
		return Memcury::Scanner::FindPattern("80 61 29 F8 48 8B 44 24 ?").Get(); // 6.21

	if (Engine_Version == 422)
		return Memcury::Scanner::FindPattern("80 61 29 F8 48 8B 44 24 ?").Get(); // was a guess

	if (Engine_Version == 423)
		return Memcury::Scanner::FindPattern("80 61 29 F8 48 8B 44 24 ?").Get(); // was a guess

	if (Engine_Version == 424)
		return Memcury::Scanner::FindPattern("80 61 29 F8 48 8B 44 24 ?").Get(); // 11.31

	if (Engine_Version == 425)
	{
		auto ba = Memcury::Scanner::FindPattern("48 8B 44 24 ? 80 61 29 F8 80 61 31 FE 48 89 41 20 33 C0 89 41", false).Get();
	
		if (!ba)
			ba = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 45 33 F6 48 C7 01 ? ? ? ? 48 C7 41").Get(); // i think this right for 12.00 ??

		return ba;
	}

	if (Engine_Version == 426)
		return Memcury::Scanner::FindPattern("80 61 31 FE 0F 57 C0 80 61 29 F0 48 8B 44 24 ? 48").Get();

	if (Engine_Version == 427)
		return Memcury::Scanner::FindPattern("80 61 31 FE 41 83 C9 FF 80 61 29 F0 48 8B 44 24 ? 48 89 41").Get();

	// if (Engine_Version == 500)
		// return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 83 CD FF").Get();

	return 0;
}

static inline uint64 FindCompletePickupAnimation()
{
	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B D9 48 8B 89 ? ? ? ? 48 85 C9 74 20 48 8D 44 24").Get();

	if (Engine_Version == 421)
		return Memcury::Scanner::FindPattern("40 53 56 57 48 83 EC 30 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED 0F 84").Get(); // 6.21

	if (Engine_Version == 422)
		return Memcury::Scanner::FindPattern("40 53 56 57 48 83 EC 30 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED 0F 84").Get(); // 7.30

	if (Engine_Version >= 423 && Engine_Version <= 426)
		return Memcury::Scanner::FindPattern("40 53 56 48 83 EC 38 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED").Get(); // 10.40

	if (Engine_Version == 427)
		return Memcury::Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 54 41 56 41 57 48 83 EC 20 48 8B B1 ? ? ? ? 48 8B D9 48 85 F6").Get(); // 17.30

	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B B9").Get(); // 19.10

	return 0;
}

static inline uint64 FindNoMCP()
{
	/* if (Fortnite_Version >= 17) // idk if needed
	{
		// todo make this relative
		// 19.10
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC 20 65 48 8B 04 25 ? ? ? ? BA ? ? ? ? 48 8B 08 8B 04 0A 39 05 ? ? ? ? 7F 23 8A 05 ? ? ? ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 8B 74 24 ? 48 83 C4 20 41 5F 41 5E 41 5D 41 5C 5F C3 48 8D 0D ? ? ? ? E8 ? ? ? ? 83 3D ? ? ? ? ? 75 C8 E8 ? ? ? ? 45 33").Get();
	} */

	if (std::floor(Fortnite_Version) == 3)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 83 A7 ? ? ? ? ? 48 8D 4C 24 ?").RelativeOffset(1).Get();

	if (std::floor(Fortnite_Version) == 4)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 83 A7 ? ? ? ? ? 83 E0 01").RelativeOffset(1).Get();

	auto fn = FindObject<UFunction>("/Script/FortniteGame.FortKismetLibrary.IsRunningNoMCP");
	LOG_INFO(LogDev, "fn: {}", __int64(fn));

	if (!fn)
		return 0;

	auto scanner = Memcury::Scanner(__int64(fn->GetFunc()));
	auto noMcpIthink = Memcury::Scanner(FindBytes(scanner, { 0xE8 })).RelativeOffset(1).Get(); // GetFunctionIdxOrPtr(fn);
	return noMcpIthink;

	if (Engine_Version == 421 || Engine_Version == 422)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 84 C0 75 CE").RelativeOffset(1).Get();

	if (Engine_Version == 423)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 84 C0 75 C0").RelativeOffset(1).Get();

	if (Engine_Version == 425)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 84 C0 75 C1").RelativeOffset(1).Get();

	if (Engine_Version == 426)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 84 C0 75 10 84 DB").RelativeOffset(1).Get();

	if (Engine_Version == 427)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 84 C0 74 F0").RelativeOffset(1).Get();

	// return (uintptr_t)GetModuleHandleW(0) + 0x1791CF0; // 11.01
	return 0;
	// return (uintptr_t)GetModuleHandleW(0) + 0x161d600; // 10.40
}

static inline uint64 FindSetZoneToIndex() // actually StartNewSafeZonePhase
{
	// return 0;

	// if (Fortnite_Version == 14.60)
		// return __int64(GetModuleHandleW(0)) + 0x207F9B0;

	// return Memcury::Scanner::FindPattern("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 C8 0F 29 78 B8 44 0F 29 40 ? 44 0F 29 48 ? 44 0F 29 50 ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 44 8B F2 89 54 24 48 4C 8B F9 48 89 4D 90 E8 ? ? ? ? 45 33 ED 48 89 45 A0 48 8B F0").Get(); // 19.10
	// return Memcury::Scanner::FindPattern("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D 68 98 48 81 EC ? ? ? ? 0F 29 70 C8 0F 29 78 B8 44 0F 29 40 ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 08 44 8B F2 89 54 24 48 48 8B F1 48 89 4C 24 ? E8 ? ? ? ? 45 33 E4 48 89 44 24 ? 4C 8B F8 48 85 C0 74 09").Get(); // 17.30
	// return Memcury::Scanner::FindPattern("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D 68 88 48 81 EC ? ? ? ? 0F 29 70 C8 0F 29 78 B8 44 0F 29 40 ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 10 44 8B F2 89 54 24 48 48 8B F1 48 89 4C 24 ? E8 ? ? ? ? 45 33 E4 48 89 45 80 4C 8B F8 48 85 C0 74 09 48 8B B8").Get(); // 17.50
	// return Memcury::Scanner::FindPattern("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 C8 0F 29 78 B8 44 0F 29 40 ? 44 0F 29 48 ? 44 0F 29 50 ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8B B1 ? ? ? ? 45 33 ED 89 54 24 70 44 8B FA 48 89 4C 24").Get(); // 18.40
	// return Memcury::Scanner::FindPattern("40 55 53 56 41 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 18 48 8B").Get(); // 14.60

	auto Addr = Memcury::Scanner::FindStringRef(L"FortGameModeAthena: No MegaStorm on SafeZone[%d].  GridCellThickness is less than 1.0.", true, 0, Fortnite_Version >= 17).Get();
	// return FindBytes(Addr, { 0x40, 0x55 }, 30000, 0, true);

	if (!Addr)
		return 0;

	for (int i = 0; i < 100000; i++)
	{
		if ((*(uint8_t*)(uint8_t*)(Addr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x53) || (*(uint8_t*)(uint8_t*)(Addr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x55))
		{
			return Addr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addr - i + 2) == 0xC4)
		{
			return Addr - i;
		}
	}

	return 0;
}

static inline uint64 FindCollectGarbage()
{
	// return 0;

	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_CollectGarbageInternal");
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true, 1);
}

static inline uint64 FindActorGetNetMode()
{
	// return 0;

	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 F6 41 08 10 48 8B D9 0F 85 ? ? ? ? 48 8B 41 20 48 85 C0 0F 84 ? ? ? ? F7 40").Get();

	if (Engine_Version == 427)
	{
		// note this sig doesnt work on s18
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 93 ? ? ? ? 48 8B C8 48 8B F8 E8 ? ? ? ? 48 85 C0 75 29").Get();
	}

	auto AActorGetNetmodeStrRef = Memcury::Scanner::FindStringRef(L"STAT_ServerUpdateCamera", false);

	if (!AActorGetNetmodeStrRef.Get())
	{
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B 01 48 8B D9 FF 90 ? ? ? ? 4C 8B").Get(); // 2.5 i think
	}

	return Memcury::Scanner(FindBytes(AActorGetNetmodeStrRef, { 0xE8 }, 255, 0, true)).RelativeOffset(1).Get();
}

static inline uint64 FindRemoveFromAlivePlayers()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"FortGameModeAthena: Player [%s] removed from alive players list (Team [%d]).  Player count is now [%d].  Team count is now [%d].", false).Get();

	if (!Addrr)
		Addrr = Memcury::Scanner::FindStringRef(L"FortGameModeAthena: Player [%s] removed from alive players list (Team [%d]).  Player count is now [%d]. PlayerBots count is now [%d]. Team count is now [%d].", false).Get();

	if (!Addrr)
		Addrr = Memcury::Scanner::FindStringRef(L"FortGameModeAthena::RemoveFromAlivePlayers: Player [%s] PC [%s] removed from alive players list (Team [%d]).  Player count is now [%d]. PlayerBots count is now [%d]. Team count is now [%d].", true, 0, Fortnite_Version >= 18).Get(); // todo check version

	for (int i = 0; i < 2000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x4C && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x4C)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x54)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
		{
			return Addrr - i;
		}
	}

	return 0;
}

static inline uint64 FindTickFlush()
{
	// auto add = Memcury::Scanner::FindStringRef(L"UDemoNetDriver::TickFlush: ReplayStreamer ERROR: %s");
	// return Memcury::Scanner(FindBytes(add, { 0xE8 }, 500, 0, true, 1)).RelativeOffset(1).Get();

	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("4C 8B DC 55 53 56 57 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 41 0F 29 7B").Get(); // 2.4.2

	if (Engine_Version == 419)
		return Memcury::Scanner::FindPattern("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 45 0F 29 43 ? 45 0F 29 4B ? 48 8B 05 ? ? ? ? 48").Get(); // 2.4.2

	if (Engine_Version == 427)
	{
		auto addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 8A").Get();

		if (!addr) // s18
			addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 44 0F").Get();
	
		return addr;
	}

	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_NetTickFlush");

	if (!Addr.Get())
	{
		if (Engine_Version == 420) // 2.5
		{
			return Memcury::Scanner::FindPattern("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 45 0F 29 43 ? 45 0F 29 4B ? 48 8B 05 ? ? ? ? 48 33").Get();
		}
	}

	return FindBytes(Addr, (Fortnite_Version < 18 ? std::vector<uint8_t>{ 0x4C, 0x8B } : std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 }), 1000, 0, true);
}

static inline uint64 FindAddNavigationSystemToWorld()
{
	return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 ED 41", false).Get();
}

static inline uint64 FindNavSystemCleanUp()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"UNavigationSystemV1::CleanUp", false);

	if (!Addr.Get())
		return 0;

	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 500, 0, true);
}

static inline uint64 FindLoadPlayset(const std::vector<uint8_t>& Bytes = std::vector<uint8_t>({ 0x48, 0x89, 0x5C }), int recursive = 0)
{
	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 4C 8B B1 ? ? ? ? 45").Get();

	if (recursive >= 2)
		return 0;

	auto StringRef = Memcury::Scanner::FindStringRef(L"UPlaysetLevelStreamComponent::LoadPlayset Error: no owner for %s", Fortnite_Version >= 7, 1);

	if (!StringRef.Get())
		return 0;

	for (int i = 0 + 0; i < 400 + 0; i++) // we should subtract from skip if goup
	{
		auto CurrentByte = *(Memcury::ASM::MNEMONIC*)(true ? StringRef.Get() - i : StringRef.Get() + i);

		if (CurrentByte == Bytes[0])
		{
			bool Found = true;
			for (int j = 1; j < Bytes.size(); j++)
			{
				if (*(Memcury::ASM::MNEMONIC*)(true ? StringRef.Get() - i + j : StringRef.Get() + i + j) != Bytes[j])
				{
					Found = false;
					break;
				}
			}
			if (Found)
			{
				return true ? StringRef.Get() - i : StringRef.Get() + i;
			}
		}

		if (CurrentByte == 0xC3)
			return FindLoadPlayset({ 0x40, 0x55 }, ++recursive);

		// std::cout << std::format("CurrentByte: 0x{:x}\n", (uint8_t)CurrentByte);
	}

	return 0;
}

static inline uint64 FindGIsServer()
{
	// auto add = Memcury::Scanner::FindStringRef(L"STAT_UpdateLevelStreaming");
	// return Memcury::Scanner(FindBytes(add, { 0x80, 0x3D }, 100, 0, true, 1)).RelativeOffset(2).Get();

	// if (Fortnite_Version == 19.10)
		// return __int64(GetModuleHandleW(0)) + 0xB30CF9D;

	// if (Fortnite_Version == 2.5)
		// return __int64(GetModuleHandleW(0)) + 0x46AD735;
	if (Fortnite_Version == 12.41)
		return __int64(GetModuleHandleW(0)) + 0x804B65A;

	if (Fortnite_Version == 14.60)
		return __int64(GetModuleHandleW(0)) + 0x939930E;

	if (Fortnite_Version == 17.30)
		return __int64(GetModuleHandleW(0)) + 0x973E499;

	return 0;

	auto Addr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering");

	std::vector<std::vector<uint8_t>> BytesArray = { { 0xC6, 0x05 }, { 0x88, 0x1D } };

	int Skip = 1;

	uint64 Addy;

	for (int i = 0; i < 50; i++) // we should subtract from skip if goup
	{
		auto CurrentByte = *(Memcury::ASM::MNEMONIC*)(Addr.Get() - i);

		// if (bPrint)
			// std::cout << "CurrentByte: " << std::hex << (int)CurrentByte << '\n';

		bool ShouldBreak = false;

		for (auto& Bytes : BytesArray)
		{
			if (CurrentByte == Bytes[0])
			{
				bool Found = true;
				for (int j = 1; j < Bytes.size(); j++)
				{
					if (*(Memcury::ASM::MNEMONIC*)(Addr.Get() - i + j) != Bytes[j])
					{
						Found = false;
						break;
					}
				}
				if (Found)
				{
					LOG_INFO(LogDev, "[{}] Skip: 0x{:x}", Skip, Memcury::Scanner(Addr.Get() - i).RelativeOffset(2).Get() - __int64(GetModuleHandleW(0)));

					if (Skip > 0)
					{
						Skip--;
						continue;
					}

					Addy = Addr.Get() - i;
					ShouldBreak = true;
					break;
				}
			}
		}

		if (ShouldBreak)
			break;

		// std::cout << std::format("CurrentByte: 0x{:x}\n", (uint8_t)CurrentByte);
	}

	/* int Skip = 2;
	auto Addy = FindBytes(Addr, { 0xC6, 0x05 }, 50, 0, true, Skip);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x44, 0x88 }, 50, 0, true, Skip);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x88, 0x1D }, 50, 0, true, Skip); */

	LOG_INFO(LogDev, "Addy: 0x{:x}", Addy - __int64(GetModuleHandleW(0)));

	return Memcury::Scanner(Addy).RelativeOffset(2).Get();
}

static inline uint64 FindChangeGameSessionId()
{
	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 50 4C 8B FA 48 8B F1 E8").Get();

	if (Engine_Version >= 427)
	{
		if (Fortnite_Version < 18)
			return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 70 4C 8B FA 4C").Get();
		else
			return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 70 4C 8B FA 4C").Get();
	}

	if (Fortnite_Version == 2.5)
		return Memcury::Scanner::FindPattern("40 55 56 41 56 48 8B EC 48 81 EC ? ? ? ? 48 8B 01 4C 8B F2").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"Changing GameSessionId from '%s' to '%s'");
	return FindBytes(Addr, { 0x40, 0x55 }, 2000, 0, true);
}

static inline uint64 FindDispatchRequest()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"MCP-Profile: Dispatching request to %s", true, 0, Fortnite_Version >= 19);
	return FindBytes(Addr, std::floor(Fortnite_Version) == 18 ? std::vector<uint8_t>{0x48, 0x8B, 0xC4 } : std::vector<uint8_t>{ 0x48, 0x89, 0x5C }, 300, 0, true);
}

static inline uint64 FindGIsClient()
{
	// if (Fortnite_Version == 2.5)
		// return __int64(GetModuleHandleW(0)) + 0x46AD734;
	if (Fortnite_Version == 4.1)
		return __int64(GetModuleHandleW(0)) + 0x4BF6F17;

	if (Fortnite_Version == 11.31)
		return __int64(GetModuleHandleW(0)) + 0x6F41270;

	if (Fortnite_Version == 12.41)
		return __int64(GetModuleHandleW(0)) + 0x804B659;

	if (Fortnite_Version == 14.60)
		return __int64(GetModuleHandleW(0)) + 0x939930D;

	if (Fortnite_Version == 17.30)
		return __int64(GetModuleHandleW(0)) + 0x973E49B;

	return 0;

	auto Addr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering");

	std::vector<std::vector<uint8_t>> BytesArray = { { 0xC6, 0x05 }, { 0x88, 0x1D } };

	int Skip = 2;

	uint64 Addy;

	for (int i = 0; i < 50; i++) // we should subtract from skip if goup
	{
		auto CurrentByte = *(Memcury::ASM::MNEMONIC*)(Addr.Get() - i);

		// if (bPrint)
			// std::cout << "CurrentByte: " << std::hex << (int)CurrentByte << '\n';

		bool ShouldBreak = false;

		for (auto& Bytes : BytesArray)
		{
			if (CurrentByte == Bytes[0])
			{
				bool Found = true;
				for (int j = 1; j < Bytes.size(); j++)
				{
					if (*(Memcury::ASM::MNEMONIC*)(Addr.Get() - i + j) != Bytes[j])
					{
						Found = false;
						break;
					}
				}
				if (Found)
				{
					LOG_INFO(LogDev, "[{}] Skip: 0x{:x}", Skip, Memcury::Scanner(Addr.Get() - i).RelativeOffset(2).Get() - __int64(GetModuleHandleW(0)));

					if (Skip > 0)
					{
						Skip--;
						continue;
					}

					Addy = Addr.Get() - i;
					ShouldBreak = true;
					break;
				}
			}
		}

		if (ShouldBreak)
			break;

		// std::cout << std::format("CurrentByte: 0x{:x}\n", (uint8_t)CurrentByte);
	}

	/* int Skip = 2;
	auto Addy = FindBytes(Addr, { 0xC6, 0x05 }, 50, 0, true, Skip);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x44, 0x88 }, 50, 0, true, Skip);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x88, 0x1D }, 50, 0, true, Skip); */

	LOG_INFO(LogDev, "Addy: 0x{:x}", Addy - __int64(GetModuleHandleW(0)));

	return Memcury::Scanner(Addy).RelativeOffset(2).Get();

	/*
	auto Addr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering");
	int Skip = 1;
	auto Addy = FindBytes(Addr, { 0xC6, 0x05 }, 50, 0, true, Skip);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x44, 0x88 }, 50, 0, true, Skip);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x88, 0x1D }, 50, 0, true, Skip);

	return Memcury::Scanner(Addy).RelativeOffset(2).Get();
	*/
}

static inline uint64 FindGetNetMode()
{
	/* return Memcury::Scanner::FindStringRef(L" (client %d)")
		.ScanFor({ 0x48, 0x8B, 0xC8, 0xE8 }, false)
		.RelativeOffset(4)
		.Get(); // credit ender */

	if (std::floor(Fortnite_Version) == 18)
		return Memcury::Scanner::FindPattern("48 83 EC 28 48 83 79 ? ? 75 20 48 8B 91 ? ? ? ? 48 85 D2 74 1E 48 8B 02 48 8B CA FF 90").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"PREPHYSBONES");
	auto BeginningFunction = Memcury::Scanner(FindBytes(Addr, { 0x40, 0x55 }, 1000, 0, true));
	auto CallToFunc = Memcury::Scanner(FindBytes(BeginningFunction, { 0xE8 }));

	return CallToFunc.RelativeOffset(1).Get();

	// return (uintptr_t)GetModuleHandleW(0) + 0x34d2140;
}

static inline uint64 FindRealloc()
{
	if (Engine_Version >= 427)
		return Memcury::Scanner::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 48 8B F1 41 8B D8 48 8B 0D ? ? ? ").Get();

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
	if (Engine_Version >= 427) // different start
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 88 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 70 4C 8B A1").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%d]", false);

	if (!Addr.Get())
		Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%s]");

	return FindBytes(Addr, Fortnite_Version <= 2.5 ? std::vector<uint8_t>{ 0x48, 0x89, 0x6C } : std::vector<uint8_t>{ 0x40, 0x55 }, 1000, 0, true);
}

static inline uint64 FindInternalTryActivateAbility()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"InternalTryActivateAbility called with invalid Handle! ASC: %s. AvatarActor: %s", true, 0, Fortnite_Version >= 17).Get();

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x4C && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x4C)
		{
			return Addrr - i;
		}
	}
	
	return 0;
	// return FindBytes(Addr, { 0x4C, 0x89, 0x4C }, 1000, 0, true);
}

static inline uint64 FindFrameStep()
{
	return Memcury::Scanner::FindPattern("48 8B 41 20 4C 8B D2 48 8B D1 44 0F B6 08 48 FF").Get();
}

static inline uint64 FindCanActivateAbility()
{
	// return 0;

	if (Engine_Version <= 420)
		return 0; // ?

	// this doesn't work on like >2.5

	if (Engine_Version == 421 || Engine_Version == 422)
		return Memcury::Scanner::FindPattern("4C 89 4C 24 20 55 56 57 41 56 48 8D 6C 24 D1").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"CanActivateAbility %s failed, blueprint refused", true, 0, Engine_Version >= 500);
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true);
}

static inline uint64 FindGiveAbilityAndActivateOnce()
{
	if (Engine_Version == 426)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 49 8B 40 10 49 8B D8 48 8B FA 48 8B F1").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!", true, 0, Engine_Version >= 500);
	auto res = FindBytes(Addr, { 0x48, 0x89, 0x5C }, 1000, 0, true);

	return res;
}

static inline uint64 FindGiveAbility()
{
	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 56 57 41 56 48 83 EC 20 83 B9").Get();
	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 56 57 41 56 48 83 EC 20 83 B9 ? ? ? ? ? 49 8B F0 4C 8B F2 48 8B D9 7E 61").Get();
	if (Engine_Version == 421)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 83 B9 ? ? ? ? ? 49 8B E8 4C 8B F2").Get();
	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC 20 8B 81 ? ? ? ? 49 8B E8 4C").Get(); // idk why finder doesnt work

	// auto Addr = Memcury::Scanner::FindStringRef(L"GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!"); // has 2 refs for some reason on some versions
	// auto realGiveAbility = Memcury::Scanner(FindBytes(Addr, { 0xE8 }, 500, 0, false, 0, true)).RelativeOffset(1).Get();

	Memcury::Scanner addr = Memcury::Scanner::FindStringRef(L"GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!", true, 1, Engine_Version >= 500); // Memcury::Scanner(FindGiveAbilityAndActivateOnce());

	// LOG_INFO(LogDev, "aaaaa: 0x{:x}", addr.Get() - __int64(GetModuleHandleW(0)));

	return Memcury::Scanner(FindBytes(addr, { 0xE8 }, 500, 0, false)).RelativeOffset(1).Get();
}

static inline uint64 FindCantBuild()
{	
	auto add = Memcury::Scanner::FindPattern("48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 41 56 48 83 EC ? 49 8B E9 4D 8B F0", false).Get();

	if (!add)
		add = Memcury::Scanner::FindPattern("48 89 54 24 ? 55 56 41 56 48 83 EC 50", false).Get(); // 4.20

	if (!add)
		add = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 60 4D 8B F1 4D 8B F8", false).Get(); // 4.26.1

	if (!add)
		add = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 60 49 8B E9 4D 8B F8 48 8B DA 48 8B F9 BE ? ? ? ? 48").Get(); // 5.00

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
		return Memcury::Scanner::FindPattern("4C 89 44 24 ? 55 56 57 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 45").Get(); // 1.7.2 & 2.4.2
	}

	return FindBytes(StringRef, 
		(Engine_Version == 420 || Engine_Version == 421 || Engine_Version == 500 ? std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 } : std::vector<uint8_t>{ 0x4C, 0x8B }),
		1000, 0, true);
}

static inline uint64 FindSendClientAdjustment()
{
	if (Fortnite_Version <= 2.5)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 20 48 8B 99 ? ? ? ? 48 39 99 ? ? ? ? 74 0A 48 83 B9").Get();

	return 0;
}

static inline uint64 FindReplicateActor()
{
	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("40 55 53 57 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8D 59 68 4C 8B F1 48 8B").Get();
	if (Engine_Version >= 419 && Fortnite_Version <= 2.5)
		return Memcury::Scanner::FindPattern("40 55 56 41 54 41 55 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 4C 8B E9 48 8B 49 68 48").Get();

	return 0;
}

static inline uint64 FindCreateChannel()
{
	if (Fortnite_Version <= 2.5)
		return Memcury::Scanner::FindPattern("40 56 57 41 54 41 55 41 57 48 83 EC 60 48 8B 01 41 8B F9 45 0F B6 E0").Get();

	return 0;
}

static inline uint64 FindSetChannelActor()
{
	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("4C 8B DC 55 53 57 41 54 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 45 33").Get();
	if (Engine_Version >= 419 && Fortnite_Version <= 2.5)
		return Memcury::Scanner::FindPattern("48 8B C4 55 53 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 89 70 E8 48 8B D9").Get();

	return 0;
}

static inline uint64 FindCallPreReplication()
{
	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("48 85 D2 0F 84 ? ? ? ? 48 8B C4 55 57 41 57 48 8D 68 A1 48 81 EC").Get();
	if (Engine_Version == 419)
		return Memcury::Scanner::FindPattern("48 85 D2 0F 84 ? ? ? ? 48 8B C4 55 57 41 54 48 8D 68 A1 48 81 EC ? ? ? ? 48 89 58 08 4C").Get();
	if (Fortnite_Version == 2.5)
		return Memcury::Scanner::FindPattern("48 85 D2 0F 84 ? ? ? ? 56 41 56 48 83 EC 38 4C 8B F2").Get();

	return 0;
}