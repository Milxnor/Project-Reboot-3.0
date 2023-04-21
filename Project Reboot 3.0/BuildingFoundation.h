#pragma once

#include "BuildingSMActor.h"
#include "Stack.h"

/* enum class EDynamicFoundationType : uint8
{
	Static = 0,
	StartEnabled_Stationary = 1,
	StartEnabled_Dynamic = 2,
	StartDisabled = 3,
	EDynamicFoundationType_MAX = 4,
};

enum class EDynamicFoundationEnabledState : uint8_t
{
	Unknown = 0,
	Enabled = 1,
	Disabled = 2,
	EDynamicFoundationEnabledState_MAX = 3
};

enum class EDynamicFoundationType : uint8_t
{
	Static = 0,
	StartEnabled_Stationary = 1,
	StartEnabled_Dynamic = 2,
	StartDisabled = 3,
	EDynamicFoundationType_MAX = 4
}; */

class ABuildingFoundation : public ABuildingSMActor
{
public:
	static inline void (*SetDynamicFoundationEnabledOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*SetDynamicFoundationTransformOriginal)(UObject* Context, FFrame& Stack, void* Ret);

	static void SetDynamicFoundationTransformHook(UObject* Context, FFrame& Stack, void* Ret);
	static void SetDynamicFoundationEnabledHook(UObject* Context, FFrame& Stack, void* Ret);
};