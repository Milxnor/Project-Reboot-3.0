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

		if (*(uint8_t*)(uint8_t*)(FailedToSpawnPawnStrRefAddr - i) == 0x48 && *(uint8_t*)(uint8_t*)(FailedToSpawnPawnStrRefAddr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(FailedToSpawnPawnStrRefAddr - i + 2) == 0x5C)
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
}

uint64 ApplyGameSessionPatch()
{
	auto GamePhaseStepStringAddr = Memcury::Scanner::FindStringRef(L"Gamephase Step: %s", false).Get();

	uint64 BeginningOfGamePhaseStepFn = 0;
	uint8_t* ByteToPatch = 0;

	if (!GamePhaseStepStringAddr)
	{
		LOG_WARN(LogFinder, "Unable to find GamePhaseStepString!");
		// return 0;

		BeginningOfGamePhaseStepFn = Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 E8 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? E8").Get(); // not actually the func but its fine

		if (!BeginningOfGamePhaseStepFn)
		{
			LOG_WARN(LogFinder, "Unable to find fallback sig for gamephase step! Report to Milxnor immediately.");
			return 0;
		}
	}

	if (!BeginningOfGamePhaseStepFn && !ByteToPatch)
	{
		for (int i = 0; i < 3000; i++)
		{
			if (*(uint8_t*)(uint8_t*)(GamePhaseStepStringAddr - i) == 0x40 && *(uint8_t*)(uint8_t*)(GamePhaseStepStringAddr - i + 1) == 0x55)
			{
				BeginningOfGamePhaseStepFn = GamePhaseStepStringAddr - i;
				break;
			}

			if (*(uint8_t*)(uint8_t*)(GamePhaseStepStringAddr - i) == 0x48 && *(uint8_t*)(uint8_t*)(GamePhaseStepStringAddr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(GamePhaseStepStringAddr - i + 2) == 0x5C)
			{
				BeginningOfGamePhaseStepFn = GamePhaseStepStringAddr - i;
				break;
			}

			if (*(uint8_t*)(uint8_t*)(GamePhaseStepStringAddr - i) == 0x48 && *(uint8_t*)(uint8_t*)(GamePhaseStepStringAddr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(GamePhaseStepStringAddr - i + 2) == 0xC4)
			{
				BeginningOfGamePhaseStepFn = GamePhaseStepStringAddr - i;
				break;
			}
		}
	}
	 
	if (!BeginningOfGamePhaseStepFn && !ByteToPatch)
	{
		LOG_WARN(LogFinder, "Unable to find beginning of GamePhaseStep! Report to Milxnor immediately.");
		return 0;
	}

	if (!ByteToPatch)
	{
		for (int i = 0; i < 500; i++)
		{
			if (*(uint8_t*)(uint8_t*)(BeginningOfGamePhaseStepFn + i) == 0x0F && *(uint8_t*)(uint8_t*)(BeginningOfGamePhaseStepFn + i + 1) == 0x84)
			{
				ByteToPatch = (uint8_t*)(uint8_t*)(BeginningOfGamePhaseStepFn + i + 1);
				break;
			}
		}
	}

	if (!ByteToPatch)
	{
		LOG_WARN(LogFinder, "Unable to find byte to patch for GamePhaseStep!");
		return 0;
	}

	DWORD dwProtection;
	VirtualProtect((PVOID)ByteToPatch, 1, PAGE_EXECUTE_READWRITE, &dwProtection);

	*ByteToPatch = 0x85;

	DWORD dwTemp;
	VirtualProtect((PVOID)ByteToPatch, 1, dwProtection, &dwTemp);

	return 0;
}