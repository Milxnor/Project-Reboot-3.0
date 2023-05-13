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

	bool IsIdentical(FUniqueNetIdRepl* OtherUniqueId)
	{
		// idk if this is right but whatever

		bool bTest = true;

		if (this->GetReplicationBytes().Num() > this->GetReplicationBytes().Num()) // != ?
			bTest = false;

		for (int i = 0; i < this->GetReplicationBytes().Num(); i++)
		{
			if (this->GetReplicationBytes().at(i) != OtherUniqueId->GetReplicationBytes().at(i))
			{
				bTest = false;
				break;
			}
		}

		// LOG_INFO(LogDev, "btest: {}", bTest);

		return bTest;
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