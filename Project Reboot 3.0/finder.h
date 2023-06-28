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
	// ServerStatReplicatorInst then first jmp??

	if (Engine_Version == 500)
	{
		auto addr = Memcury::Scanner::FindPattern("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 33 F6 4C 8B E1 45 0F B6 E9 49 8B F8 41 8B C6", false).Get();

		if (!addr)
			addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 4C 89 64 24 ? 55 41 55 41 57 48 8B EC 48 83 EC 60 45 8A E1 4C 8B E9 48 83 FA").Get(); // 20.00

		return addr;
	}

	if (Engine_Version >= 427) // ok so like the func is split up in ida idfk what to do about it
	{
		if (Fortnite_Version < 18)
		{
			if (Fortnite_Version == 16.50)
				return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 60 45 33 ED 45 8A F9 44 38 2D ? ? ? ? 49 8B F8 48 8B F2 4C 8B E1").Get();
		
			return Memcury::Scanner::FindPattern("40 55 53 57 41 54 41 55 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85").Get();
		}
		else
			return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 60 45 33 ED 45 8A F9 44 38 2D ? ? ? ? 49 8B F8 48 8B").Get();
	}

	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("4C 8B DC 57 48 81 EC ? ? ? ? 80 3D ? ? ? ? ? 49 89 6B F0 49 89 73 E8").Get();

	if (Engine_Version == 419)
	{
		auto iasdfk = Memcury::Scanner::FindPattern("4C 8B DC 49 89 5B 08 49 89 6B 18 49 89 73 20 57 41 56 41 57 48 83 EC 60 80 3D", false).Get();

		if (!iasdfk)
			return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8B EC 48 83 EC 60 80 3D ? ? ? ? ? 45 0F B6 F1 49 8B F8").Get();

		return iasdfk;
	}

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
	{
		if (Fortnite_Version <= 6.02)
			return Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8D 04 D1").RelativeOffset(3).Get();

		return Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1").RelativeOffset(3).Get();
	}

	auto cc = Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 14 C8 EB 03 49 8B D6 8B 42 08 C1 E8 1D A8 01 0F 85 ? ? ? ? F7 86 ? ? ? ? ? ? ? ?", false);
	auto addr = cc.Get() ? cc.RelativeOffset(3).Get() : 0; // 4.16
	
	if (!addr)
	{
		if (Engine_Version >= 416 || Engine_Version <= 420)
		{
			auto aa = Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 1C C8 81 4B ? ? ? ? ? 49 63 76 30", false);
			addr = aa.Get() ? aa.RelativeOffset(3).Get() : 0;

			if (!addr)
			{
				addr = Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 1C C8 81 4B ? ? ? ? ? 49 63 76 30", false).Get() ? Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 1C C8 81 4B ? ? ? ? ? 49 63 76 30", false).RelativeOffset(3).Get() : 0;
			}
		}
	}

	return addr;
}

static inline uint64 FindAddToAlivePlayers()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"FortGameModeAthena: Player [%s] doesn't have a valid PvP team, and won't be added to the alive players list.").Get();

	if (!Addrr)
		return 0;

	for (int i = 0; i < 4000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x85 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xD2)
		{
			return Addrr - i;
		}
	}

	return 0;
}

static inline uint64 FindFinishResurrection()
{
	uintptr_t Addrr = Engine_Version >= 427 ? FindNameRef(L"OnResurrectionCompleted") : FindFunctionCall(L"OnResurrectionCompleted"); // Call is inlined

	if (!Addrr)
		return 0;

	// auto addr = Memcury::Scanner::FindPattern("40 53 48 83 EC 20 0F B6 81 ? ? ? ? 83 C2 03 48 8B D9 3B D0 0F 85").Get();
	// return addr;

	LOG_INFO(LogDev, "WTF: 0x{:x}", Addrr - __int64(GetModuleHandleW(0)));

	for (int i = 0; i < 2000; i++)
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

static inline uint64 FindGetSquadIdForCurrentPlayer()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"GetSquadIdForCurrentPlayer failed to find a squad id for player %s", true, 0, Fortnite_Version >= 19).Get();

	if (!Addrr)
		return 0;

	for (int i = 0; i < 2000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}
	}

	return 0;
}

static inline uint64 FindRebootingDelegate()
{
	if (Fortnite_Version < 8.3)
		return 0;

	auto ServerOnAttemptInteractAddr = Memcury::Scanner::FindStringRef(L"[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting", true, 0, Fortnite_Version >= 16).Get();
	
	for (int i = 0; i < 10000; i++)
	{
		if ((*(uint8_t*)(uint8_t*)(ServerOnAttemptInteractAddr + i) == 0x48 && *(uint8_t*)(uint8_t*)(ServerOnAttemptInteractAddr + i + 1) == 0x8D
			&& *(uint8_t*)(uint8_t*)(ServerOnAttemptInteractAddr + i + 2) == 0x05))
		{
			auto loadAddress = Memcury::Scanner(ServerOnAttemptInteractAddr + i).RelativeOffset(3).Get();

			if (IsNullSub(loadAddress)) // Safety
				return ServerOnAttemptInteractAddr + i;
		}
	}

	auto addr = 0; // Memcury::Scanner::FindPattern("48 8D 05 ? ? ? ? 33 F6 48 89 44 24 ? 49 8B CE 49 8B 06 89 74 24 60 FF 90 ? ? ? ? 4C 8B A4 24 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9").Get();

	return addr;
}

static inline uint64 FindPickupInitialize()
{
	if (Engine_Version == 419)
		return Memcury::Scanner::FindPattern("48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 80 B9 ? ? ? ? ? 41 0F B6 E9").Get(); // 1.11
	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 41 56 48 83 EC 20 80 B9 ? ? ? ? ? 45 0F B6 F1 49 8B E8").Get(); // 4.1
	if (Engine_Version == 421)
	{
		auto addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 55 57 41 57 48 83 EC 30 80 B9 ? ? ? ? ? 41 0F B6", false).Get(); // 6.21

		if (!addr)
			addr = Memcury::Scanner::FindPattern("48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 80 B9 ? ? ? ? ? 41 0F B6 E9").Get(); // 5.41

		return addr;
	}
	if (Engine_Version == 422)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 41 56 41 57 48 83 EC 30 80 B9 ? ? ? ? ? 45 0F B6 F1").Get(); // 7.30
	if (Engine_Version == 423)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 41 56 41 57 48 83 EC 30 80 B9 ? ? ? ? ? 45 0F B6 F1 4D").Get(); // 8.51 & 10.40

	return 0;
}

