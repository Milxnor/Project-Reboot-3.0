// this file wasn't fun

#pragma once

#include "reboot.h"

struct FUniqueNetIdRepl // : public FUniqueNetIdWrapper
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/Engine.UniqueNetIdRepl");
		return Struct;
	}

	static int GetSizeOfStruct()
	{
		static auto Size = GetStruct()->GetPropertiesSize();
		return Size;
	}

	TArray<uint8>& GetReplicationBytes()
	{
		static auto ReplicationBytesOffset = FindOffsetStruct("/Script/Engine.UniqueNetIdRepl", "ReplicationBytes");
		return *(TArray<uint8>*)(__int64(this) + ReplicationBytesOffset);
	}

	FORCEINLINE int GetSize() { return GetReplicationBytes().Num(); } // LITERLALY IDK IF THIS IS RIGHT CUZ I CANT FIND IMPL
	FORCEINLINE uint8* GetBytes() { return GetReplicationBytes().Data; } // ^^^

	FORCENOINLINE bool IsIdentical(FUniqueNetIdRepl* OtherUniqueId)
	{
		return (GetSize() == OtherUniqueId->GetSize()) &&
			(memcmp(GetBytes(), OtherUniqueId->GetBytes(), GetSize()) == 0);
	}

	void CopyFromAnotherUniqueId(FUniqueNetIdRepl* OtherUniqueId)
	{
		CopyStruct(this, OtherUniqueId, GetSizeOfStruct(), GetStruct());

		return;

		auto& ReplicationBytes = GetReplicationBytes();

		ReplicationBytes.Free();

		// Now this is what we call 1 to 1 array copying.

		for (int i = 0; i < OtherUniqueId->GetReplicationBytes().Num(); i++)
		{
			ReplicationBytes.Add(OtherUniqueId->GetReplicationBytes().at(i));
		}
	}

	/* bool IsEqual(FUniqueNetIdRepl* Other) */
};