#pragma once

#include <filesystem>
#include <fstream>

#include <json.hpp>

#include "FortVolume.h"
#include "FortGameStateAthena.h"

namespace fs = std::filesystem;

namespace Builder
{
	static inline bool LoadSave(const std::string& SaveFileName, const FVector& Location, const FRotator& Rotation)
	{
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

		std::ifstream fileStream(fs::current_path().string() + "\\Islands\\" + SaveFileName + ".save");

		if (!fileStream.is_open())
		{
			// SendMessageToConsole(PlayerController, L"Failed to open filestream (file may not exist)!\n");
			return false;
		}

		nlohmann::json j;
		fileStream >> j;

		for (const auto& obj : j) {
			for (auto it = obj.begin(); it != obj.end(); ++it) {
				auto& ClassName = it.key();
				auto Class = FindObject<UClass>(ClassName);

				if (!Class)
				{
					continue;
				}

				std::vector<float> stuff;

				auto& value = it.value();

				std::vector<std::string> DevicePropertiesStr;

				if (value.is_array()) {
					for (const auto& elem : value) {
						if (!elem.is_array())
						{
							stuff.push_back(elem);
						}
						else // Device Properties
						{
							for (const auto& elem2 : elem) {
								for (auto it2 = elem2.begin(); it2 != elem2.end(); ++it2) {
									auto& value2z = it2.value();
									DevicePropertiesStr.push_back(value2z);
								}
							}
						}
					}
				}
				else
				{

				}

				// std::cout << "stuff.size(): " << stuff.size() << '\n';

				if (stuff.size() >= 8)
				{
					FRotator rot{};
					rot.Pitch = stuff[3] + Rotation.Pitch;
					rot.Roll = stuff[4] + Rotation.Roll;
					rot.Yaw = stuff[5] + Rotation.Yaw;

					FVector Scale3D = { 1, 1, 1 };

					if (stuff.size() >= 11)
					{
						Scale3D.X = stuff[8];
						Scale3D.Y = stuff[9];
						Scale3D.Z = stuff[10];
					}

					auto NewActor = GetWorld()->SpawnActor<ABuildingActor>(Class, FVector{ stuff[0] + Location.X , stuff[1] + Location.Y, stuff[2] + Location.Z },
						rot.Quaternion(), Scale3D);

					if (!NewActor)
						continue;

					// NewActor->bCanBeDamaged = false;
					NewActor->InitializeBuildingActor(NewActor, nullptr, false);
					// NewActor->GetTeamIndex() = stuff[6];
					// NewActor->SetHealth(stuff[7]);

					static auto FortActorOptionsComponentClass = FindObject<UClass>(L"/Script/FortniteGame.FortActorOptionsComponent");
					auto ActorOptionsComponent = FortActorOptionsComponentClass ? NewActor->GetComponentByClass(FortActorOptionsComponentClass) : nullptr;

					// continue;

					/*
					if (ActorOptionsComponent)
					{
						// UE::TMap<FString, FString> Map{};

						TArray<FString> PropertyNameStrs;

						for (int kl = 0; kl < DevicePropertiesStr.size(); kl += 2)
						{
							if (kl + 1 >= DevicePropertiesStr.size())
								break;

							auto PropertyName = DevicePropertiesStr[kl];
							auto PropertyData = DevicePropertiesStr[kl + 1];

							LOG_INFO(LogCreative, "PropertyName: {}", PropertyName);
							LOG_INFO(LogCreative, "PropertyData: {}", PropertyData);

							PropertyNameStrs.Add(std::wstring(PropertyName.begin(), PropertyName.end()).c_str());
						}

						for (int jk = 0; jk < PropertyNameStrs.size(); jk++)
						{
							LOG_INFO(LogCreative, "[{}] PropertyName: {}", jk, PropertyNameStrs.at(jk).ToString());
						}
					}
					*/
				}
			}
		}

		return true;
	}

	static inline bool LoadSave(const std::string& SaveFileName, AFortVolume* LoadIntoVolume)
	{
		/* auto AllBuildingActors = LoadIntoVolume->GetActorsWithinVolumeByClass(ABuildingActor::StaticClass());

		for (int i = 0; i < AllBuildingActors.Num(); ++i)
		{
			auto CurrentBuildingActor = (ABuildingActor*)AllBuildingActors[i];
			CurrentBuildingActor->SilentDie();
		} */

		return LoadSave(SaveFileName, LoadIntoVolume->GetActorLocation(), LoadIntoVolume->GetActorRotation());
	}
}