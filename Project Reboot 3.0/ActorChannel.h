#pragma once

#include "Channel.h"
#include "NetworkGuid.h"

enum ESetChannelActorFlags
{
	None1 = 0, // Bro compiler what
	SkipReplicatorCreation = (1 << 0),
	SkipMarkActive = (1 << 1),
};

class UActorChannel : public UChannel
{
public:
	static inline void (*originalSetChannelActor)(UActorChannel*, AActor*);
	static inline __int64 (*originalReplicateActor)(UActorChannel*);

	void SetChannelActor(AActor* Actor, ESetChannelActorFlags Flags)
	{
		originalSetChannelActor(this, Actor); // T(REP) ADD FLAGS FOR NEWER BUILDS
	}

	__int64 ReplicateActor() // Returns how many bits were replicated (does not include non-bunch packet overhead)
	{
		return originalReplicateActor(this);
	}

	double& GetLastUpdateTime()
	{
		static auto LastUpdateTimeOffset = GetOffset("Actor") + 8 + 4 + 4 + 8; // checked on 4.19
		return Get<double>(LastUpdateTimeOffset);
	}

	double& GetRelevantTime()
	{
		static auto RelevantTimeOffset = GetOffset("Actor") + 8 + 4 + 4; // checked on 4.19
		return Get<double>(RelevantTimeOffset);
	}

	AActor*& GetActor()
	{
		static auto ActorOffset = GetOffset("Actor");
		return Get<AActor*>(ActorOffset);
	}

	void Close()
	{
		static void (*ActorChannelClose)(UActorChannel*) = decltype(ActorChannelClose)(Addresses::ActorChannelClose);
		ActorChannelClose(this);
	}
};