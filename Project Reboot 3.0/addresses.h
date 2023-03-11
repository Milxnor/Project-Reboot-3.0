#pragma once

// #include "finder.h"
#include "log.h"
#include "inc.h"

namespace Addresses
{
	extern inline uint64 StaticFindObject = 0;
	extern inline uint64 StaticLoadObject = 0;
	extern inline uint64 ObjectArray = 0;
	extern inline uint64 InitListen = 0;
	extern inline uint64 CreateNetDriver = 0;
	extern inline uint64 SetWorld = 0;
	extern inline uint64 ProcessEvent = 0;
	extern inline uint64 PickupDelay = 0;
	extern inline uint64 GetMaxTickRate = 0;
	extern inline uint64 GetPlayerViewpoint = 0;
	extern inline uint64 InitHost = 0;
	extern inline uint64 PauseBeaconRequests = 0;
	extern inline uint64 SpawnActor = 0;
	extern inline uint64 KickPlayer = 0;
	extern inline uint64 TickFlush = 0;
	extern inline uint64 GetNetMode = 0;
	extern inline uint64 Realloc = 0;
	extern inline uint64 CollectGarbage = 0;
	extern inline uint64 NoMCP = 0;
	extern inline uint64 PickTeam = 0;
	extern inline uint64 InternalTryActivateAbility = 0;
	extern inline uint64 GiveAbility = 0;
	extern inline uint64 CantBuild = 0;
	extern inline uint64 ReplaceBuildingActor = 0;
	extern inline uint64 GiveAbilityAndActivateOnce = 0;
	extern inline uint64 OnDamageServer = 0;
	extern inline uint64 GIsServer = 0;
	extern inline uint64 GIsClient = 0;
	extern inline uint64 ActorGetNetMode = 0;
	extern inline uint64 ChangeGameSessionId = 0;
	extern inline uint64 DispatchRequest = 0;
	extern inline uint64 AddNavigationSystemToWorld = 0;
	extern inline uint64 NavSystemCleanUp = 0;
	extern inline uint64 LoadPlayset = 0;
	extern inline uint64 SetZoneToIndex = 0;
	extern inline uint64 CompletePickupAnimation = 0;

	void SetupVersion(); // Finds Engine Version
	void FindAll();
	void Print();
	void Init();

	std::vector<uint64> GetFunctionsToNull();
}

namespace Offsets
{
	extern inline uint64 Func = 0;
	extern inline uint64 PropertiesSize = 0;
	extern inline uint64 Children = 0;
	extern inline uint64 SuperStruct = 0;
	extern inline uint64 Offset_Internal = 0;
	extern inline uint64 ServerReplicateActors = 0;

	void FindAll();
	void Print();
}

#define ENUM_CLASS_FLAGS(Enum) \
	inline           Enum& operator|=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline           Enum& operator&=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline           Enum& operator^=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator| (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator& (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator^ (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline constexpr bool  operator! (Enum  E)             { return !(__underlying_type(Enum))E; } \
	inline constexpr Enum  operator~ (Enum  E)             { return (Enum)~(__underlying_type(Enum))E; }