#pragma once

#include "reboot.h"

enum class EFortQuickBars : uint8_t // WRONG!!! It has a creative quickbar, do not use unless you know what you are doing.
{
	Primary = 0,
	Secondary = 1,
	Max_None = 2,
	EFortQuickBars_MAX = 3
};

struct FQuickBarSlot
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.QuickBarSlot");
		return Struct;
	}

	static int GetStructSize()
	{
		return GetStruct()->GetPropertiesSize();
	}

	TArray<FGuid>& GetItems()
	{
		static auto ItemsOffset = FindOffsetStruct("/Script/FortniteGame.QuickBarSlot", "Items");
		return *(TArray<FGuid>*)(__int64(this) + ItemsOffset);
	}
};

struct FQuickBar
{
	TArray<FQuickBarSlot>& GetSlots()
	{
		static auto SlotsOffset = FindOffsetStruct("/Script/FortniteGame.QuickBar", "Slots");
		return *(TArray<FQuickBarSlot>*)(__int64(this) + SlotsOffset);
	}
};

class AFortQuickBars : public AActor
{
public:
	void ServerRemoveItemInternal(const FGuid& Item, bool bFindReplacement, bool bForce)
	{
		static auto ServerRemoveItemInternalFn = FindObject<UFunction>("/Script/FortniteGame.FortQuickBars.ServerRemoveItemInternal");

		struct
		{
			FGuid                                       Item;                                                     // (Parm, IsPlainOldData)
			bool                                               bFindReplacement;                                         // (Parm, ZeroConstructor, IsPlainOldData)
			bool                                               bForce;                                                   // (Parm, ZeroConstructor, IsPlainOldData)
		} AFortQuickBars_ServerRemoveItemInternal_Params{ Item, bFindReplacement, bForce };
		ProcessEvent(ServerRemoveItemInternalFn, &AFortQuickBars_ServerRemoveItemInternal_Params);
	}

	void EmptySlot(EFortQuickBars InQuickBar, int SlotIndex)
	{
		static auto EmptySlotFn = FindObject<UFunction>("/Script/FortniteGame.FortQuickBars.EmptySlot");
		struct
		{
			EFortQuickBars                                     InQuickBar;                                               // (Parm, ZeroConstructor, IsPlainOldData)
			int                                                SlotIndex;                                                // (Parm, ZeroConstructor, IsPlainOldData)
		} AFortQuickBars_EmptySlot_Params{ InQuickBar, SlotIndex };

		this->ProcessEvent(EmptySlotFn, &AFortQuickBars_EmptySlot_Params);
	}

	int GetSlotIndex(const FGuid& Item, EFortQuickBars QuickBars = EFortQuickBars::Max_None)
	{
		static auto PrimaryQuickBarOffset = GetOffset("PrimaryQuickBar");
		static auto SecondaryQuickBarOffset = GetOffset("SecondaryQuickBar");
		auto PrimaryQuickBar = GetPtr<FQuickBar>(PrimaryQuickBarOffset);
		auto SecondaryQuickBar = GetPtr<FQuickBar>(SecondaryQuickBarOffset);

		if (QuickBars == EFortQuickBars::Primary || QuickBars == EFortQuickBars::Max_None)
		{
			auto& PrimaryQuickBarSlots = PrimaryQuickBar->GetSlots();

			for (int i = 0; i < PrimaryQuickBarSlots.Num(); ++i)
			{
				auto Slot = PrimaryQuickBarSlots.AtPtr(i, FQuickBarSlot::GetStructSize());

				auto& SlotItems = Slot->GetItems();

				for (int z = 0; z < SlotItems.Num(); z++)
				{
					auto& CurrentItem = SlotItems.at(z);

					if (CurrentItem == Item)
						return i;
				}
			}
		}

		if (QuickBars == EFortQuickBars::Secondary || QuickBars == EFortQuickBars::Max_None)
		{
			auto& SecondaryQuickBarSlots = SecondaryQuickBar->GetSlots();

			for (int i = 0; i < SecondaryQuickBarSlots.Num(); ++i)
			{
				auto Slot = SecondaryQuickBarSlots.AtPtr(i, FQuickBarSlot::GetStructSize());

				auto& SlotItems = Slot->GetItems();

				for (int z = 0; z < SlotItems.Num(); z++)
				{
					auto& CurrentItem = SlotItems.at(z);

					if (CurrentItem == Item)
						return i;
				}
			}
		}

		return -1;
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortQuickBars");
		return Class;
	}
};