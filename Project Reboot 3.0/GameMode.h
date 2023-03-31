#pragma once

#include "GameModeBase.h"

class AGameMode : public AGameModeBase
{
public:

	void RestartGame();

	class AGameState*& GetGameState()
	{
		static auto GameStateOffset = this->GetOffset("GameState");
		return this->Get<class AGameState*>(GameStateOffset);
	}
};