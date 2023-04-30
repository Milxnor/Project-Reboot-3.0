#include "NetConnection.h"

bool UNetConnection::ClientHasInitializedLevelFor(const AActor* TestActor) const
{
	static auto ClientHasInitializedLevelForAddr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 5A 20 48 8B F1 4C 8B C3", false).Get();

	if (!ClientHasInitializedLevelForAddr)
	{
		ClientHasInitializedLevelForAddr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 5A 20 48 8B F1 4C 8B C3 48 8D", false).Get(); // 1.8
	
		if (!ClientHasInitializedLevelForAddr)
		{
			ClientHasInitializedLevelForAddr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F9 48 85 D2 74 35 48").Get(); // 1.7.2
		}
	}
	
	if (!ClientHasInitializedLevelForAddr)
		return true;

	static bool (*ClientHasInitializedLevelForOriginal)(const UNetConnection * Connection, const AActor * TestActor)
		= decltype(ClientHasInitializedLevelForOriginal)(ClientHasInitializedLevelForAddr);
	// ^ This is virtual but it doesn't really matter

	return ClientHasInitializedLevelForOriginal(this, TestActor);
}