static inline uint64 FindCreateNetDriver()
{
	return 0;
}

static inline uint64 FindLoadAsset()
{
	return 0;

	auto Addrr = Memcury::Scanner::FindStringRef(L"Loaded delay-load asset %s").Get();

	for (int i = 0; i < 2000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 &&
			(*(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x74 || *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x6C))
		{
			return Addrr - i;
		}
	}

	return 0;
}

static inline uint64 FindKickPlayer()
{
	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("40 53 56 48 81 EC ? ? ? ? 48 8B DA 48 8B F1 E8 ? ? ? ? 48 8B 06 48 8B CE").Get();
	if (std::floor(Fortnite_Version) == 18)
		return Memcury::Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 4C 89 60 20 55 41 56 41 57 48 8B EC 48 83 EC 60 48 83 65 ? ? 4C 8B F2 83 65 E8 00 4C 8B E1 83 65 EC").Get();
	if (std::floor(Fortnite_Version) == 19)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 55 56 57 48 8B EC 48 83 EC 60 48 8B FA 48 8B F1 E8").Get();
	if (Engine_Version >= 423 || Engine_Version <= 425) // && instead of || ??
		return Memcury::Scanner::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 49 8B F0 48 8B DA 48 85 D2").Get();

	uint64 Ret = 0;

	auto Addr = Memcury::Scanner::FindStringRef(L"Validation Failure: %s. kicking %s", false, 0, Fortnite_Version >= 19);

	if (Addr.Get())
	{
		Ret = Addr.Get() ? FindBytes(Addr, { 0x40, 0x55 }, 1000, 0, true) : Ret;

		if (!Ret)
			Ret = Addr.Get() ? FindBytes(Addr, { 0x40, 0x53 }, 2000, 0, true) : Ret;
	}

	if (Ret)
		return Ret;

	auto Addr2 = Memcury::Scanner::FindStringRef(L"Failed to kick player"); // L"KickPlayer %s Reason %s"
	auto Addrr = Addr2.Get();

	for (int i = 0; i < 3000; i++)
	{
		/* if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x53)
		{
			return Addrr - i;
		} */

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}

		if (Fortnite_Version >= 17)
		{
			if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
			{
				return Addrr - i;
			}
		}
	}

	return Memcury::Scanner::FindPattern("40 53 41 56 48 81 EC ? ? ? ? 48 8B 01 48 8B DA 4C 8B F1 FF 90").Get();
}

static inline uint64 FindInitHost()
{
	if (Engine_Version == 427) // idk im dumb
	{
		auto addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B F1 4C 8D 05", false).Get();
	
		if (!addr) // s18
			addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 56 41 57 48 8D 68 A1 48 81 EC ? ? ? ? 48 8B F1 4C 8D 35 ? ? ? ? 4D").Get();

		return addr;
	}

	auto Addr = Memcury::Scanner::FindStringRef(L"BeaconPort=");
	return FindBytes(Addr, (Engine_Version == 427 ? std::vector<uint8_t>{ 0x48, 0x8B, 0x5C } : std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 }), 1000, 0, true);
}

static inline uint64 FindPickSupplyDropLocation()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"PickSupplyDropLocation: Failed to find valid location using rejection.  Using safe zone location.", true, 0).Get();

	if (!Addrr)
		return 0;

	// Newer versions it is "AFortAthenaMapInfo::PickSupplyDropLocation" (no wide str), but they also changed params so ill add later.

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}
	}

	return 0;
}

static inline uint64 FindPauseBeaconRequests()
{
	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 ED 48 8B F1 84 D2 74 27 80 3D").Get();

	if (Engine_Version == 427)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 33 F6 48 8B F9 84 D2 74").Get();

	// todo try 40 53 48 83 EC 30 48 8B ? 84 D2 74 ? 80 3D for S1-S15

	if (Engine_Version == 426)
	{
		auto addr = Memcury::Scanner::FindPattern("40 57 48 83 EC 30 48 8B F9 84 D2 74 62 80 3D", false).Get();

		if (!addr)
			addr = Memcury::Scanner::FindPattern("40 53 48 83 EC 30 48 8B D9 84 D2 74 5E 80 3D").Get();
		
		return addr;
	}

	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 30 48 8B D9 84 D2 74 68 80 3D ? ? ? ? ? 72 2C 48 8B 05 ? ? ? ? 4C 8D 44").Get();

	if (Fortnite_Version == 6.30 || Fortnite_Version == 6.31) // bro for real! (i think its cuz theres like 3 refs to the same string)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 30 48 8B D9 84 D2 74 68 80 3D").Get();

	if (Engine_Version == 419)
	{
		auto aa = Memcury::Scanner::FindPattern("40 53 48 83 EC 30 48 8B D9 84 D2 74 6F 80 3D", false).Get();

		if (!aa)
			return Memcury::Scanner::FindPattern("40 53 48 83 EC 30 48 8B D9 84 D2 74 68 80 3D ? ? ? ? ? 72").Get(); // i supposed this is just because its getitng wrong string ref

		return aa;
	}

	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 30 48 8B D9 84 D2 74 6F 80 3D ? ? ? ? ? 72 33 48 8B 05").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"All Beacon Requests Resumed.");
	return FindBytes(Addr, { 0x40, 0x53 }, 1000, 0, true);
}

static inline uint64 FindOnRep_ZiplineState()
{
	if (Fortnite_Version < 7)
		return 0;

	auto Addrr = Memcury::Scanner::FindStringRef(L"ZIPLINES!! Role(%s)  AFortPlayerPawn::OnRep_ZiplineState ZiplineState.bIsZiplining=%d", false).Get();

	if (!Addrr)
		Addrr = Memcury::Scanner::FindStringRef(L"ZIPLINES!! GetLocalRole()(%s)  AFortPlayerPawn::OnRep_ZiplineState ZiplineState.bIsZiplining=%d", false).Get();

	if (!Addrr)
		Addrr = Memcury::Scanner::FindStringRef("AFortPlayerPawn::HandleZiplineStateChanged").Get(); // L"%s LocalRole[%s] ZiplineState.bIsZiplining[%d]"

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

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
		{
			return Addrr - i;
		}
	}
	
	return 0;
}

