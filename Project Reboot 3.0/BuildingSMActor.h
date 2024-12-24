#pragma once

#include "BuildingActor.h"
#include "PlayerState.h"

enum class EFortResourceType : uint8_t
{
	Wood = 0,
	Stone = 1,
	Metal = 2,
	Permanite = 3,
	None = 4,
	EFortResourceType_MAX = 5
};

class ABuildingSMActor : public ABuildingActor
{
public:
	bool IsPlayerPlaced()
	{
		static auto bPlayerPlacedOffset = GetOffset("bPlayerPlaced");
		static auto bPlayerPlacedFieldMask = GetFieldMask(this->GetProperty("bPlayerPlaced"));
		return ReadBitfieldValue(bPlayerPlacedOffset, bPlayerPlacedFieldMask);
	}

	void SetPlayerPlaced(bool NewValue)
	{
		static auto bPlayerPlacedOffset = GetOffset("bPlayerPlaced");
		static auto bPlayerPlacedFieldMask = GetFieldMask(this->GetProperty("bPlayerPlaced"));
		this->SetBitfieldValue(bPlayerPlacedOffset, bPlayerPlacedFieldMask, NewValue);
	}

	APlayerState*& GetEditingPlayer()
	{
		static auto EditingPlayerOffset = GetOffset("EditingPlayer");
		return Get<APlayerState*>(EditingPlayerOffset);
	}

	int& GetCurrentBuildingLevel()
	{
		static auto CurrentBuildingLevelOffset = GetOffset("CurrentBuildingLevel");
		return Get<int>(CurrentBuildingLevelOffset);
	}

	EFortResourceType& GetResourceType()
	{
		static auto ResourceTypeOffset = GetOffset("ResourceType");
		return Get<EFortResourceType>(ResourceTypeOffset);
	}

	void SetEditingPlayer(APlayerState* NewEditingPlayer) // actually AFortPlayerStateZone
	{
		if (// AActor::HasAuthority() &&
			(!GetEditingPlayer() || !NewEditingPlayer)
			)
		{
			SetNetDormancy((ENetDormancy)(2 - (NewEditingPlayer != 0)));
			// they do something here
			GetEditingPlayer() = NewEditingPlayer;
		}
	}

	static UClass* StaticClass();
};