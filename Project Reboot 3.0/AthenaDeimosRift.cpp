#include "AthenaDeimosRift.h"

void AAthenaDeimosRift::QueueActorsToSpawnHook(UObject* Context, FFrame* Stack, void* Ret)
{
	LOG_INFO(LogDev, "QueueActorsToSpawnHook!");
	return QueueActorsToSpawnOriginal(Context, Stack, Ret);
}