static inline uint64 FindGetMaxTickRate() // UEngine::getmaxtickrate
{
	// TODO switch to index maybe?

	/* auto GetMaxTickRateIndex = *Memcury::Scanner::FindStringRef(L"GETMAXTICKRATE")
        .ScanFor({ 0x4D, 0x8B, 0xC7, 0xE8 })
        .RelativeOffset(4)
        .ScanFor({ 0xFF, 0x90 })
        .AbsoluteOffset(2)
        .GetAs<int*>() / 8;

    LOG_INFO(LogHook, "GetMaxTickRateIndex {}", GetMaxTickRateIndex); */

	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 50 0F 29 74 24 ? 48 8B D9 0F 29 7C 24 ? 0F 28 F9 44 0F 29").Get(); // the string is in func + it's in function chunks.

	if (Engine_Version == 427)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 60 0F 29 74 24 ? 48 8B D9 0F 29 7C 24 ? 0F 28").Get(); // function chunks woo!

	auto Addrr = Memcury::Scanner::FindStringRef(L"Hitching by request!").Get();

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
	// return FindBytes(stringRef, Fortnite_Version <= 4.1 ? std::vector<uint8_t>{ 0x40, 0x53 } : std::vector<uint8_t>{ 0x48, 0x89, 0x5C }, 1000, 0, true);
}

uint64 FindStartAircraftPhase();
uint64 FindGetSessionInterface();
uint64 FindGetPlayerViewpoint();
uint64 ApplyGameSessionPatch();

static inline uint64 FindFree()
{
	uint64 addr = 0;

	if (Engine_Version >= 420 && Engine_Version <= 426)
		addr = Memcury::Scanner::FindPattern("48 85 C9 74 2E 53 48 83 EC 20 48 8B D9").Get();
	else if (Engine_Version >= 427)
		addr = Memcury::Scanner::FindPattern("48 85 C9 0F 84 ? ? ? ? 53 48 83 EC 20 48 89 7C 24 ? 48 8B D9 48 8B 3D").Get();

	return addr;
}

static inline uint64 FindStepExplicitProperty()
{
	return Memcury::Scanner::FindPattern("41 8B 40 ? 4D 8B C8").Get();
}

static inline uint64 FindIsNetRelevantForOffset()
{
	if (Engine_Version == 416) // checked on 1.7.2 & 1.8
		return 0x420 / 8;
	if (Fortnite_Version == 1.11 || (Fortnite_Version >= 2.42 && Fortnite_Version <= 3.2)) // checked 1.11, 2.4.2, 2.5, 3.0, 3.1
		return 0x418 / 8;

	return 0;
}

static inline uint64 FindActorChannelClose()
{
	auto StringRef = Memcury::Scanner::FindStringRef(L"UActorChannel::Close: ChIndex: %d, Actor: %s");

	return FindBytes(StringRef, { 0x48, 0x89, 0x5C }, 1000, 0, true);
}

static inline uint64 FindSpawnActor()
{
	if (Engine_Version >= 427)
	{
		auto stat = Memcury::Scanner::FindStringRef(L"STAT_SpawnActorTime");
		return FindBytes(stat, { 0x48, 0x8B, 0xC4 }, 3000, 0, true);
	}

	auto Addr = Memcury::Scanner::FindStringRef(L"SpawnActor failed because no class was specified");

	if (Engine_Version >= 416 && Fortnite_Version <= 3.2)
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
	if (Fortnite_Version == 20.40)
		SetWorldIndex = 0x7B;

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
	auto Addr = FindFunctionCall(L"OnDamageServer", 
		Engine_Version == 416 ? std::vector<uint8_t>{ 0x4C, 0x89, 0x4C } : 
		Engine_Version == 419 || Engine_Version >= 427 ? std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 } : std::vector<uint8_t>{ 0x40, 0x55 }
	);

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

	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("4C 8B C9 48 8B 44 24 ? 83 C9 FF 41 80 61 ? ? 41 80 61 ? ? 49 89 41 20 33 C0 41 88 41 30 49 89 41").Get();

	return 0;
}

static inline uint64 FindCreateBuildingActorCallForDeco() // kill me
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"ServerCreateBuildingAndSpawnDeco called without a valid DecoItemDef").Get(); // honestly L (we should get it from the ufunc not string)

	if (!Addrr)
		return 0;

	for (int i = 0; i < 10000; i++)
	{
		if ((*(uint8_t*)(uint8_t*)(Addrr + i) == 0xC6 && *(uint8_t*)(uint8_t*)(Addrr + i + 6) == 0xE8)) // Checked 10.40
		{
			return Addrr - i + 6;
		}
	}

	return 0;
}

static inline uint64 FindUpdateTrackedAttributesLea() // kill me
{
	// 10.40 = (__int64(GetModuleHandleW(0)) + 0x19E19A5)

	// So we keep going until we find a lea with nullsub..

	uint64 ApplyGadgetAttributesAddr = Memcury::Scanner::FindPattern("48 85 D2 0F 84 ? ? ? ? 55 41 54 41 55 41 57 48 8D 6C 24").Get();

	if (!ApplyGadgetAttributesAddr)
		return 0;

	for (int i = 0; i < 10000; i++)
	{
		if ((*(uint8_t*)(uint8_t*)(ApplyGadgetAttributesAddr + i) == 0x48 && *(uint8_t*)(uint8_t*)(ApplyGadgetAttributesAddr + i + 1) == 0x8D
			&& *(uint8_t*)(uint8_t*)(ApplyGadgetAttributesAddr + i + 2) == 0x05))
		{
			auto loadAddress = Memcury::Scanner(ApplyGadgetAttributesAddr + i).RelativeOffset(3).Get();

			if (IsNullSub(loadAddress)) // Safety
				return ApplyGadgetAttributesAddr + i;
		}
	}

	return 0;
}

