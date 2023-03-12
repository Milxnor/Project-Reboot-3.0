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

		if (bPrint)
			LOG_INFO(LogFinder, "[{}] CurrentByte: 0x{:x}", i, (int)CurrentByte);

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

static inline uint64 FindStaticFindObject()
{
	if (Engine_Version == 500)
	{
		return Memcury::Scanner::FindPattern("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 33 F6 4C 8B E1 45 0F B6 E9 49 8B F8 41 8B C6").Get();
	}

	if (Engine_Version >= 427) // ok so like the func is split up in ida idfk what to do about it
	{
		if (Fortnite_Version < 18)
			return Memcury::Scanner::FindPattern("40 55 53 57 41 54 41 55 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85").Get();
		else
			return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 60 45 33 ED 45 8A F9 44 38 2D ? ? ? ? 49 8B F8 48 8B").Get();
	}

	auto Addr = Memcury::Scanner::FindStringRef(L"Illegal call to StaticFindObject() while serializing object data!", true, 0, Engine_Version >= 427);
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 255, 0, true); // Addr.ScanFor(bytes, false).Get();
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
	if (Engine_Version == 427) // idk im dumb
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B F1 4C 8D 05").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"BeaconPort=");
	return FindBytes(Addr, (Engine_Version == 427 ? std::vector<uint8_t>{ 0x48, 0x8B, 0x5C } : std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 }), 1000, 0, true);
}

static inline uint64 FindPauseBeaconRequests()
{
	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 ED 48 8B F1 84 D2 74 27 80 3D").Get();

	if (Engine_Version == 427)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 33 F6 48 8B F9 84 D2 74").Get();

	if (Engine_Version == 426)
		return Memcury::Scanner::FindPattern("40 57 48 83 EC 30 48 8B F9 84 D2 74 62 80 3D").Get();

	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 30 48 8B D9 84 D2 74 68 80 3D ? ? ? ? ? 72 2C 48 8B 05 ? ? ? ? 4C 8D 44").Get();

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
	if (Engine_Version >= 427)
	{
		auto stat = Memcury::Scanner::FindStringRef(L"STAT_SpawnActorTime");
		return FindBytes(stat, { 0x48, 0x8B, 0xC4 }, 3000, 0, true);
	}

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
	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_LoadObject").ScanFor({ 0x4C, 0x89, 0x4C }, false);
	return Addr.Get();
}

static inline uint64 FindCompletePickupAnimation()
{
	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B D9 48 8B 89 ? ? ? ? 48 85 C9 74 20 48 8D 44 24").Get();

	if (Engine_Version == 421)
		return Memcury::Scanner::FindPattern("40 53 56 57 48 83 EC 30 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED 0F 84").Get(); // 6.21

	if (Engine_Version == 422)
		return Memcury::Scanner::FindPattern("40 53 56 57 48 83 EC 30 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED 0F 84").Get(); // 7.30

	if (Engine_Version == 423)
		return Memcury::Scanner::FindPattern("40 53 56 48 83 EC 38 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED 0F 84 ? ? ? ? 49 63 8D").Get(); // 10.40

	if (Engine_Version == 424)
		return Memcury::Scanner::FindPattern("40 53 56 48 83 EC 38 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED 0F 84 ? ? ? ? 49 63 8D").Get(); // 11.31

	if (Engine_Version == 425)
		return Memcury::Scanner::FindPattern("40 53 56 48 83 EC 38 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED 0F 84 ? ? ? ? 49 63 8D").Get(); // 12.41

	if (Engine_Version == 426)
		return Memcury::Scanner::FindPattern("40 53 56 48 83 EC 38 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED").Get(); // 14.60

	return 0;
}

static inline uint64 FindNoMCP()
{
	if (std::floor(Fortnite_Version) == 3)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 83 A7 ? ? ? ? ? 48 8D 4C 24 ?").Get();

	if (std::floor(Fortnite_Version) == 4)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 83 A7 ? ? ? ? ? 83 E0 01").RelativeOffset(1).Get();

	auto fn = FindObject<UFunction>("/Script/FortniteGame.FortKismetLibrary.IsRunningNoMCP");
	LOG_INFO(LogDev, "fn: {}", __int64(fn));

	if (!fn)
		return 0;

	auto noMcpIthink = GetFunctionIdxOrPtr(fn);
	return noMcpIthink;

	if (Fortnite_Version >= 17)
	{
		// todo make this relative
		// 19.10
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC 20 65 48 8B 04 25 ? ? ? ? BA ? ? ? ? 48 8B 08 8B 04 0A 39 05 ? ? ? ? 7F 23 8A 05 ? ? ? ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 8B 74 24 ? 48 83 C4 20 41 5F 41 5E 41 5D 41 5C 5F C3 48 8D 0D ? ? ? ? E8 ? ? ? ? 83 3D ? ? ? ? ? 75 C8 E8 ? ? ? ? 45 33").Get();
	}

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

static inline uint64 FindSetZoneToIndex()
{
	if (Fortnite_Version == 14.60)
		return __int64(GetModuleHandleW(0)) + 0x207F9B0;

	return 0;

	auto Addr = Memcury::Scanner::FindStringRef(L"FortGameModeAthena: No MegaStorm on SafeZone[%d].  GridCellThickness is less than 1.0.");
	return FindBytes(Addr, { 0x40, 0x55 }, 30000, 0, true);
}

static inline uint64 FindCollectGarbage()
{
	// return 0;

	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_CollectGarbageInternal");
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true, 1);
}

