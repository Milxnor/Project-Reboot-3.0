#pragma once

#include "Actor.h"

class UActorComponent : public UObject
{
public:
	AActor* GetOwner();
};