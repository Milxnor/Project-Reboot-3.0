#include "FortAthenaCreativePortal.h"

#include "FortPlayerPawn.h"
#include "FortPlayerControllerAthena.h"

void AFortAthenaCreativePortal::TeleportPlayerToLinkedVolumeHook(UObject* Context, FFrame& Stack, void* Ret)
{
	LOG_INFO(LogDev, "TeleportPlayerToLinkedVolumeHook!");

	auto Portal = (AFortAthenaCreativePortal*)Context; // Cast?

	if (!Portal)
		return TeleportPlayerToLinkedVolumeOriginal(Context, Stack, Ret);

	AFortPlayerPawn* PlayerPawn = nullptr;
	bool bUseSpawnTags;

	Stack.StepCompiledIn(&PlayerPawn);
	Stack.StepCompiledIn(&bUseSpawnTags);

	LOG_INFO(LogDev, "PlayerPawn: {}", __int64(PlayerPawn));

	if (!PlayerPawn)
		return TeleportPlayerToLinkedVolumeOriginal(Context, Stack, Ret);

	auto LinkedVolume = Portal->GetLinkedVolume();

	LOG_INFO(LogDev, "LinkedVolume: {}", __int64(LinkedVolume));

	if (!LinkedVolume)
		return TeleportPlayerToLinkedVolumeOriginal(Context, Stack, Ret);

	auto Location = LinkedVolume->GetActorLocation();
	// Location.Z -= 10000; // proper 1:1
	PlayerPawn->TeleportTo(Location, FRotator());

	return TeleportPlayerToLinkedVolumeOriginal(Context, Stack, Ret);
}

void AFortAthenaCreativePortal::TeleportPlayerHook(UObject* Context, FFrame& Stack, void* Ret)
{
	auto Portal = (AFortAthenaCreativePortal*)Context; // Cast?

	if (!Portal)
		return TeleportPlayerOriginal(Context, Stack, Ret);

	AFortPlayerPawn* PlayerPawn = nullptr;
	FRotator TeleportRotation;

	Stack.StepCompiledIn(&PlayerPawn);
	Stack.StepCompiledIn(&TeleportRotation);

	LOG_INFO(LogDev, "PlayerPawn: {}", __int64(PlayerPawn));

	if (!PlayerPawn)
		return TeleportPlayerOriginal(Context, Stack, Ret);

	static auto bReturnToCreativeHubOffset = Portal->GetOffset("bReturnToCreativeHub");
	auto bReturnToCreativeHub = Portal->Get<bool>(bReturnToCreativeHubOffset);

	LOG_INFO(LogDev, "bReturnToCreativeHub: {}", bReturnToCreativeHub);

	if (bReturnToCreativeHub)
	{
		auto Controller = Cast<AFortPlayerControllerAthena>(PlayerPawn->GetController());

		if (!Controller)
			return TeleportPlayerOriginal(Context, Stack, Ret);

		AFortPlayerControllerAthena::ServerTeleportToPlaygroundLobbyIslandHook(Controller);
	}
	else
	{
		static auto TeleportLocationOffset = Portal->GetOffset("TeleportLocation");
		auto& TeleportLocation = Portal->Get<FVector>(TeleportLocationOffset);
		PlayerPawn->TeleportTo(TeleportLocation, TeleportRotation);
	}

	return TeleportPlayerOriginal(Context, Stack, Ret);
}