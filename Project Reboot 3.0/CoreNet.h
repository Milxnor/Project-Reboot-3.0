#pragma once

#include "inc.h"

struct FPacketIdRange
{
	FPacketIdRange(int32 _First, int32 _Last) : First(_First), Last(_Last) { }
	FPacketIdRange(int32 PacketId) : First(PacketId), Last(PacketId) { }
	FPacketIdRange() : First(INDEX_NONE), Last(INDEX_NONE) { }
	int32 First;
	int32 Last;

	bool InRange(int32 PacketId) const
	{
		return (First <= PacketId && PacketId <= Last);
	}
};