#include "NetDriver.h"

void FNetworkObjectList::Remove(AActor* const Actor)
{
	if (Actor == nullptr)
	{
		return;
	}

	TSharedPtr<FNetworkObjectInfo>* NetworkObjectInfoPtr = nullptr;

	for (int i = 0; i < AllNetworkObjects.Num(); i++)
	{
		auto& CurrentNetworkObject = AllNetworkObjects[i];

		if (CurrentNetworkObject->Actor == Actor)
		{
			NetworkObjectInfoPtr = &CurrentNetworkObject;
			break;
		}
	}

	if (NetworkObjectInfoPtr == nullptr)
	{
		// Sanity check that we're not on the other lists either
		// check(!ActiveNetworkObjects.Contains(Actor));
		// check(!ObjectsDormantOnAllConnections.Contains(Actor));
		// check((ActiveNetworkObjects.Num() + ObjectsDormantOnAllConnections.Num()) == AllNetworkObjects.Num());
		return;
	}

	FNetworkObjectInfo* NetworkObjectInfo = NetworkObjectInfoPtr->Get();

	for (int i = 0; i < NetworkObjectInfo->DormantConnections.Num(); i++)
	{
		auto& ConnectionIt = NetworkObjectInfo->DormantConnections[i];

		UNetConnection* Connection = ConnectionIt.Get();

		if (Connection == nullptr) // || Connection->State == USOCK_Closed)
		{
			NetworkObjectInfo->DormantConnections.Remove(i);
			// ConnectionIt.RemoveCurrent();
			continue;
		}

		int32* NumDormantObjectsPerConnectionRef = nullptr;

		for (int z = 0; z < NumDormantObjectsPerConnection.Pairs.Num(); z++)
		{
			auto& Pair = NumDormantObjectsPerConnection.Pairs[z];

			if (Pair.First.ObjectIndex == Connection->InternalIndex)
			{
				NumDormantObjectsPerConnectionRef = &Pair.Second;
				break;
			}
		}

		if (!NumDormantObjectsPerConnectionRef)
		{
			// We should add here TODO MILXNOR
		}

		// check(NumDormantObjectsPerConnectionRef > 0);

		if (NumDormantObjectsPerConnectionRef)
			(*NumDormantObjectsPerConnectionRef)--;
	}

	// Remove this object from all lists

	for (int i = 0; i < AllNetworkObjects.Num(); i++)
	{
		auto& CurrentNetworkObject = AllNetworkObjects[i];

		if (CurrentNetworkObject->Actor == Actor)
		{
			AllNetworkObjects.Remove(i);
			break;
		}
	}

	for (int i = 0; i < ActiveNetworkObjects.Num(); i++)
	{
		auto& CurrentActiveNetworkObject = ActiveNetworkObjects[i];

		if (CurrentActiveNetworkObject->Actor == Actor)
		{
			ActiveNetworkObjects.Remove(i);
			break;
		}
	}

	for (int i = 0; i < ObjectsDormantOnAllConnections.Num(); i++)
	{
		auto& CurrentDormantObject = ObjectsDormantOnAllConnections[i];

		if (CurrentDormantObject->Actor == Actor)
		{
			ObjectsDormantOnAllConnections.Remove(i);
			break;
		}
	}

	// check((ActiveNetworkObjects.Num() + ObjectsDormantOnAllConnections.Num()) == AllNetworkObjects.Num());
}