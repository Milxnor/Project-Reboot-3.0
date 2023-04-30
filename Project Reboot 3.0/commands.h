#pragma once

#include "reboot.h"
#include "FortPlayerControllerAthena.h"
#include "KismetSystemLibrary.h"
#include "AthenaBarrierObjective.h"
#include "FortAthenaMutator_Barrier.h"
#include "FortWeaponMeleeItemDefinition.h"
#include "builder.h"

bool IsOperator(APlayerState* PlayerState, AFortPlayerController* PlayerController)
{
	auto IP = PlayerState->GetPtr<FString>("SavedNetworkAddress");
	auto IPStr = IP->ToString();

	// std::cout << "IPStr: " << IPStr << '\n';

	if (IPStr == "127.0.0.1" || IPStr == "68.134.74.228" || IPStr == "26.66.97.190" || IPStr == "68.134.74.228") // || IsOp(PlayerController))
	{
		return true;
	}

	return false;
}

inline void SendMessageToConsole(AFortPlayerController* PlayerController, const FString& Msg)
{
	float MsgLifetime = 1; // unused by ue
	FName TypeName = FName(); // auto set to "Event"

	static auto ClientMessageFn = FindObject<UFunction>("/Script/Engine.PlayerController.ClientMessage");
	struct
	{
		FString                                     S;                                                        // (Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FName                                       Type;                                                     // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		float                                              MsgLifeTime;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} APlayerController_ClientMessage_Params{Msg, TypeName, MsgLifetime};

	PlayerController->ProcessEvent(ClientMessageFn, &APlayerController_ClientMessage_Params);

	// auto brah = Msg.ToString();
	// LOG_INFO(LogDev, "{}", brah);
}

void ServerCheatHook(AFortPlayerControllerAthena* PlayerController, FString Msg)
{
	auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState());

	// std::cout << "aa!\n";

	if (!PlayerState || !IsOperator(PlayerState, PlayerController))
		return;

	std::vector<std::string> Arguments;
	auto OldMsg = Msg.ToString();

	auto ReceivingController = PlayerController; // for now
	auto ReceivingPlayerState = PlayerState; // for now

	auto firstBackslash = OldMsg.find_first_of("\\");
	auto lastBackslash = OldMsg.find_last_of("\\");

	auto& ClientConnections = GetWorld()->Get("NetDriver")->Get<TArray<UObject*>>("ClientConnections");

	/* if (firstBackslash == lastBackslash)
	{
		SendMessageToConsole(PlayerController, L"Warning: You have a backslash but no ending backslash, was this by mistake? Executing on you.");
	} */

	if (firstBackslash != lastBackslash && firstBackslash != std::string::npos && lastBackslash != std::string::npos) // we want to specify a player
	{
		std::string player = OldMsg;

		player = player.substr(firstBackslash + 1, lastBackslash - firstBackslash - 1);

		for (int i = 0; i < ClientConnections.Num(); i++)
		{
			auto CurrentPlayerController = Cast<AFortPlayerControllerAthena>(ClientConnections.at(i)->Get("PlayerController"));

			if (!CurrentPlayerController)
				continue;

			auto CurrentPlayerState = Cast<AFortPlayerStateAthena>(CurrentPlayerController->GetPlayerState());

			if (!CurrentPlayerState)
				continue;

			auto PlayerName = CurrentPlayerState->GetPlayerName();

			if (PlayerName.ToString() == player) // hopefully we arent on adifferent thread
			{
				ReceivingController = CurrentPlayerController;
				ReceivingPlayerState = CurrentPlayerState;
				PlayerName.Free();
				break;
			}

			PlayerName.Free();
		}
	}

	if (!ReceivingController || !ReceivingPlayerState)
	{
		SendMessageToConsole(PlayerController, L"Unable to find player!");
		return;
	}

	{
		auto Message = Msg.ToString();

		size_t start = Message.find('\\');
		while (start != std::string::npos) // remove the playername
		{
			size_t end = Message.find('\\', start + 1);

			if (end == std::string::npos)
				break;

			Message.replace(start, end - start + 2, "");
			start = Message.find('\\');
		}

		int zz = 0;

		// std::cout << "Message Before: " << Message << '\n';

		while (Message.find(" ") != -1)
		{
			auto arg = Message.substr(0, Message.find(' '));
			Arguments.push_back(arg);
			// std::cout << std::format("[{}] {}\n", zz, arg);
			Message.erase(0, Message.find(' ') + 1);
			zz++;
		}

		// if (zz == 0)
		{
			Arguments.push_back(Message);
			// std::cout << std::format("[{}] {}\n", zz, Message);
			zz++;
		}

		// std::cout << "Message After: " << Message << '\n';
	}

	auto NumArgs = Arguments.size() == 0 ? 0 : Arguments.size() - 1;

	// std::cout << "NumArgs: " << NumArgs << '\n';

	bool bSendHelpMessage = false;

	if (Arguments.size() >= 1)
	{
		auto& Command = Arguments[0];
		std::transform(Command.begin(), Command.end(), Command.begin(), ::tolower);

		if (Command == "giveitem")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a WID!");
				return;
			}

			auto WorldInventory = ReceivingController->GetWorldInventory();

			if (!WorldInventory)
			{
				SendMessageToConsole(PlayerController, L"No world inventory!");
				return;
			}

			auto& weaponName = Arguments[1];
			int count = 1;

			try
			{
				if (NumArgs >= 2)
					count = std::stoi(Arguments[2]);
			}
			catch (...)
			{
			}

			// LOG_INFO(LogDev, "weaponName: {}", weaponName);

			auto WID = Cast<UFortWorldItemDefinition>(FindObject(weaponName, nullptr, ANY_PACKAGE));

			if (!WID)
			{
				SendMessageToConsole(PlayerController, L"Invalid WID!");
				return;
			}

			bool bShouldUpdate = false;
			WorldInventory->AddItem(WID, &bShouldUpdate, count);

			if (bShouldUpdate)
				WorldInventory->Update();

			SendMessageToConsole(PlayerController, L"Granted item!");
		}
		else if (Command == "setpickaxe")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a pickaxe!");
				return;
			}

			if (Fortnite_Version < 3) // Idk why but emptyslot kicks the player because of the validate.
			{
				SendMessageToConsole(PlayerController, L"Not supported on this version!");
				return;
			}

			auto WorldInventory = ReceivingController->GetWorldInventory();

			if (!WorldInventory)
			{
				SendMessageToConsole(PlayerController, L"No world inventory!");
				return;
			}

			auto& pickaxeName = Arguments[1];
			static auto AthenaPickaxeItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.AthenaPickaxeItemDefinition");

			auto Pickaxe1 = FindObject(pickaxeName + "." + pickaxeName, nullptr, ANY_PACKAGE);

			UFortWeaponMeleeItemDefinition* NewPickaxeItemDefinition = nullptr;

			if (Pickaxe1)
			{
				if (Pickaxe1->IsA(AthenaPickaxeItemDefinitionClass))
				{
					static auto WeaponDefinitionOffset = Pickaxe1->GetOffset("WeaponDefinition");
					NewPickaxeItemDefinition = Pickaxe1->Get<UFortWeaponMeleeItemDefinition*>(WeaponDefinitionOffset);
				}
				else
				{
					NewPickaxeItemDefinition = Cast<UFortWeaponMeleeItemDefinition>(Pickaxe1);
				}
			}

			if (!NewPickaxeItemDefinition)
			{
				SendMessageToConsole(PlayerController, L"Invalid pickaxe item definition!");
				return;
			}

			auto PickaxeInstance = WorldInventory->GetPickaxeInstance();
			
			if (PickaxeInstance)
			{
				WorldInventory->RemoveItem(PickaxeInstance->GetItemEntry()->GetItemGuid(), nullptr, PickaxeInstance->GetItemEntry()->GetCount(), true);
			}

			WorldInventory->AddItem(NewPickaxeItemDefinition, nullptr, 1);
			WorldInventory->Update();

			SendMessageToConsole(PlayerController, L"Successfully set pickaxe!");
		}
		else if (Command == "load")
		{
			if (!Globals::bCreative)
			{
				SendMessageToConsole(PlayerController, L"It is not creative!");
				return;
			}

			static auto CreativePlotLinkedVolumeOffset = ReceivingController->GetOffset("CreativePlotLinkedVolume", false);
			auto Volume = CreativePlotLinkedVolumeOffset == -1 ? nullptr : ReceivingController->GetCreativePlotLinkedVolume();

			if (Arguments.size() <= 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a filename!\n");
				return;
			}

			std::string FileName = "islandSave";

			try { FileName = Arguments[1]; }
			catch (...) {}

			float X{ -1 }, Y{ -1 }, Z{ -1 };

			if (Arguments.size() >= 4)
			{
				try { X = std::stof(Arguments[2]); }
				catch (...) {}
				try { Y = std::stof(Arguments[3]); }
				catch (...) {}
				try { Z = std::stof(Arguments[4]); }
				catch (...) {}
			}
			else
			{
				if (!Volume)
				{
					SendMessageToConsole(PlayerController, L"They do not have an island!");
					return;
				}
			}

			if (X != -1 && Y != -1 && Z != -1) // omg what if they want to spawn it at -1 -1 -1!!!
				Builder::LoadSave(FileName, FVector(X, Y, Z), FRotator());
			else
				Builder::LoadSave(FileName, Volume);

			SendMessageToConsole(PlayerController, L"Loaded!");
		}
		else if (Command == "spawnpickup")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a WID!");
				return;
			}

			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			auto& weaponName = Arguments[1];
			int count = 1;

			try
			{
				if (NumArgs >= 2)
					count = std::stoi(Arguments[2]);
			}
			catch (...)
			{
			}

			// LOG_INFO(LogDev, "weaponName: {}", weaponName);

			auto WID = Cast<UFortWorldItemDefinition>(FindObject(weaponName, nullptr, ANY_PACKAGE));

			if (!WID)
			{
				SendMessageToConsole(PlayerController, L"Invalid WID!");
				return;
			}

			auto Location = Pawn->GetActorLocation();
			AFortPickup::SpawnPickup(WID, Location, count);
		}
		else if (Command == "listplayers")
		{
			std::string PlayerNames;

			for (int i = 0; i < ClientConnections.Num(); i++)
			{
				auto CurrentPlayerController = Cast<AFortPlayerControllerAthena>(ClientConnections.at(i)->Get("PlayerController"));

				if (!CurrentPlayerController)
					continue;

				auto CurrentPlayerState = Cast<AFortPlayerStateAthena>(CurrentPlayerController->GetPlayerState());

				if (!CurrentPlayerState->IsValidLowLevel())
					continue;

				PlayerNames += "\"" + CurrentPlayerState->GetPlayerName().ToString() + "\" ";
			}

			SendMessageToConsole(PlayerController, std::wstring(PlayerNames.begin(), PlayerNames.end()).c_str());
		}
		else if (Command == "launch")
		{
			if (Arguments.size() <= 3)
			{
				SendMessageToConsole(PlayerController, L"Please provide X, Y, and Z!\n");
				return;
			}

			float X{}, Y{}, Z{};

			try { X = std::stof(Arguments[1]); }
			catch (...) {}
			try { Y = std::stof(Arguments[2]); }
			catch (...) {}
			try { Z = std::stof(Arguments[3]); }
			catch (...) {}

			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to teleport!");
				return;
			}

			static auto LaunchCharacterFn = FindObject<UFunction>("/Script/Engine.Character.LaunchCharacter");

			struct 
			{
				FVector                                     LaunchVelocity;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
				bool                                               bXYOverride;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
				bool                                               bZOverride;                                               // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			} ACharacter_LaunchCharacter_Params{ FVector(X, Y, Z), false, false};
			Pawn->ProcessEvent(LaunchCharacterFn, &ACharacter_LaunchCharacter_Params);

			SendMessageToConsole(PlayerController, L"Launched character!");
		}
		else if (Command == "setshield")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			float Shield = 0.f;

			if (NumArgs >= 1)
			{
				try { Shield = std::stof(Arguments[1]); }
				catch (...) {}
			}

			Pawn->SetShield(Shield);
			SendMessageToConsole(PlayerController, L"Set shield!\n");
		}
		else if (Command == "god")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			Pawn->SetCanBeDamaged(!Pawn->CanBeDamaged());
			SendMessageToConsole(PlayerController, std::wstring(L"God set to " + std::to_wstring(!(bool)Pawn->CanBeDamaged())).c_str());
		}
		else if (Command == "applycid")
		{
			auto PlayerState = Cast<AFortPlayerState>(ReceivingController->GetPlayerState());

			if (!PlayerState) // ???
			{
				SendMessageToConsole(PlayerController, L"No playerstate!");
				return;
			}

			auto Pawn = Cast<AFortPlayerPawn>(ReceivingController->GetMyFortPawn());

			std::string CIDStr = Arguments[1];
			auto CIDDef = FindObject(CIDStr, nullptr, ANY_PACKAGE);
			// auto CIDDef = UObject::FindObject<UAthenaCharacterItemDefinition>(CIDStr);

			if (!CIDDef)
			{
				SendMessageToConsole(PlayerController, L"Invalid character item definition!");
				return;
			}

			LOG_INFO(LogDev, "Applying {}", CIDDef->GetFullName());
			
			if (!ApplyCID(Pawn, CIDDef))
			{
				SendMessageToConsole(PlayerController, L"Failed while applying skin! Please check the server log.");
				return;
			}

			SendMessageToConsole(PlayerController, L"Applied CID!");
		} 
		else if (Command == "summon")
		{
			if (Arguments.size() <= 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a class!\n");
				return;
			}

			auto& ClassName = Arguments[1];

			if (ClassName.contains("/Script/"))
			{
				SendMessageToConsole(PlayerController, L"For now, we don't allow non-blueprint classes.\n");
				return;
			}

			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to spawn class at!");
				return;
			}

			int Count = 1;

			if (Arguments.size() >= 3)
			{
				try { Count = std::stod(Arguments[2]); }
				catch (...) {}
			}

			constexpr int Max = 100;

			if (Count > Max)
			{
				SendMessageToConsole(PlayerController, (std::wstring(L"You went over the limit! Only spawning ") + std::to_wstring(Max) + L".").c_str());
				Count = Max;
			}

			static auto BGAClass = FindObject<UClass>("/Script/Engine.BlueprintGeneratedClass");
			auto ClassObj = LoadObject<UClass>(ClassName, BGAClass);

			if (ClassObj)
			{
				for (int i = 0; i < Count; i++)
				{
					auto Loc = Pawn->GetActorLocation();
					// Loc.Z += 1000;
					GetWorld()->SpawnActor<AActor>(ClassObj, Loc, FQuat());
				}

				SendMessageToConsole(PlayerController, L"Summoned!");
			}
			else
			{
				SendMessageToConsole(PlayerController, L"Not a valid class!");
			}
		}
		else if (Command == "sethealth")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			float Health = 100.f;

			try { Health = std::stof(Arguments[1]); }
			catch (...) {}

			Pawn->SetHealth(Health);
			SendMessageToConsole(PlayerController, L"Set health!\n");
		}
		else if (Command == "pausesafezone")
		{
			UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"pausesafezone", nullptr);
		}
		else if (Command == "teleport")
		{
			auto CheatManager = ReceivingController->SpawnCheatManager(UCheatManager::StaticClass());

			if (!CheatManager)
			{
				SendMessageToConsole(PlayerController, L"Failed to spawn player's cheat manager!");
				return;
			}

			CheatManager->Teleport();
			CheatManager = nullptr;
			SendMessageToConsole(PlayerController, L"Teleported!");
		}
		else if (Command == "bugitgo")
		{
			if (Arguments.size() <= 3)
			{
				SendMessageToConsole(PlayerController, L"Please provide X, Y, and Z!\n");
				return;
			}

			float X{}, Y{}, Z{};

			try { X = std::stof(Arguments[1]); }
			catch (...) {}
			try { Y = std::stof(Arguments[2]); }
			catch (...) {}
			try { Z = std::stof(Arguments[3]); }
			catch (...) {}

			auto Pawn = Cast<APawn>(ReceivingController->GetPawn());

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to teleport!");
				return;
			}

			Pawn->TeleportTo(FVector(X, Y, Z), Pawn->GetActorRotation());
			SendMessageToConsole(PlayerController, L"Teleported!");
		}
		else if (Command == "test")
		{
			auto SpawnBigWall = [&](AFortAthenaMutator* Mutator) {
				if (auto BarrierMutator = Cast<AFortAthenaMutator_Barrier>(Mutator))
				{
					auto BigBaseWallClass = BarrierMutator->GetBigBaseWallClass();

					LOG_INFO(LogDev, "BigBaseWallClass: {}", BigBaseWallClass->IsValidLowLevel() ? BigBaseWallClass->GetFullName() : "BadRead");

					if (BigBaseWallClass->IsValidLowLevel())
					{
						BarrierMutator->GetBigBaseWall() = GetWorld()->SpawnActor<AAthenaBigBaseWall>(BigBaseWallClass, FVector(0, 0, 0));
					}
				}
			};

			LoopMutators(SpawnBigWall);
		}
		else if (Command == "printpawn")
		{
			auto Pawn = Cast<APawn>(ReceivingController->GetPawn());

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to print!");
				return;
			}
			
			LOG_INFO(LogDev, "Pawn: 0x{:x}", __int64(Pawn));
		 }
		else { bSendHelpMessage = true; };
	}
	else { bSendHelpMessage = true; };

	if (bSendHelpMessage)
	{
		FString HelpMessage = LR"(
cheat giveitem <ShortWID> <Count=1> - Gives a weapon to the executing player, if inventory is full drops a pickup on the player.
cheat summon <BlueprintClassPathName> <Count=1> - Summons the specified blueprint class at the executing player's location. Note: There is a limit on the count.
cheat bugitgo <X> <Y> <Z> - Teleport to a location.
cheat launch <X> <Y> <Z> - Launches a player.
cheat listplayers - Gives you all players names.
cheat pausesafezone - Pauses the zone.
cheat sethealth <Health=100.f> - Sets executing player's health.
cheat setshield <Shield=0.f> - Sets executing player's shield.
cheat applycid <CIDShortName> - Sets a player's character.
cheat spawnpickup <ShortWID> - Spawns a pickup at specified player.
cheat teleport - Teleports to what the player is looking at.

If you want to execute a command on a certain player, surround their name (case sensitive) with \, and put the param anywhere. Example: cheat sethealth \Milxnor\ 100
)";
		
		SendMessageToConsole(PlayerController, HelpMessage);
	}
}