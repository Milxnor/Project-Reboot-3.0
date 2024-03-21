#include "Channel.h"
#include "NetConnection.h"

int32 UChannel::IsNetReady(bool Saturate)
{
	static auto NumOutRecOffset = 0x4C;

	if (*(int*)(__int64(this) + NumOutRecOffset) < 255)
	{
		return GetConnection()->IsNetReady(Saturate);
	}

	return 0;
}