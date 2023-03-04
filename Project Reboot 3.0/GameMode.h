#pragma once

#include "GameModeBase.h"

class AGameMode : public AGameModeBase
{
public:

	class AGameState*& GetGameState()
	{
		static auto GameStateOffset = this->GetOffset("GameState");
		return this->Get<class AGameState*>(GameStateOffset);
	}
};