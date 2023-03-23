#include "AthenaMarkerComponent.h"
#include "FortPlayerControllerAthena.h"

void UAthenaMarkerComponent::ServerAddMapMarkerHook(UAthenaMarkerComponent* MarkerComponent, FFortClientMarkerRequest MarkerRequest)
{
	auto Owner = MarkerComponent->GetOwner();

	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(Owner);

	if (!PlayerController)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState());

	if (!PlayerState)
		return;

	auto MarkerRequestPtr = &MarkerRequest;

	static auto FortWorldMarkerDataStruct = FindObject<UStruct>("/Script/FortniteGame.FortWorldMarkerData");
	static auto FortWorldMarkerDataSize = FortWorldMarkerDataStruct->GetPropertiesSize();

	auto MarkerData = Alloc<FFortWorldMarkerData>(FortWorldMarkerDataSize);

	static auto IconOffset = FindOffsetStruct("/Script/FortniteGame.MarkedActorDisplayInfo", "Icon");
	static auto WorldPositionOffset = FindOffsetStruct("/Script/FortniteGame.FortWorldMarkerData", "WorldPosition", false);
	static auto WorldPositionOffsetOffset = FindOffsetStruct("/Script/FortniteGame.FortWorldMarkerData", "WorldPositionOffset", false);

	FMarkerID MarkerID{};
	MarkerID.PlayerID = PlayerState->GetPlayerID();
	MarkerID.InstanceID = MarkerRequestPtr->GetInstanceID();

	MarkerData->GetMarkerType() = MarkerRequestPtr->GetMarkerType();
	MarkerData->GetOwner() = PlayerState;
	MarkerData->GetWorldNormal() = MarkerRequestPtr->GetWorldNormal();
	if (WorldPositionOffset != 0)
		MarkerData->GetWorldPosition() = MarkerRequestPtr->GetWorldPosition();
	if (WorldPositionOffset != 0)
		MarkerData->GetWorldPositionOffset() = MarkerRequestPtr->GetWorldPositionOffset();
	MarkerData->GetMarkerID() = MarkerID;
	MarkerData->GetMarkedActorClass().SoftObjectPtr.WeakPtr.ObjectIndex = -1;
	MarkerData->GetMarkedActorClass().SoftObjectPtr.TagAtLastTest = -1;
	MarkerData->GetMarkedActorClass().SoftObjectPtr.WeakPtr.ObjectSerialNumber = 0;
	MarkerData->GetMarkedActor().SoftObjectPtr.WeakPtr.ObjectIndex = -1;
	MarkerData->GetMarkedActor().SoftObjectPtr.TagAtLastTest = -1;
	MarkerData->GetMarkedActor().SoftObjectPtr.WeakPtr.ObjectSerialNumber = 0;
	((TSoftObjectPtr<UObject>*)(__int64(MarkerData->GetCustomDisplayInfo()) + IconOffset))->SoftObjectPtr.WeakPtr.ObjectIndex = -1;
	((TSoftObjectPtr<UObject>*)(__int64(MarkerData->GetCustomDisplayInfo()) + IconOffset))->SoftObjectPtr.TagAtLastTest = -1;
	((TSoftObjectPtr<UObject>*)(__int64(MarkerData->GetCustomDisplayInfo()) + IconOffset))->SoftObjectPtr.WeakPtr.ObjectSerialNumber = 0;

	/* if (MarkerRequest.MarkedActor)
	{
		MarkerData.MarkedActor.WeakPtr.ObjectIndex = MarkerRequest.MarkedActor->InternalIndex;
		MarkerData.MarkedActor.WeakPtr.ObjectSerialNumber = 0;

		MarkerData.MarkedActorClass.WeakPtr.ObjectIndex = MarkerRequest.MarkedActor->Class->InternalIndex;
		MarkerData.MarkedActorClass.WeakPtr.ObjectSerialNumber = 0;

		char (*WtfSkidda)(UAthenaMarkerComponent * a1, AActor * a2, FFortWorldMarkerData & MarkerData) = decltype(WtfSkidda)(__int64(GetModuleHandleW(0)) + 0x1297E00);
		(int)WtfSkidda(PlayerController->MarkerComponent, MarkerRequest.MarkedActor, MarkerData);
		// std::cout << "WtfSkidda: " << (int)WtfSkidda(PlayerController->MarkerComponent, MarkerRequest.MarkedActor, MarkerData) << '\n';
	}

	if (MarkerData.MarkerType == EFortWorldMarkerType::Item)
	{
		if (auto Pickup = Cast<AFortPickup>(MarkerRequest.MarkedActor))
		{
			MarkerData.ItemDefinition = Pickup->PrimaryPickupItemEntry.ItemDefinition;
			MarkerData.ItemCount = Pickup->PrimaryPickupItemEntry.Count;
		}
	} */

	// static void (*Idk)(UAthenaMarkerComponent* MarkerComponent, FFortWorldMarkerData MarkerData) = decltype(Idk)(__int64(GetModuleHandleW(0)) + 0x12A8990);

	static auto PlayerTeamOffset = PlayerState->GetOffset("PlayerTeam");
	auto PlayerTeam = PlayerState->Get<UObject*>(PlayerTeamOffset);

	if (!PlayerTeam)
		return;

	static auto TeamMembersOffset = PlayerTeam->GetOffset("TeamMembers");
	auto& TeamMembers = PlayerTeam->Get<TArray<AController*>>(TeamMembersOffset);

	for (int i = 0; i < TeamMembers.Num(); i++)
	{
		if (TeamMembers.at(i) == PlayerController)
			continue;

		auto CurrentTeamMemberPC = Cast<AFortPlayerControllerAthena>(TeamMembers.at(i));

		if (!CurrentTeamMemberPC)
			continue;

		auto CurrentTeamMemberMarkerComponent = CurrentTeamMemberPC->GetMarkerComponent();// (UAthenaMarkerComponent*)CurrentTeamMemberPC->GetComponentByClass(UAthenaMarkerComponent::StaticClass());

		if (!CurrentTeamMemberMarkerComponent)
			continue;

		static auto ClientAddMarkerFn = FindObject<UFunction>("/Script/FortniteGame.AthenaMarkerComponent.ClientAddMarker");
		CurrentTeamMemberMarkerComponent->ProcessEvent(ClientAddMarkerFn, MarkerData);
	}
}

