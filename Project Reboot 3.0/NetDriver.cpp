#include "NetDriver.h"

#include "reboot.h"

void UNetDriver::TickFlushHook(UNetDriver* NetDriver)
{
	static auto ReplicationDriverOffset = NetDriver->GetOffset("ReplicationDriver", false);

	if (ReplicationDriverOffset == 0)
	{
		NetDriver->ServerReplicateActors();
	}
	else
	{
		if (auto ReplicationDriver = NetDriver->Get(ReplicationDriverOffset))
			reinterpret_cast<void(*)(UObject*)>(ReplicationDriver->VFTable[Offsets::ServerReplicateActors])(ReplicationDriver);
	}

	return TickFlushOriginal(NetDriver);
}

void UNetDriver::ServerReplicateActors()
{

}