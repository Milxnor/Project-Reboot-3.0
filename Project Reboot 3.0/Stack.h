#pragma once

#include "OutputDevice.h"
#include "Class.h"

struct FFrame : public FOutputDevice // https://github.com/EpicGames/UnrealEngine/blob/7acbae1c8d1736bb5a0da4f6ed21ccb237bc8851/Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h#L83
{
public:
	void** VFT;

	// Variables.
	UFunction* Node;
	UObject* Object;
	uint8* Code;
	uint8* Locals;

	// MORE STUFF HERE
};