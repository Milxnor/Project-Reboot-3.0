#include "FortAthenaMapInfo.h"
#include "GameplayStatics.h"
#include "FortAthenaSupplyDrop.h"
#include "FortGameModeAthena.h"
#include "Vector2D.h"

FVector2D GenerateRandomVector2D(float Radius)
{
	float v3;
	float v4;

	do
	{
		v3 = (float)((float)rand() * 0.000061037019) - 1.0;
		v4 = (float)((float)rand() * 0.000061037019) - 1.0;
	} while ((float)((float)(v4 * v4) + (float)(v3 * v3)) > 1.0);

	return FVector2D(v3 * Radius, v4 * Radius);
}

FVector AFortAthenaMapInfo::PickSupplyDropLocation(FVector Center, float Radius)
{
	static FVector* (*PickSupplyDropLocationOriginal)(AFortAthenaMapInfo* MapInfo, FVector* outLocation, __int64 Center, float Radius) = decltype(PickSupplyDropLocationOriginal)(Addresses::PickSupplyDropLocation);

	if (!PickSupplyDropLocationOriginal)
		return FVector(0, 0, 0);

	// LOG_INFO(LogDev, "GetAircraftDropVolume: {}", __int64(GetAircraftDropVolume()));

	FVector Out = FVector(0, 0, 0);
	auto ahh = PickSupplyDropLocationOriginal(this, &Out, __int64(&Center), Radius);
	return Out;
}

void AFortAthenaMapInfo::SpawnLlamas()
{
	if (!GetLlamaClass())
	{
		// LOG_INFO(LogDev, "No Llama Class, is this intended?");
		return;
	}

	int AmountOfLlamasSpawned = 0;
	auto AmountOfLlamasToSpawn = CalcuateCurveMinAndMax(GetLlamaQuantityMin(), GetLlamaQuantityMax(), 1);

	LOG_INFO(LogDev, "Attempting to spawn {} llamas.", AmountOfLlamasToSpawn);

	for (int i = 0; i < AmountOfLlamasToSpawn; i++)
	{
		int Radius = 100000;
		FVector Location = PickSupplyDropLocation(FVector(1, 1, 10000), Radius);

		// LOG_INFO(LogDev, "Initial Llama at {} {} {}", Location.X, Location.Y, Location.Z);

		if (Location.CompareVectors(FVector(0, 0, 0)))
			continue;

		FRotator RandomYawRotator = FRotator();
		RandomYawRotator.Yaw = (float)rand() * 0.010986663f;

		FTransform InitialSpawnTransform;
		InitialSpawnTransform.Translation = Location;
		InitialSpawnTransform.Rotation = RandomYawRotator.Quaternion();
		InitialSpawnTransform.Scale3D = FVector(1, 1, 1);

		auto LlamaStart = GetWorld()->SpawnActor<AFortAthenaSupplyDrop>(GetLlamaClass(), InitialSpawnTransform, 
			CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, true));
	
		// LOG_INFO(LogDev, "LlamaStart: {}", __int64(LlamaStart));

		if (!LlamaStart)
			continue;
		
		auto GroundLocation = LlamaStart->FindGroundLocationAt(InitialSpawnTransform.Translation);

		FTransform FinalSpawnTransform = InitialSpawnTransform;
		FinalSpawnTransform.Translation = GroundLocation;

		LOG_INFO(LogDev, "Spawning Llama at {} {} {}", GroundLocation.X, GroundLocation.Y, GroundLocation.Z);

		UGameplayStatics::FinishSpawningActor(LlamaStart, FinalSpawnTransform);
		AmountOfLlamasSpawned++;
	}

	LOG_INFO(LogGame, "Spawned {} llamas.", AmountOfLlamasSpawned);
}