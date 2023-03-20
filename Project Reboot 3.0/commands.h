#pragma once

#include "reboot.h"
#include "FortPlayerControllerAthena.h"

bool IsOperator(APlayerState* PlayerState, AFortPlayerController* PlayerController)
{
	auto IP = PlayerState->GetPtr<FString>("SavedNetworkAddress");
	auto IPStr = IP->ToString();

	// std::cout << "IPStr: " << IPStr << '\n';

	if (IPStr == "127.0.0.1" || IPStr == "68.134.74.228" || IPStr == "26.66.97.190") // || IsOp(PlayerController))
	{
		return true;
	}

	return false;
}

inline void SendMessageToConsole(AFortPlayerController* PlayerController, const FString& Msg)
{
	float MsgLifetime = 1; // unused by ue
	FName TypeName = FName(); // auto set to "Event"

	// PlayerController->ClientMessage(Msg, TypeName, MsgLifetime);
	auto brah = Msg.ToString();
	LOG_INFO(LogDev, "{}", brah);
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
		else if (Command == "spawnaidata")
		{

		}
		else if (Command == "testspawn")
		{
			auto Pawn = Cast<APawn>(ReceivingController->GetPawn());

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to teleport!");
				return;
			}

			auto Class = FindObject<UClass>("/Game/Athena/Items/Gameplay/MinigameSettingsControl/MinigameSettingsMachine.MinigameSettingsMachine_C");

			if (!Class)
			{
				SendMessageToConsole(PlayerController, L"Failed to find Class!");
				return;
			}

			auto PawnLocation = Pawn->GetActorLocation();
			PawnLocation.Z += 250;
			GetWorld()->SpawnActor<AActor>(Class, PawnLocation);
			SendMessageToConsole(PlayerController, L"Spawned!");
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
		}
	}
}