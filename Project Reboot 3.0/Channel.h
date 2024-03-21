#pragma once

#include "Object.h"

enum EChannelType
{
	CHTYPE_None = 0,
	CHTYPE_Control = 1,
	CHTYPE_Actor = 2,
	CHTYPE_File = 3,
	CHTYPE_Voice = 4,
	CHTYPE_MAX = 8,
};

enum EChannelCreateFlags
{
	None = (1 << 0),
	OpenedLocally = (1 << 1),
};

class UChannel : public UObject
{
public:
	void StartBecomingDormant() // T(REP)
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

	class UNetConnection*& GetConnection()
	{
		static auto ConnectionOffset = GetOffset("Connection");
		return *(UNetConnection**)(__int64(this) + ConnectionOffset);
	}

	int32 IsNetReady(bool Saturate);
};