#include "AthenaMarkerComponent.h"
#include "FortPlayerControllerAthena.h"
#include "Text.h"
#include "KismetTextLibrary.h"

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

	bool useRealloc = false;
	auto MarkerData = Alloc<FFortWorldMarkerData>(FFortWorldMarkerData::GetStructSize(), useRealloc);

	static auto IconOffset = FindOffsetStruct("/Script/FortniteGame.MarkedActorDisplayInfo", "Icon");
	static auto DisplayNameOffset = FindOffsetStruct("/Script/FortniteGame.MarkedActorDisplayInfo", "DisplayName");
	static auto WorldPositionOffset = FindOffsetStruct("/Script/FortniteGame.FortWorldMarkerData", "WorldPosition", false);
	static auto bIncludeSquadOffset = FindOffsetStruct("/Script/FortniteGame.FortWorldMarkerData", "bIncludeSquad", false);
	static auto WorldPositionOffsetOffset = FindOffsetStruct("/Script/FortniteGame.FortWorldMarkerData", "WorldPositionOffset", false);

	FMarkerID MarkerID{};
	MarkerID.PlayerID = PlayerState->GetPlayerID();
	MarkerID.InstanceID = MarkerRequestPtr->GetInstanceID();

	MarkerData->GetMarkerType() = MarkerRequestPtr->GetMarkerType();
	MarkerData->GetOwner() = PlayerState;
	MarkerData->GetWorldNormal() = MarkerRequestPtr->GetWorldNormal();

	if (WorldPositionOffset != -1)
		MarkerData->GetWorldPosition() = MarkerRequestPtr->GetWorldPosition();
	if (WorldPositionOffset != -1)
		MarkerData->GetWorldPositionOffset() = MarkerRequestPtr->GetWorldPositionOffset();
	if (bIncludeSquadOffset != -1)
		MarkerData->DoesIncludeSquad() = MarkerRequestPtr->DoesIncludeSquad();

	MarkerData->GetMarkerID() = MarkerID;
	MarkerData->GetMarkedActorClass().SoftObjectPtr.WeakPtr.ObjectIndex = -1;
	MarkerData->GetMarkedActorClass().SoftObjectPtr.TagAtLastTest = 0;
	MarkerData->GetMarkedActorClass().SoftObjectPtr.WeakPtr.ObjectSerialNumber = 0;
	MarkerData->GetMarkedActor().SoftObjectPtr.WeakPtr.ObjectIndex = -1;
	MarkerData->GetMarkedActor().SoftObjectPtr.TagAtLastTest = 0;
	MarkerData->GetMarkedActor().SoftObjectPtr.WeakPtr.ObjectSerialNumber = 0;
	((TSoftObjectPtr<UObject>*)(__int64(MarkerData->GetCustomDisplayInfo()) + IconOffset))->SoftObjectPtr.WeakPtr.ObjectIndex = -1;
	((TSoftObjectPtr<UObject>*)(__int64(MarkerData->GetCustomDisplayInfo()) + IconOffset))->SoftObjectPtr.TagAtLastTest = 0;
	((TSoftObjectPtr<UObject>*)(__int64(MarkerData->GetCustomDisplayInfo()) + IconOffset))->SoftObjectPtr.WeakPtr.ObjectSerialNumber = 0;
	*(FText*)(__int64(MarkerData->GetCustomDisplayInfo()) + DisplayNameOffset) = UKismetTextLibrary::Conv_StringToText(L"");

	static auto PlayerTeamOffset = PlayerState->GetOffset("PlayerTeam");
	auto PlayerTeam = PlayerState->Get<UObject*>(PlayerTeamOffset);

	if (!PlayerTeam)
		return;

	static auto TeamMembersOffset = PlayerTeam->GetOffset("TeamMembers");
	auto& TeamMembers = PlayerTeam->Get<TArray<AController*>>(TeamMembersOffset);

	for (int i = 0; i < TeamMembers.Num(); ++i)
	{
		if (TeamMembers.at(i) == PlayerController)
			continue;

		auto CurrentTeamMemberPC = Cast<AFortPlayerControllerAthena>(TeamMembers.at(i));

		if (!CurrentTeamMemberPC)
			continue;

		auto CurrentTeamMemberMarkerComponent = CurrentTeamMemberPC->GetMarkerComponent();// (UAthenaMarkerComponent*)CurrentTeamMemberPC->GetComponentByClass(UAthenaMarkerComponent::StaticClass());

		if (!CurrentTeamMemberMarkerComponent)
			continue;

		static auto ClientAddMarkerFn = FindObject<UFunction>(L"/Script/FortniteGame.AthenaMarkerComponent.ClientAddMarker");

		if (ClientAddMarkerFn)
		{
			CurrentTeamMemberMarkerComponent->ProcessEvent(ClientAddMarkerFn, MarkerData);
		}
		else
		{
			// We are assuming it is not the TArray and it is FFortWorldMarkerContainer
			
			static auto MarkerStreamOffset = CurrentTeamMemberMarkerComponent->GetOffset("MarkerStream");
			auto MarkerStream = CurrentTeamMemberMarkerComponent->GetPtr<FFastArraySerializer>(MarkerStreamOffset);

			static auto MarkersOffset = FindOffsetStruct("/Script/FortniteGame.FortWorldMarkerContainer", "Markers");

			if (MarkersOffset != -1)
			{
				// We are assuming it is a FFastArraySerializerItem
				((FFastArraySerializerItem*)MarkerData)->MostRecentArrayReplicationKey = -1;
				((FFastArraySerializerItem*)MarkerData)->ReplicationID = -1;
				((FFastArraySerializerItem*)MarkerData)->ReplicationKey = -1;

				auto Markers = (TArray<FFortWorldMarkerData>*)(__int64(MarkerStream) + MarkersOffset);
				Markers->AddPtr(MarkerData, FFortWorldMarkerData::GetStructSize());
				MarkerStream->MarkArrayDirty();
			}
		}
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

	for (int i = 0; i < TeamMembers.Num(); ++i)
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

		if (ClientCancelMarkerFn)
		{
			CurrentTeamMemberMarkerComponent->ProcessEvent(ClientCancelMarkerFn, &MarkerID);
		}
		else
		{
			// UnmarkActorOnClient ?
		}
	}
}