// this file wasn't fun

#pragma once

#include "reboot.h"

struct FUniqueNetIdRepl // : public FUniqueNetIdWrapper
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/Engine.UniqueNetIdRepl");
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

	void CopyFromAnotherUniqueId(FUniqueNetIdRepl* OtherUniqueId)
	{
		CopyStruct(this, OtherUniqueId, GetSizeOfStruct(), GetStruct());

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