static inline uint64 FindActorGetNetMode()
{
	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 F6 41 08 10 48 8B D9 0F 85 ? ? ? ? 48 8B 41 20 48 85 C0 0F 84 ? ? ? ? F7 40").Get();

	if (Engine_Version == 427)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 93 ? ? ? ? 48 8B C8 48 8B F8 E8 ? ? ? ? 48 85 C0 75 29").Get();

	auto AActorGetNetmode = Memcury::Scanner::FindStringRef(L"STAT_ServerUpdateCamera");
	return Memcury::Scanner(FindBytes(AActorGetNetmode, { 0xE8 }, 255, 0, true)).RelativeOffset(1).Get();
}

static inline uint64 FindTickFlush()
{
	// auto add = Memcury::Scanner::FindStringRef(L"UDemoNetDriver::TickFlush: ReplayStreamer ERROR: %s");
	// return Memcury::Scanner(FindBytes(add, { 0xE8 }, 500, 0, true, 1)).RelativeOffset(1).Get();

	if (Engine_Version == 427)
		return Memcury::Scanner::FindPattern("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 8A").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_NetTickFlush");
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

static inline uint64 FindLoadPlayset()
{
	auto StringRef = Memcury::Scanner::FindStringRef(L"UPlaysetLevelStreamComponent::LoadPlayset Error: no owner for %s", Fortnite_Version >= 7);

	if (!StringRef.Get())
		return 0;

	return FindBytes(StringRef, { 0x48, 0x89, 0x5C }, 1000, 0, true);
}

static inline uint64 FindGIsServer()
{
	// auto add = Memcury::Scanner::FindStringRef(L"STAT_UpdateLevelStreaming");
	// return Memcury::Scanner(FindBytes(add, { 0x80, 0x3D }, 100, 0, true, 1)).RelativeOffset(2).Get();

	// if (Fortnite_Version == 19.10)
		// return __int64(GetModuleHandleW(0)) + 0xB30CF9D;

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

	auto Addr = Memcury::Scanner::FindStringRef(L"Changing GameSessionId from '%s' to '%s'");
	return FindBytes(Addr, { 0x40, 0x55 }, 2000, 0, true);
}

static inline uint64 FindDispatchRequest()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"MCP-Profile: Dispatching request to %s", true, 0, Fortnite_Version >= 18); // todo check s18
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 300, 0, true);
}

static inline uint64 FindGIsClient()
{
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
	auto Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%d]", false);

	if (!Addr.Get())
		Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%s]");

	return FindBytes(Addr, { 0x40, 0x55 }, 1000, 0, true);
}

static inline uint64 FindInternalTryActivateAbility()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"InternalTryActivateAbility called with invalid Handle! ASC: %s. AvatarActor: %s", true, 0, Fortnite_Version >= 18);
	return FindBytes(Addr, { 0x4C, 0x89, 0x4C }, 1000, 0, true);
}

static inline uint64 FindCanActivateAbility()
{
	if (Engine_Version == 421 || Engine_Version == 422)
		return Memcury::Scanner::FindPattern("4C 89 4C 24 20 55 56 57 41 56 48 8D 6C 24 D1").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"CanActivateAbility %s failed, blueprint refused");
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
	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 56 57 41 56 48 83 EC 20 83 B9 ? ? ? ? ? 49 8B F0 4C 8B F2 48 8B D9 7E 61").Get();

	if (Engine_Version == 421)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 83 B9 ? ? ? ? ? 49 8B E8 4C 8B F2").Get();

	// auto Addr = Memcury::Scanner::FindStringRef(L"GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!"); // has 2 refs for some reason on some versions
	// auto realGiveAbility = Memcury::Scanner(FindBytes(Addr, { 0xE8 }, 500, 0, false, 0, true)).RelativeOffset(1).Get();

	Memcury::Scanner addr = Memcury::Scanner(FindGiveAbilityAndActivateOnce());

	return Memcury::Scanner(FindBytes(addr, { 0xE8 }, 500, 0, false, 1)).RelativeOffset(1).Get();
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
		// todo hardcode sigs

		return 0;
	}

	return FindBytes(StringRef, (Engine_Version == 420 ? std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 } : std::vector<uint8_t>{ 0x4C, 0x8B }), 1000, 0, true);
}