static inline uint64 FindCombinePickupLea() // kill me
{
	// return 0;

	/* uint64 OnRep_PickupLocationDataAddr = 0; // TODO (Idea: Find SetupCombinePickupDelegates from this).

	if (!OnRep_PickupLocationDataAddr)
		return 0; */
	
	uint64 SetupCombinePickupDelegatesAddr = 0;
	
	bool bShouldCheckSafety = true;

	if (Engine_Version <= 420)
	{
		SetupCombinePickupDelegatesAddr = Memcury::Scanner::FindPattern("49 89 73 10 49 89 7B 18 4D 89 73 20 4D 89 7B E8 41 0F 29 73 ? 75").Get(); // found on 4.1
		bShouldCheckSafety = false; // it's like "corrupted" and not just return
	}
	else if (Engine_Version >= 421)
	{
		SetupCombinePickupDelegatesAddr = Memcury::Scanner::FindPattern("48 89 AC 24 ? ? ? ? 48 89 B4 24 ? ? ? ? 48 89 BC 24 ? ? ? ? 0F 29 B4 24 ? ? ? ? 75").Get(); // Haha so funny thing, this isn't actually the start its the middle because it's in function chunks yay!
	}

	if (!SetupCombinePickupDelegatesAddr)
		return 0;

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(SetupCombinePickupDelegatesAddr + i) == 0x48 && *(uint8_t*)(uint8_t*)(SetupCombinePickupDelegatesAddr + i + 1) == 0x8D
			&& *(uint8_t*)(uint8_t*)(SetupCombinePickupDelegatesAddr + i + 2) == 0x05) // Checked on 10.40, it was the first lea.
		{
			auto loadAddress = Memcury::Scanner(SetupCombinePickupDelegatesAddr + i).RelativeOffset(3).Get();

			if (!bShouldCheckSafety || IsNullSub(loadAddress))
				return SetupCombinePickupDelegatesAddr + i;
		}
	}

	return 0;
}

