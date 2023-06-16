#include "finder.h"

#include "reboot.h"
#include "FortPlayerControllerAthena.h"

uint64 FindGetPlayerViewpoint()
{
	// We find FailedToSpawnPawn and then go back on VFT by 1.

	uint64 FailedToSpawnPawnAddr = 0;

	auto FailedToSpawnPawnStrRefAddr = Memcury::Scanner::FindStringRef(L"%s failed to spawn a pawn", true, 0, Fortnite_Version >= 18).Get();

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(FailedToSpawnPawnStrRefAddr - i) == 0x40 && *(uint8_t*)(uint8_t*)(FailedToSpawnPawnStrRefAddr - i + 1) == 0x53)
		{
			FailedToSpawnPawnAddr = FailedToSpawnPawnStrRefAddr - i;
			break;
		}
	}

	if (!FailedToSpawnPawnAddr)
	{
		LOG_ERROR(LogFinder, "Failed to find FailedToSpawnPawn! Report to Milxnor immediately.");
		return 0;
	}

	static auto FortPlayerControllerAthenaDefault = FindObject<AFortPlayerControllerAthena>(L"/Script/FortniteGame.Default__FortPlayerControllerAthena"); // FindObject<UClass>(L"/Game/Athena/Athena_PlayerController.Default__Athena_PlayerController_C");
	void** const PlayerControllerVFT = FortPlayerControllerAthenaDefault->VFTable;

	int FailedToSpawnPawnIdx = 0;

	for (int i = 0; i < 500; i++)
	{
		if (PlayerControllerVFT[i] == (void*)FailedToSpawnPawnAddr)
		{
			FailedToSpawnPawnIdx = i;
			break;
		}
	}

	if (FailedToSpawnPawnIdx == 0)
	{
		LOG_ERROR(LogFinder, "Failed to find FailedToSpawnPawn in virtual function table! Report to Milxnor immediately.");
		return 0;
	}

	return __int64(PlayerControllerVFT[FailedToSpawnPawnIdx - 1]);

	// LITERALLY KMS BRO

	if (Engine_Version == 420 && Fortnite_Version < 4.5)
	{
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 41 56 41 57 48 8B EC 48 83 EC 50").Get(); // idk why finder doesnt work and cba to debug
	}

	auto Addrr = Memcury::Scanner::FindStringRef(L"APlayerController::GetPlayerViewPoint: out_Location, ViewTarget=%s").Get();

	LOG_INFO(LogDev, "GetPlayerViewpoint StringRef: 0x{:x}", __int64(Addrr) - __int64(GetModuleHandleW(0)));

	for (int i = 0; i < Fortnite_Version >= 20 ? 2000 : 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x55)
		{
			LOG_INFO(LogDev, "GetPlayerViewpoint1!");
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
		{
			LOG_INFO(LogDev, "GetPlayerViewpoint2!");
			return Addrr - i;
		}

		if (Fortnite_Version == 7.20 && *(uint8_t*)(uint8_t*)(Addrr - i) == 0xC3) // hmm scuffed lmfao
		{
			LOG_INFO(LogDev, "Hit C3!");
			break;
		}
	}

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x74)
		{
			LOG_INFO(LogDev, "GetPlayerViewpoint3!");
			return Addrr - i;
		}
	}

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			LOG_INFO(LogDev, "GetPlayerViewpoint4!");
			return Addrr - i;
		}
	}

	return 0;
}