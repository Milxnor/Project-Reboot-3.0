#pragma once

#include "Player.h"
#include "Array.h"
#include "Map.h"
#include "WeakObjectPtrTemplates.h"
#include "ActorChannel.h"
#include <memcury.h>

class UNetConnection : public UPlayer
{
public:
	AActor*& GetOwningActor()
	{
		static auto OwningActorOffset = GetOffset("OwningActor");
		return Get<AActor*>(OwningActorOffset);
	}

	FName& GetClientWorldPackageName() const
	{
		static auto ClientWorldPackageNameOffset = Offsets::ClientWorldPackageName;
		return *(FName*)(__int64(this) + ClientWorldPackageNameOffset);
	}

	AActor*& GetViewTarget()
	{
		static auto ViewTargetOffset = GetOffset("ViewTarget");
		return Get<AActor*>(ViewTargetOffset);
	}

	int32 IsNetReady(bool Saturate)
	{
		static auto IsNetReadyOffset = 0x298; // 1.11
		int32 (*IsNetReadyOriginal)(UNetConnection* Connection, bool Saturate) = decltype(IsNetReadyOriginal)(this->VFTable[IsNetReadyOffset / 8]);
		return IsNetReadyOriginal(this, Saturate);
	}

	double& GetLastReceiveTime()
	{
		static auto LastReceiveTimeOffset = GetOffset("LastReceiveTime");
		return Get<double>(LastReceiveTimeOffset);
	}

	/* TSet<FName>& GetClientVisibleLevelNames()
	{
		static auto ClientVisibleLevelNamesOffset = 0x336C8;
		return *(TSet<FName>*)(__int64(this) + ClientVisibleLevelNamesOffset);
	} */

	class UNetDriver*& GetDriver()
	{
		static auto DriverOffset = GetOffset("Driver");
		return Get<UNetDriver*>(DriverOffset);
	}

	int32& GetTickCount()
	{
		static auto TickCountOffset = GetOffset("LastReceiveTime") + 8 + 8 + 8 + 8 + 8 + 4;
		return Get<int32>(TickCountOffset);
	}

	TMap<TWeakObjectPtr<AActor>, UActorChannel*>& GetActorChannels()
	{
		static auto ActorChannelsOffset = 0x33588;
		return *(TMap<TWeakObjectPtr<AActor>, UActorChannel*>*)(__int64(this) + ActorChannelsOffset);
	}

	TArray<AActor*>& GetSentTemporaries()
	{
		static auto SentTemporariesOffset = GetOffset("SentTemporaries");
		return Get<TArray<AActor*>>(SentTemporariesOffset);
	}

	bool ClientHasInitializedLevelFor(const AActor* TestActor) const;
};