static inline uint64 FindCompletePickupAnimation()
{
	if (Engine_Version == 416 || Engine_Version == 419)
		return Memcury::Scanner::FindPattern("4C 8B DC 53 55 56 48 83 EC 60 48 8B F1 48 8B 89 ? ? ? ? 48 85 C9").Get();

	if (Engine_Version == 420)
	{
		auto addy = Memcury::Scanner::FindPattern("4C 8B DC 53 55 56 48 83 EC 60 48 8B F1 48 8B 89", false).Get(); // 3.1

		if (!addy)
			addy = Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B D9 48 8B 89 ? ? ? ? 48 85 C9 74 20 48 8D 44 24").Get();

		return addy;
	}

	if (Engine_Version == 421)
	{
		auto adda = Memcury::Scanner::FindPattern("40 53 56 48 83 EC 38 4C 89 6C 24 ? 48 8B F1 4C 8B A9", false).Get();

		if (!adda)
			adda = Memcury::Scanner::FindPattern("40 53 56 57 48 83 EC 30 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED 0F 84").Get(); // 6.21

		return adda;
	}

	if (Engine_Version == 422)
		return Memcury::Scanner::FindPattern("40 53 56 57 48 83 EC 30 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED 0F 84").Get(); // 7.30

	if (Engine_Version >= 423 && Engine_Version <= 426)
		return Memcury::Scanner::FindPattern("40 53 56 48 83 EC 38 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED").Get(); // 10.40

	if (Engine_Version == 427)
	{
		auto sig = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 54 41 56 41 57 48 83 EC 20 48 8B B1 ? ? ? ? 48 8B D9 48 85 F6", false).Get(); // 17.30
	
		if (!sig)
			sig = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B B9 ? ? ? ? 48 8B D9 48 85 FF 74 16 48 89", false).Get(); // 18.40

		if (!sig)
			sig = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 10 48 89 68 18 57 48 83 EC 20 48 8B D9 48 8B 89 ? ? ? ? 48 85").Get(); // 16.50

		return sig;
	}

	if (Engine_Version == 500)
	{
		auto addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B B9", false).Get(); // 19.10;

		if (!addr)
			addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B B9 ? ? ? ? 45 33 E4 48 8B D9 48 85 FF 74 0F").Get(); // 20.40
	
		return addr;
	}

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

	if (Fortnite_Version == 4.5)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 90 EB EA").RelativeOffset(1).Get();

	if (std::floor(Fortnite_Version) == 3)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 83 A7 ? ? ? ? ? 48 8D 4C 24 ?").RelativeOffset(1).Get();

	if (std::floor(Fortnite_Version) == 4)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 83 A7 ? ? ? ? ? 83 E0 01").RelativeOffset(1).Get();

	if (std::floor(Fortnite_Version) == 5)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? 84 C0 75 CE").RelativeOffset(1).Get();

	LOG_INFO(LogDev, "finding it");
	auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.IsRunningNoMCP");
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
	if (Engine_Version == 419)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 70 48 8B B9 ? ? ? ? 33 DB 0F 29 74 24 ? 48 8B F1 48 85 FF 74 2C E8").Get(); // 1.11
	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? EB 31 80 B9 ? ? ? ? ?").RelativeOffset(1).Get(); // 3.5
	if (Fortnite_Version >= 7 && Fortnite_Version <= 8) // intentional, 8.00 has the same pattern.
		return Memcury::Scanner::FindPattern("E9 ? ? ? ? 48 8B C1 40 38 B9").RelativeOffset(1).Get(); // 7.40
	if (Engine_Version == 423)
		return Memcury::Scanner::FindPattern("E8 ? ? ? ? EB 42 80 BA").RelativeOffset(1).Get(); // doesnt work

	auto Addr = Memcury::Scanner::FindStringRef(L"FortGameModeAthena: No MegaStorm on SafeZone[%d].  GridCellThickness is less than 1.0.", true, 0, Engine_Version >= 427).Get();
	// return FindBytes(Addr, { 0x40, 0x55 }, 30000, 0, true);

	if (!Addr)
		return 0;

	for (int i = 0; i < 100000; i++)
	{
		if ((*(uint8_t*)(uint8_t*)(Addr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x53) 
			|| (*(uint8_t*)(uint8_t*)(Addr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x55))
		{
			return Addr - i;
		}

		if (Fortnite_Version < 8)
		{
			if (*(uint8_t*)(uint8_t*)(Addr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addr - i + 2) == 0x5C)
			{
				return Addr - i;
			}
		}

		if (*(uint8_t*)(uint8_t*)(Addr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addr - i + 2) == 0xC4)
		{
			return Addr - i;
		}
	}

	return 0;
}

static inline uint64 FindSetTimer()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_SetTimer", false).Get();

	if (!Addr)
		return 0;

	for (int i = 0; i < 1000; i++)
	{
		/* if ((*(uint8_t*)(uint8_t*)(Addr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x53)
			|| (*(uint8_t*)(uint8_t*)(Addr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x55))
		{
			return Addr - i;
		}

		if (Fortnite_Version < 8)
		{
			if (*(uint8_t*)(uint8_t*)(Addr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addr - i + 2) == 0x5C)
			{
				return Addr - i;
			}
		} */

		if (*(uint8_t*)(uint8_t*)(Addr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addr - i + 2) == 0xC4)
		{
			return Addr - i;
		}
	}

	return 0;
}

static inline uint64 FindEnterAircraft()
{
	auto Addr = Memcury::Scanner::FindStringRef(L"EnterAircraft: [%s] is attempting to enter aircraft after having already exited.", true, 0, Engine_Version >= 500).Get();

	for (int i = 0; i < 1000; i++)
	{
		if ((*(uint8_t*)(uint8_t*)(Addr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x53)
			|| (*(uint8_t*)(uint8_t*)(Addr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x55))
		{
			return Addr - i;
		}

		if (Fortnite_Version >= 15)
		{
			if (*(uint8_t*)(uint8_t*)(Addr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addr - i + 2) == 0x5C && *(uint8_t*)(uint8_t*)(Addr - i + 3) == 0x24)
			{
				return Addr - i;
			}
		}

		if (*(uint8_t*)(uint8_t*)(Addr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addr - i + 2) == 0x74) // 4.1
		{
			return Addr - i;
		}
	}

	return 0;
}

static inline uint64 FindFreeArrayOfEntries()
{
	// horrific way

	if (Engine_Version == 422 || Engine_Version == 423)
		return Memcury::Scanner::FindPattern("48 83 EC 38 48 89 6C 24 ? 4C 89 74 24 ? 4C 8B F1 48 8B 09 41 8B 6E 08 85 ED 0F 84 ? ? ? ? 48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 48 8D B9").Get(); // 7.30 & 10.40
	
	return 0;
}

static inline uint64 FindFreeEntry()
{
	// horrific way

	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F1 48 8B 89 ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 48 8B 8E ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 48 8B 8E ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 48 8B 9E ? ? ? ? 48 85").Get(); // 4.1

	return 0;
}

static inline uint64 FindBeginningOfFuncEXP(uint64 Addr, int ToSearch = 1000)
{
	for (int i = 0; i < ToSearch; i++)
	{
		if (Fortnite_Version >= 10)
		{
			if ((*(uint8_t*)(uint8_t*)(Addr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x53))
			{
				return Addr - i;
			}
		}

		if ((*(uint8_t*)(uint8_t*)(Addr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x55))
		{
			return Addr - i;

		}

		if (*(uint8_t*)(uint8_t*)(Addr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addr - i + 2) == 0x5C)
		{
			return Addr - i;

		}

		/* if (*(uint8_t*)(uint8_t*)(Addr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addr - i + 2) == 0xC4)
		{
			return Addr - i;
		} */
	}

	return 0;
}

static inline uint64 FindRemoveGadgetData()
{
	uint64 RemoveGadgetDataAddr = 0;

	if (Engine_Version <= 423)
	{
		auto Addr = Memcury::Scanner::FindStringRef(L"UFortGadgetItemDefinition::RemoveGadgetData - Removing Gadget Data for Gadget Item [%s]!", false).Get();

		if (!Addr)
			Addr = Memcury::Scanner::FindStringRef(L"UFortGadgetItemDefinition::RemoveGadgetData - Removing Gadget Data for Gadet Item [%s]!", false).Get();

		if (!Addr)
			Addr = Memcury::Scanner::FindStringRef(L"UFortGadgetItemDefinition::RemoveGadgetData - Failed to get the Player Controller to cleanup Gadget Item [%s]!").Get();

		if (!Addr)
			return 0;

		RemoveGadgetDataAddr = FindBeginningOfFuncEXP(Addr);
	}
	else if (Engine_Version == 426)
		RemoveGadgetDataAddr = Memcury::Scanner::FindPattern("48 85 D2 0F 84 ? ? ? ? 56 41 56 41 57 48 83 EC 30 48 8B 02 48").Get(); // 14.60
	
	LOG_INFO(LogDev, "RemoveGadgetData: 0x{:x}", RemoveGadgetDataAddr - __int64(GetModuleHandleW(0)));

	if (!RemoveGadgetDataAddr)
		return 0;

	uint64 RemoveGadgetDataCall = Memcury::Scanner::FindPointerRef((PVOID)RemoveGadgetDataAddr, 0, false, false).Get();

	if (!RemoveGadgetDataCall)
		return RemoveGadgetDataAddr;

	uint64 FortGadgetItemDefinition_RemoveGadgetDataAddr = FindBeginningOfFuncEXP(RemoveGadgetDataCall);

	if (!FortGadgetItemDefinition_RemoveGadgetDataAddr)
		return RemoveGadgetDataAddr;

	uint64 FortGadgetItemDefinition_RemoveGadgetDataCall = Memcury::Scanner::FindPointerRef((PVOID)FortGadgetItemDefinition_RemoveGadgetDataAddr, 0, false, false).Get();

	if (!FortGadgetItemDefinition_RemoveGadgetDataCall)
		return FortGadgetItemDefinition_RemoveGadgetDataAddr;
	
	uint64 AthenaGadgetItemDefinition_RemoveGadgetDataAddr = FindBeginningOfFuncEXP(FortGadgetItemDefinition_RemoveGadgetDataCall);

	if (!AthenaGadgetItemDefinition_RemoveGadgetDataAddr)
		return FortGadgetItemDefinition_RemoveGadgetDataAddr;

	return AthenaGadgetItemDefinition_RemoveGadgetDataAddr;
}

static inline uint64 FindApplyGadgetData()
{
	uint64 FortGadgetItemDefinition_ApplyGadgetDataAddr = 0;

	if (Engine_Version >= 420 && Engine_Version <= 422)
		FortGadgetItemDefinition_ApplyGadgetDataAddr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 41 0F B6 D9 49 8B").Get(); // 4.1 & 6.21 & 7.40
	if (Engine_Version >= 423 && Engine_Version <= 426)
		FortGadgetItemDefinition_ApplyGadgetDataAddr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC 20 41 0F").Get(); // 8.51 & 12.41

	uint64 FortGadgetItemDefinition_ApplyGadgetDataCall = Memcury::Scanner::FindPointerRef((PVOID)FortGadgetItemDefinition_ApplyGadgetDataAddr, 0, false, false).Get();

	if (!FortGadgetItemDefinition_ApplyGadgetDataCall)
		return FortGadgetItemDefinition_ApplyGadgetDataAddr;

	auto AthenaGadgetItemDefinition_ApplyGadgetDataAddr = FindBeginningOfFuncEXP(FortGadgetItemDefinition_ApplyGadgetDataCall);

	if (!AthenaGadgetItemDefinition_ApplyGadgetDataAddr)
		return FortGadgetItemDefinition_ApplyGadgetDataAddr;

	return AthenaGadgetItemDefinition_ApplyGadgetDataAddr;
}

static inline uint64 FindGetInterfaceAddress()
{
	if (Engine_Version <= 421)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 FF 48 8B DA 48 8B F1 48").Get(); // 4.1 & 6.21

	return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 DB 48 8B FA 48 8B F1 48 85 D2 0F 84 ? ? ? ? 8B 82 ? ? ? ? C1 E8").Get();
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

	if (Engine_Version == 500) // hah well this and 427 does like nothing cuz inline mostly
	{
		auto addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 F6 41 08 10 48 8B D9 0F 85 ? ? ? ? 48 8B 41 20 48 85 C0 0F 84 ? ? ? ? F7 40", false).Get();

		if (!addr)
			addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 F6 41 08 10 48 8B D9 0F 85").Get(); // 20.40

		return addr;
	}

	if (Engine_Version == 427)
	{
		auto addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 93 ? ? ? ? 48 8B C8 48 8B F8 E8 ? ? ? ? 48 85 C0 75 29", false).Get();

		if (!addr)
			addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 F6 41 08 10 48 8B D9 0F 85 ? ? ? ? 48 8B 41 20 48 85 C0 0F 84").Get(); // 17.50 & 18.40

		return addr;
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
		Addrr = Memcury::Scanner::FindStringRef(L"FortGameModeAthena::RemoveFromAlivePlayers: Player [%s] PC [%s] removed from alive players list (Team [%d]).  Player count is now [%d]. PlayerBots count is now [%d]. Team count is now [%d].", true, 0, Fortnite_Version >= 16).Get(); // checked on 16.40

	for (int i = 0; i < 2000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x4C && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x4C) // most common
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x54) // idk what verisont bh
		{
			for (int z = 3; z < 50; z++)
			{
				if (*(uint8_t*)(uint8_t*)(Addrr - i - z) == 0x4C && *(uint8_t*)(uint8_t*)(Addrr - i - z + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i - z + 2) == 0x4C)
				{
					return Addrr - i - z;
				}
			}

			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4) // i forgot what version
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
		auto addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 8A", false).Get();

		if (!addr) // s18
			addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 44 0F", false).Get();
	
		if (!addr)
			addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B F9 48 89 4D 38 48 8D 4D 40").Get(); // 16.50

		return addr;
	}

	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_NetTickFlush", false);

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
	uint64 addr = 0;

	if (Engine_Version == 421)
		addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 83 B9 ? ? ? ? ? 41 0F B6 F1 0F B6 FA 48", false).Get();
	else if (Engine_Version == 423)
		addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 ED 41", false).Get();
	else if (Engine_Version == 425)
		addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 ED 41 0F B6 F1").Get();

	return addr;
}

static inline uint64 FindNavSystemCleanUp()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"UNavigationSystemV1::CleanUp", false).Get();

	if (!Addrr)
		return 0;

	for (int i = 0; i < 500; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x53)
		{
			return Addrr - i;
		}
	}

	return 0;
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

	auto Addrr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering").Get();

	/* int found = 0;

	for (int i = 0; i < 600; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x88 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x1D)
		{
			for (int z = 0; z < 15; z++)
			{
				LOG_INFO(LogDev, "[{}] [{}] GIsServerTest: 0x{:x}", found, z, Memcury::Scanner(Addrr - i).RelativeOffset(z).Get() - __int64(GetModuleHandleW(0)));
			}

			found++;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0xC6 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x05)
		{
			for (int z = 0; z < 15; z++)
			{
				LOG_INFO(LogDev, "[{}] [{}] GIsServerTest: 0x{:x}", found, z, Memcury::Scanner(Addrr - i).RelativeOffset(z).Get() - __int64(GetModuleHandleW(0)));
			}

			found++;
		}
	} */

	if (Fortnite_Version == 4.1)
		return __int64(GetModuleHandleW(0)) + 0x4BF6F18;
	if (Fortnite_Version == 10.40)
		return __int64(GetModuleHandleW(0)) + 0x637925C;
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
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 70 4C 8B FA 4C").Get(); // no work on s18
	}

	if (Fortnite_Version == 2.5)
		return Memcury::Scanner::FindPattern("40 55 56 41 56 48 8B EC 48 81 EC ? ? ? ? 48 8B 01 4C 8B F2").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"Changing GameSessionId from '%s' to '%s'");
	return FindBytes(Addr, { 0x40, 0x55 }, 2000, 0, true);
}

