#pragma once

#include "Object.h"

class UChannel : public UObject
{
public:
	void StartBecomingDormant()
	{
		void (*StartBecomingDormantOriginal)(UChannel* Channel) = decltype(StartBecomingDormantOriginal)(this->VFTable[0x298 / 8]);
		StartBecomingDormantOriginal(this);
	}

	bool IsPendingDormancy()
	{
		static auto BitfieldOffset = GetOffset("Connection") + 8;
		return ((PlaceholderBitfield*)(__int64(this) + BitfieldOffset))->Seventh;
	}

	bool IsDormant()
	{
		static auto BitfieldOffset = GetOffset("Connection") + 8;
		return ((PlaceholderBitfield*)(__int64(this) + BitfieldOffset))->Third;
	}

	int32 IsNetReady(bool Saturate);
};