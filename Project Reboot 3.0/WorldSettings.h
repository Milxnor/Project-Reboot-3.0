#pragma once

#include "Actor.h"
#include "NetConnection.h"
#include "Array.h"

struct FNetViewer
{
	UNetConnection* Connection; 
	AActor* InViewer;  
	AActor* ViewTarget; 
	FVector ViewLocation; 
	FVector ViewDir;

	FNetViewer()
		: Connection(NULL)
		, InViewer(NULL)
		, ViewTarget(NULL)
		, ViewLocation(ForceInit)
		, ViewDir(ForceInit)
	{
	}

	FNetViewer(UNetConnection* InConnection, float DeltaSeconds);
};

class AWorldSettings : public AActor
{
public:
	TArray<struct FNetViewer>& GetReplicationViewers()
	{
		static auto ReplicationViewersOffset = GetOffset("ReplicationViewers");
		return Get<TArray<struct FNetViewer>>(ReplicationViewersOffset);
	}
};
