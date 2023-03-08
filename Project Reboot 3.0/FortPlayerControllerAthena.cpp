#include "FortPlayerControllerAthena.h"
#include "FortPlayerPawn.h"
#include "FortKismetLibrary.h"

#include "SoftObjectPtr.h"

void ApplyCID(AFortPlayerPawn* Pawn, UObject* CID)
{
	if (!CID)
		return;

	static auto HeroDefinitionOffset = CID->GetOffset("HeroDefinition");
	auto HeroDefinition = CID->Get(HeroDefinitionOffset);

	using UFortHeroSpecialization = UObject;

	static auto SpecializationsOffset = HeroDefinition->GetOffset("Specializations");
	auto& Specializations = HeroDefinition->Get<TArray<TSoftObjectPtr<UFortHeroSpecialization>>>(SpecializationsOffset);

	auto PlayerState = Pawn->GetPlayerState();

	for (int i = 0; i < Specializations.Num(); i++)
	{
		auto& SpecializationSoft = Specializations.at(i);

		auto Specialization = SpecializationSoft.Get();

		if (Specialization)
		{
			static auto Specialization_CharacterPartsOffset = Specialization->GetOffset("CharacterParts");
			auto& CharacterParts = Specialization->Get<TArray<TSoftObjectPtr<UObject>>>(Specialization_CharacterPartsOffset);

			bool aa;

			TArray<UObject*> CharacterPartsaa;

			for (int z = 0; z < CharacterParts.Num(); z++)
			{
				auto& CharacterPartSoft = CharacterParts.at(z);
				auto CharacterPart = CharacterPartSoft.Get();

				CharacterPartsaa.Add(CharacterPart);

				continue;
			}

			UFortKismetLibrary::ApplyCharacterCosmetics(GetWorld(), CharacterPartsaa, PlayerState, &aa);
			CharacterPartsaa.Free();
		}
	}
}

void AFortPlayerControllerAthena::ServerAcknowledgePossessionHook(APlayerController* Controller, APawn* Pawn)
{
	static auto AcknowledgedPawnOffset = Controller->GetOffset("AcknowledgedPawn");
	Controller->Get<APawn*>(AcknowledgedPawnOffset) = Pawn;

	auto ControllerAsFort = Cast<AFortPlayerController>(Controller);
	auto PawnAsFort = Cast<AFortPlayerPawn>(Pawn);
	auto PlayerStateAsFort = Cast<AFortPlayerState>(Pawn->GetPlayerState());

	if (!PawnAsFort)
		return;

	static auto UpdatePlayerCustomCharacterPartsVisualizationFn = FindObject<UFunction>("/Script/FortniteGame.FortKismetLibrary.UpdatePlayerCustomCharacterPartsVisualization");

	if (!UpdatePlayerCustomCharacterPartsVisualizationFn)
	{
		static auto CosmeticLoadoutPCOffset = Controller->GetOffset("CosmeticLoadoutPC");
		auto CosmeticLoadout = ControllerAsFort->GetCosmeticLoadout();

		ApplyCID(PawnAsFort, CosmeticLoadout->GetCharacter());

		return;
	}

	if (!PlayerStateAsFort)
		return;
	
	UFortKismetLibrary::StaticClass()->ProcessEvent(UpdatePlayerCustomCharacterPartsVisualizationFn, &PlayerStateAsFort);
}

void AFortPlayerControllerAthena::GetPlayerViewPointHook(AFortPlayerControllerAthena* PlayerController, FVector& Location, FRotator& Rotation)
{
	if (auto MyFortPawn = PlayerController->GetMyFortPawn())
	{
		Location = MyFortPawn->GetActorLocation();
		Rotation = PlayerController->GetControlRotation();
		return;
	}

	return AFortPlayerControllerAthena::GetPlayerViewPointOriginal(PlayerController, Location, Rotation);
}