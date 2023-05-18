#pragma once

#include "Actor.h"
#include "EnvQuery.h"
#include "reboot.h"
#include "EnvQueryTypes.h"

struct FEncounterEnvironmentQueryInfo // idk what file this actually goes in or if this struct ever actaully changes
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.EncounterEnvironmentQueryInfo");
		return Struct;
	}

	static int GetPropertiesSize() { return GetStruct()->GetPropertiesSize(); }

	UEnvQuery*& GetEnvironmentQuery()
	{
		static auto EnvironmentQueryOffset = FindOffsetStruct("/Script/FortniteGame.EncounterEnvironmentQueryInfo", "EnvironmentQuery");
		return *(UEnvQuery**)(__int64(this) + EnvironmentQueryOffset);
	}

	TArray<FEnvNamedValue>& GetQueryParams()
	{
		static auto QueryParamsOffset = FindOffsetStruct("/Script/FortniteGame.EncounterEnvironmentQueryInfo", "QueryParams");
		return *(TArray<FEnvNamedValue>*)(__int64(this) + QueryParamsOffset);
	}

	bool& IsDirectional()
	{
		static auto bIsDirectionalOffset = FindOffsetStruct("/Script/FortniteGame.EncounterEnvironmentQueryInfo", "bIsDirectional");
		return *(bool*)(__int64(this) + bIsDirectionalOffset);
	}
};

class UFortAIEncounterInfo : public UObject
{
public:
};