#pragma once

#include "Actor.h"

class AController : public AActor
{
public:
	AActor* GetViewTarget();
};