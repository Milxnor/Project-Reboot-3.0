#pragma once

#include "Actor.h"

class AGameState : public AActor
{
public:
	float GetServerWorldTimeSeconds(); // should be in AGameStateBase
};