static inline uint64 FindDispatchRequest()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"MCP-Profile: Dispatching request to %s", false, 0, Fortnite_Version >= 19).Get();

	if (!Addrr)
	{
		return 0;
	}

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
		{
			return Addrr - i;
		}
	}

	return 0;

	// return FindBytes(Addr, std::floor(Fortnite_Version) == 18 ? std::vector<uint8_t>{0x48, 0x8B, 0xC4 } : std::vector<uint8_t>{ 0x48, 0x89, 0x5C }, 300, 0, true);
}

static inline uint64 FindMcpIsDedicatedServerOffset()
{
	if (Engine_Version == 421 || Engine_Version == 422) // checked on 5.41 & 6.21 & 7.30
		return 0x28;

	return 0x60; // 1.7.2 & 1.11 & 4.1
}

static inline uint64 FindGIsClient()
{
	/* if (Fortnite_Version >= 20)
		return 0; */

	auto Addr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering");

	std::vector<std::vector<uint8_t>> BytesArray = { {0x88, 0x05}, {0xC6, 0x05}, {0x88, 0x1D}, {0x44, 0x88}};

	int Skip = Engine_Version <= 420 ? 1 : 2;

	uint64 Addy;

	for (int i = 0; i < 50; i++) // we should subtract from skip if goup
	{
		auto CurrentByte = *(Memcury::ASM::MNEMONIC*)(Addr.Get() - i);

		// if (bPrint)
			// std::cout << "CurrentByte: " << std::hex << (int)CurrentByte << '\n';

		bool ShouldBreak = false;

		// LOG_INFO(LogDev, "[{}] Byte: 0x{:x}", i, (int)CurrentByte);

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
					int Relative = Bytes[0] == 0x44 ? 3 : 2;
					// LOG_INFO(LogDev, "[{}] No Rel 0x{:x} Rel: 0x{:x}", Skip, Memcury::Scanner(Addr.Get() - i).Get() - __int64(GetModuleHandleW(0)), Memcury::Scanner(Addr.Get() - i).RelativeOffset(Relative).Get() - __int64(GetModuleHandleW(0)));
					
					if (Skip > 0)
					{
						Skip--;
						continue;
					}

					Addy = Memcury::Scanner(Addr.Get() - i).RelativeOffset(Relative).Get();
					ShouldBreak = true;
					break;
				}
			}
		}

		if (ShouldBreak)
			break;

		// std::cout << std::format("CurrentByte: 0x{:x}\n", (uint8_t)CurrentByte);
	}

	// LOG_INFO(LogDev, "Addy: 0x{:x}", Addy - __int64(GetModuleHandleW(0)));

	return Addy; // 0; // Memcury::Scanner(Addy3).RelativeOffset(2).Get();

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
	if (std::floor(Fortnite_Version) == 18)
		return Memcury::Scanner::FindPattern("48 83 EC 28 48 83 79 ? ? 75 20 48 8B 91 ? ? ? ? 48 85 D2 74 1E 48 8B 02 48 8B CA FF 90").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"PREPHYSBONES");

	auto BeginningFunction = FindBytes(Addr, { 0x40, 0x55 }, 1000, 0, true);

	uint64 CallToFunc = 0;

	for (int i = 0; i < 400; i++)
	{
		if ((*(uint8_t*)(uint8_t*)(BeginningFunction + i) == 0xE8) && (*(uint8_t*)(uint8_t*)(BeginningFunction + i - 1) != 0x8B)) // scuffed but idk how to guarantee its not a register
		{
			CallToFunc = BeginningFunction + i;
			break;
		}
	}

	if (!CallToFunc)
	{
		LOG_WARN(LogDev, "Failed to find call for UWorld::GetNetMode! Report this to Milxnor immediately.");
		return 0;
	}

	LOG_INFO(LogDev, "CallToFunc: 0x{:x}", CallToFunc - __int64(GetModuleHandleW(0)));

	return Memcury::Scanner(CallToFunc).RelativeOffset(1).Get();

	// return (uintptr_t)GetModuleHandleW(0) + 0x34d2140;
}

