#pragma once

#include "NameTypes.h"

enum class EAIParamType : uint8
{
	Float,
	Int,
	Bool
	// MAX UMETA(Hidden)
};

struct FEnvNamedValue // i dont thin kthis ever changes
{
	FName ParamName;
	EAIParamType ParamType;
	float Value;
};