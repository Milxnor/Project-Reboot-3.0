#pragma once

#include "reboot.h"

#include "Text.h"

enum class EBotNamingMode : uint8 // idk if this changes
{
	RealName = 0,
	SkinName = 1,
	Anonymous = 2,
	Custom = 3,
	EBotNamingMode_MAX = 4,
};

class UFortBotNameSettings : public UObject
{
public:
	EBotNamingMode& GetNamingMode()
	{
		static auto NamingModeOffset = GetOffset("NamingMode");
		return Get<EBotNamingMode>(NamingModeOffset);
	}

	FText& GetOverrideName()
	{
		static auto OverrideNameOffset = GetOffset("OverrideName");
		return Get<FText>(OverrideNameOffset);
	}

	bool ShouldAddPlayerIDSuffix()
	{
		static auto bAddPlayerIDSuffixOffset = GetOffset("bAddPlayerIDSuffix");
		return Get<bool>(bAddPlayerIDSuffixOffset);
	}
};