static inline uint64 FindApplyCharacterCustomization()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"AFortPlayerState::ApplyCharacterCustomization - Failed initialization, using default parts. Player Controller: %s PlayerState: %s, HeroId: %s", false, 0, Fortnite_Version >= 20, true).Get();

	if (!Addrr)
		return 0;

	for (int i = 0; i < 7000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x53)
		{
			return Addrr - i;
		}

		if (Fortnite_Version >= 15) // hm?
		{
			if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
			{
				return Addrr - i;
			}
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
		{
			return Addrr - i;
		}
	}

	uint64 Addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 50 10 55 57 48 8D 68 A1 48 81 EC ? ? ? ? 80 B9").Get();

	return Addr;
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
	if (Engine_Version == 426)
	{
		auto testAddr = Memcury::Scanner::FindPattern("88 54 24 10 53 56 41 54 41 55 41 56 48 83 EC 60 4C 8B A1", false).Get(); // 14.60 what is happening lol ????

		if (!testAddr)
			testAddr = Memcury::Scanner::FindPattern("88 54 24 10 53 55 56 41 55 41 ? 48 83 EC 70 48", false).Get(); // 15.10 & 15.50

		if (testAddr)
			return testAddr;
	}

	else if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 88 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 70 45 33 ED 4D").Get(); // 19.10

	else if (Engine_Version >= 427) // different start
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 88 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 70 4C 8B A1").Get();

	if (Fortnite_Version == 7.20 || Fortnite_Version == 7.30)
		return Memcury::Scanner::FindPattern("89 54 24 10 53 56 41 54 41 55 41 56 48 81 EC").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%d]", false, 0, Engine_Version >= 427); // todo check if its just s18+ but this doesn't matter for now cuz we hardcode sig

	if (!Addr.Get())
		Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%s]"); // i don't even know what version this is

	return FindBytes(Addr, Fortnite_Version <= 4.1 ? std::vector<uint8_t>{ 0x48, 0x89, 0x6C } : std::vector<uint8_t>{ 0x40, 0x55 }, 1000, 0, true);
}

static inline uint64 FindInternalTryActivateAbility()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"InternalTryActivateAbility called with invalid Handle! ASC: %s. AvatarActor: %s", true, 0, Fortnite_Version >= 16).Get(); // checked 16.40

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

	auto Addrr = Memcury::Scanner::FindStringRef(L"CanActivateAbility %s failed, blueprint refused", true, 0, Engine_Version >= 500).Get();

	for (int i = 0; i < 2000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
		{
			return Addrr - i;
		}
	}

	return 0;

	// auto Addr = Memcury::Scanner::FindStringRef(L"CanActivateAbility %s failed, blueprint refused", true, 0, Engine_Version >= 500);
	// return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true);
}

static inline uint64 FindGiveAbilityAndActivateOnce()
{
	if (Engine_Version == 426)
	{
		auto sig1 = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 49 8B 40 10 49 8B D8 48 8B FA 48 8B F1", false).Get();

		if (!sig1)
			sig1 = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 40 49 8B 40 10 49").Get(); // 15.50

		return sig1;
	}

	auto Addrr = Memcury::Scanner::FindStringRef(L"GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!", true, 0, Engine_Version >= 500).Get();

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x55)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}
	}

	return 0;

	/* auto Addr = Memcury::Scanner::FindStringRef(L"GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!", true, 0, Engine_Version >= 500);
	auto res = FindBytes(Addr, { 0x48, 0x89, 0x5C }, 1000, 0, true);

	return res; */
}

