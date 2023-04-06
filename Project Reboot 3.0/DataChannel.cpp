#include "Channel.h"
#include "NetConnection.h"

int32 UChannel::IsNetReady(bool Saturate)
{
	static auto NumOutRecOffset = 0x4C;

	if (*(int*)(__int64(this) + NumOutRecOffset) < 255)
	{
		static auto ConnectionOffset = GetOffset("Connection");
		auto Connection = Get<UNetConnection*>(ConnectionOffset);
		return Connection->IsNetReady(Saturate);
	}

	return 0;
}