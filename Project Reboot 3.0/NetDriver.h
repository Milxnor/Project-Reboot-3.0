#pragma once

#include "Object.h"
#include "UnrealString.h"

#include "World.h"

class UWorld;

struct FURL // idk where this actually goes
{
	FString                                     Protocol;                                                 // 0x0000(0x0010) (ZeroConstructor)
	FString                                     Host;                                                     // 0x0010(0x0010) (ZeroConstructor)
	int                                         Port;                                                     // 0x0020(0x0004) (ZeroConstructor, IsPlainOldData)
	int                                         Valid;                                                    // 0x0024(0x0004) (ZeroConstructor, IsPlainOldData)
	FString                                     Map;                                                      // 0x0028(0x0010) (ZeroConstructor)
	FString                                     RedirectUrl;                                              // 0x0038(0x0010) (ZeroConstructor)
	TArray<FString>                             Op;                                                       // 0x0048(0x0010) (ZeroConstructor)
	FString                                     Portal;                                                   // 0x0058(0x0010) (ZeroConstructor)
};

class UNetDriver : public UObject
{
public:
	// static inline int ReplicationDriverOffset = 0;
	static inline bool (*InitListenOriginal)(UNetDriver* NetDriver, FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error);
	static inline void (*SetWorldOriginal)(UNetDriver* NetDriver, UWorld* World);
	static inline void (*TickFlushOriginal)(UNetDriver* NetDriver);

	static void TickFlushHook(UNetDriver* NetDriver);

	bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error) { return InitListenOriginal(this, InNotify, ListenURL, bReuseAddressAndPort, Error); }
	void SetWorld(UWorld* World) { return SetWorldOriginal(this, World); }
	void ServerReplicateActors();
};