static inline uint64 FindGiveAbility()
{
	if (Engine_Version <= 420)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 56 57 41 56 48 83 EC 20 83 B9").Get();
	/* if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 56 57 41 56 48 83 EC 20 83 B9").Get();
	if (Fortnite_Version == 1.11)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 56 57 41 56 48 83 EC 20 83 B9 ? ? ? ? ? 49 8B F0").Get();
	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 56 57 41 56 48 83 EC 20 83 B9 ? ? ? ? ? 49 8B F0 4C 8B F2 48 8B D9 7E 61").Get(); */
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
		add = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 60 49 8B E9 4D 8B F8 48 8B DA 48 8B F9 BE ? ? ? ? 48", false).Get(); // 5.00

	if (!add)
		add = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 70 49 8B E9 4D 8B F8 48 8B DA 48 8B F9").Get(); // 20.00

	return add;

	auto CreateBuildingActorAddr = Memcury::Scanner(GetFunctionIdxOrPtr(FindObject<UFunction>(L"/Script/FortniteGame.FortAIController.CreateBuildingActor")));
	auto LikeHuh = Memcury::Scanner(FindBytes(CreateBuildingActorAddr, { 0x40, 0x88 }, 3000));
	auto callaa = Memcury::Scanner(FindBytes(LikeHuh, { 0xE8 }));

	return callaa.RelativeOffset(1).Get();
}

static inline uint64 FindReplaceBuildingActor()
{
	auto StringRef = Memcury::Scanner::FindStringRef(L"STAT_Fort_BuildingSMActorReplaceBuildingActor", false);

	if (!StringRef.Get()) // we are on a version where stats dont exist
	{
		return Memcury::Scanner::FindPattern("4C 89 44 24 ? 55 56 57 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 45").Get(); // 1.7.2 & 2.4.2
	}

	return FindBytes(StringRef, 
		(Engine_Version == 420 || Engine_Version == 421 || Engine_Version >= 427 ? std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 } : std::vector<uint8_t>{ 0x4C, 0x8B }),
		1000, 0, true);
}

static inline uint64 FindSendClientAdjustment()
{
	if (Fortnite_Version <= 3.2)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 20 48 8B 99 ? ? ? ? 48 39 99 ? ? ? ? 74 0A 48 83 B9").Get();
	if (Fortnite_Version >= 20)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 20 48 8B 99 ? ? ? ? 48 39 99 ? ? ? ? 74 0A 48 83 B9").Get();

	return 0;
}

static inline uint64 FindReplicateActor()
{
	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("40 55 53 57 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8D 59 68 4C 8B F1 48 8B").Get();
	if (Engine_Version >= 419 && Fortnite_Version <= 3.2)
	{
		auto addr = Memcury::Scanner::FindPattern("40 55 56 57 41 54 41 55 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 4C", false).Get(); // 3.0, we could just use this sig for everything?

		if (!addr)
			addr = Memcury::Scanner::FindPattern("40 55 56 41 54 41 55 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 4C 8B E9 48 8B 49 68 48").Get();

		return addr;
	}

	if (Fortnite_Version >= 20)
		return Memcury::Scanner::FindPattern("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8D 69 68").Get();

	return 0;
}

static inline uint64 FindCreateChannel()
{
	if (Fortnite_Version <= 3.2)
		return Memcury::Scanner::FindPattern("40 56 57 41 54 41 55 41 57 48 83 EC 60 48 8B 01 41 8B F9 45 0F B6 E0").Get();
	if (Fortnite_Version >= 20)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 44 89 4C 24 ? 55 57 41 54 41 56 41 57 48 8B EC 48 83 EC 50 45 33 E4 48 8D 05 ? ? ? ? 44 38 25").Get();

	return 0;
}

static inline uint64 FindSetChannelActor()
{
	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("4C 8B DC 55 53 57 41 54 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 45 33").Get();
	if (Engine_Version >= 419 && Fortnite_Version <= 3.2)
	{
		auto aa = Memcury::Scanner::FindPattern("48 8B C4 55 53 57 41 54 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 45 33 E4 48 89 70", false).Get();

		if (!aa)
			return Memcury::Scanner::FindPattern("48 8B C4 55 53 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 89 70 E8 48 8B D9").Get();
		
		return aa;
	}
	if (Fortnite_Version >= 20)
		return Memcury::Scanner::FindPattern("40 55 53 56 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 45 33 E4 48 8D 3D ? ? ? ? 44 89 A5").Get();

	return 0;
}

static inline uint64 FindCallPreReplication()
{
	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("48 85 D2 0F 84 ? ? ? ? 48 8B C4 55 57 41 57 48 8D 68 A1 48 81 EC").Get();
	if (Engine_Version == 419)
		return Memcury::Scanner::FindPattern("48 85 D2 0F 84 ? ? ? ? 48 8B C4 55 57 41 54 48 8D 68 A1 48 81 EC ? ? ? ? 48 89 58 08 4C").Get();
	if (Fortnite_Version >= 2.5 && Fortnite_Version <= 3.2)
		return Memcury::Scanner::FindPattern("48 85 D2 0F 84 ? ? ? ? 56 41 56 48 83 EC 38 4C 8B F2").Get();
	if (Fortnite_Version >= 20)
		return Memcury::Scanner::FindPattern("48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 40 F6 41 58 30 48 8B EA 48 8B D9 40 B6 01").Get();

	return 0;
}

static inline uint64 FindClearAbility()
{
	auto GiveAbilityAndActivateOnce = FindGiveAbilityAndActivateOnce();

	if (!GiveAbilityAndActivateOnce)
		return 0;

	return Memcury::Scanner(GiveAbilityAndActivateOnce).ScanFor({ 0xE8 }, true, 4).RelativeOffset(1).Get();

	if (Engine_Version == 416)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 48 63 81 ? ? ? ? 33").Get();
	if (Engine_Version == 419)
		return Memcury::Scanner::FindPattern("").Get();
	if (Engine_Version == 420)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F9 C6 81 ? ? ? ? ? 8B").Get();
	if (Engine_Version == 421)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 56 57 41 57 48 83 EC 20 80 89 ? ? ? ? ? 33").Get();
	if (Engine_Version == 422)
		return Memcury::Scanner::FindPattern("").Get();
	if (Engine_Version == 423)
		return Memcury::Scanner::FindPattern("40 53 57 41 56 48 83 EC 20 80 89 ? ? ? ? ? 33").Get();
	if (Engine_Version == 500)
		return Memcury::Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 56 48 83 EC 20 80 89 ? ? ? ? ? 48 8B F2 44 8B 89 ? ? ? ? 33 D2 48 8B").Get();

	return 0;
}