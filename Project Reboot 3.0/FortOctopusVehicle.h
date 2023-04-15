#pragma once

#include "Actor.h"
#include "Vector.h"

class AFortOctopusVehicle : public AActor // AFortAthenaSKVehicle
{
public:
	static void ServerUpdateTowhookHook(AFortOctopusVehicle* OctopusVehicle, FVector InNetTowhookAimDir);
};