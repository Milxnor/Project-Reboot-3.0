#pragma once

#include "Player.h"
#include "Array.h"
#include "Map.h"
#include "WeakObjectPtrTemplates.h"
#include "ActorChannel.h"
#include <memcury.h>
#include "KismetStringLibrary.h"

class UNetConnection : public UPlayer
{
public:
	static inline UChannel* (*originalCreateChannel)(UNetConnection*, int, bool, int32_t);
	static inline UChannel* (*originalCreateChannelByName)(UNetConnection* Connection, FName* ChName, EChannelCreateFlags CreateFlags, int32_t ChannelIndex);

	TSet<FNetworkGUID>& GetDestroyedStartupOrDormantActors() // T(REP)
	{
		static int off = Fortnite_Version == 1.11 ? 0x33678 : 0;

		return *(TSet<FNetworkGUID>*)(__int64(this) + off);
	}

	UChannel* CreateChannel(EChannelType ChannelType, bool bOpenedLocally, EChannelCreateFlags CreateFlags, int32 ChannelIndex = INDEX_NONE)
	{
		if (Engine_Version >= 422)
		{
			FString ActorStr = L"Actor";
			FName ActorName = UKismetStringLibrary::Conv_StringToName(ActorStr);

			int ChannelIndex = -1; // 4294967295
			return (UActorChannel*)originalCreateChannelByName(this, &ActorName, CreateFlags, ChannelIndex);
		}
		else
		{
			return (UActorChannel*)originalCreateChannel(this, ChannelType, bOpenedLocally, ChannelIndex);
		}
	}

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

	inline AActor*& GetViewTarget()
	{
		static auto ViewTargetOffset = GetOffset("ViewTarget");
		return Get<AActor*>(ViewTargetOffset);
	}

	bool& GetTimeSensitive() // T(REP)
	{
		return *(bool*)(0);
	}

	TArray<class UChildConnection*>& GetChildren()
	{
		static auto ChildrenOffset = GetOffset("Children");
		return Get<TArray<class UChildConnection*>>(ChildrenOffset);
	}

	int32 IsNetReady(bool Saturate) // T(REP)
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

	UObject*& GetPackageMap()
	{
		static auto PackageMapOffset = GetOffset("PackageMap");
		return Get<UObject*>(PackageMapOffset);
	}

	bool ClientHasInitializedLevelFor(const AActor* TestActor) const;
};