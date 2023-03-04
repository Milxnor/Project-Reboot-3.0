#include "NetDriver.h"

#include "reboot.h"

void UNetDriver::TickFlushHook(UNetDriver* NetDriver)
{
	static auto ReplicationDriverOffset = NetDriver->GetOffset("ReplicationDriver");

	if (auto ReplicationDriver = NetDriver->Get(ReplicationDriverOffset))
		reinterpret_cast<void(*)(UObject*)>(ReplicationDriver->VFTable[Offsets::ServerReplicateActors])(ReplicationDriver);

	return TickFlushOriginal(NetDriver);
}