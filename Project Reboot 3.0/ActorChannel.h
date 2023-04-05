#pragma once

#include "Channel.h"
#include "NetworkGuid.h"

class UActorChannel : public UChannel
{
public:
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