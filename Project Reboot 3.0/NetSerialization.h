#pragma once

struct FFastArraySerializerItem
{
	int                                                ReplicationID;                                            // 0x0000(0x0004) (ZeroConstructor, IsPlainOldData, RepSkip, RepNotify, Interp, NonTransactional, EditorOnly, NoDestructor, AutoWeak, ContainsInstancedReference, AssetRegistrySearchable, SimpleDisplay, AdvancedDisplay, Protected, BlueprintCallable, BlueprintAuthorityOnly, TextExportTransient, NonPIEDuplicateTransient, ExposeOnSpawn, PersistentInstance, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic, NativeAccessSpecifierProtected, NativeAccessSpecifierPrivate)
	int                                                ReplicationKey;                                           // 0x0004(0x0004) (ZeroConstructor, IsPlainOldData, RepSkip, RepNotify, Interp, NonTransactional, EditorOnly, NoDestructor, AutoWeak, ContainsInstancedReference, AssetRegistrySearchable, SimpleDisplay, AdvancedDisplay, Protected, BlueprintCallable, BlueprintAuthorityOnly, TextExportTransient, NonPIEDuplicateTransient, ExposeOnSpawn, PersistentInstance, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic, NativeAccessSpecifierProtected, NativeAccessSpecifierPrivate)
	int                                                MostRecentArrayReplicationKey;                            // 0x0008(0x0004) (ZeroConstructor, IsPlainOldData, RepSkip, RepNotify, Interp, NonTransactional, EditorOnly, NoDestructor, AutoWeak, ContainsInstancedReference, AssetRegistrySearchable, SimpleDisplay, AdvancedDisplay, Protected, BlueprintCallable, BlueprintAuthorityOnly, TextExportTransient, NonPIEDuplicateTransient, ExposeOnSpawn, PersistentInstance, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic, NativeAccessSpecifierProtected, NativeAccessSpecifierPrivate)

	FFastArraySerializerItem()
	{
		ReplicationID = -1;
		ReplicationKey = -1;
		MostRecentArrayReplicationKey = -1;
	}
};

struct FFastArraySerializer2
{
	char ItemMap[0x50];
	int IDCounter;
	int ArrayReplicationKey;
	char GuidReferencesMap[0x50];
	char GuidReferencesMap_StructDelta[0x50];

	/* void MarkItemDirty(FFastArraySerializerItem& Item)
	{
		if (Item.ReplicationID == -1)
		{
			Item.ReplicationID = ++IDCounter;
			if (IDCounter == -1)
			{
				IDCounter++;
			}
		}

		Item.ReplicationKey++;
		MarkArrayDirty();
	} */

	void MarkArrayDirty()
	{
		IncrementArrayReplicationKey();

		CachedNumItems = -1;
		CachedNumItemsToConsiderForWriting = -1;
	}

	void IncrementArrayReplicationKey()
	{
		ArrayReplicationKey++;
		if (ArrayReplicationKey == -1)
		{
			ArrayReplicationKey++;
		}
	}

	int CachedNumItems;
	int CachedNumItemsToConsiderForWriting;
	unsigned char DeltaFlags; // EFastArraySerializerDeltaFlags
	int idkwhatthisis;
};

struct FFastArraySerializer
{
	static inline bool bNewSerializer;

	int& GetArrayReplicationKey()
	{
		static int ArrayReplicationKeyOffset = 0x50 + 0x4;

		return *(int*)(__int64(this) + ArrayReplicationKeyOffset);
	}

	int& GetIDCounter()
	{
		static int IDCounterOffset = 0x50;

		return *(int*)(__int64(this) + IDCounterOffset);
	}

	int& GetCachedNumItems()
	{
		static int CachedNumItemsOffset = 0x50 + 0x8 + 0x50 + (bNewSerializer ? 0x50 : 0x0);

		// LOG_INFO(LogDev, "bNewSerializer: {}", bNewSerializer);

		return *(int*)(__int64(this) + CachedNumItemsOffset);
	}

	int& GetCachedNumItemsToConsiderForWriting()
	{
		static int CachedNumItemsToConsiderForWritingOffset = 0x50 + 0x8 + 0x50 + 0x4 + (bNewSerializer ? 0x50 : 0x0);

		return *(int*)(__int64(this) + CachedNumItemsToConsiderForWritingOffset);
	}

	void MarkItemDirty(FFastArraySerializerItem* Item)
	{
		if (Item->ReplicationID == -1)
		{
			Item->ReplicationID = ++GetIDCounter();

			if (GetIDCounter() == -1)
				GetIDCounter()++;
		}

		Item->ReplicationKey++;
		MarkArrayDirty();
	}

	void MarkArrayDirty()
	{
		// ((FFastArraySerializer2*)this)->MarkArrayDirty();
		// return;

		// ItemMap.Reset();		// This allows to clients to add predictive elements to arrays without affecting replication.
		GetArrayReplicationKey()++;

		if (GetArrayReplicationKey() == -1)
			GetArrayReplicationKey()++;

		// Invalidate the cached item counts so that they're recomputed during the next write
		GetCachedNumItems() = -1;
		GetCachedNumItemsToConsiderForWriting() = -1;
	}
};