void UAthenaMarkerComponent::ServerRemoveMapMarkerHook(UAthenaMarkerComponent* MarkerComponent, FMarkerID MarkerID, uint8_t CancelReason)
{
	auto Owner = MarkerComponent->GetOwner();

	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(Owner);

	if (!PlayerController)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState());

	if (!PlayerState)
		return;

	static auto PlayerTeamOffset = PlayerState->GetOffset("PlayerTeam");
	auto PlayerTeam = PlayerState->Get<UObject*>(PlayerTeamOffset);

	if (!PlayerTeam)
		return;

	static auto TeamMembersOffset = PlayerTeam->GetOffset("TeamMembers");
	auto& TeamMembers = PlayerTeam->Get<TArray<AController*>>(TeamMembersOffset);

	for (int i = 0; i < TeamMembers.Num(); i++)
	{
		if (TeamMembers.at(i) == PlayerController)
			continue;

		auto CurrentTeamMemberPC = Cast<AFortPlayerControllerAthena>(TeamMembers.at(i));

		if (!CurrentTeamMemberPC)
			continue;

		auto CurrentTeamMemberMarkerComponent = CurrentTeamMemberPC->GetMarkerComponent();

		if (!CurrentTeamMemberMarkerComponent)
			continue;

		static auto ClientCancelMarkerFn = FindObject<UFunction>("/Script/FortniteGame.AthenaMarkerComponent.ClientCancelMarker");
		CurrentTeamMemberMarkerComponent->ProcessEvent(ClientCancelMarkerFn, &MarkerID);
	}
}