#pragma once

#include "Array.h"

#include <fstream>

#include "reboot.h"

class FKismetBytecodeDisassembler
{
private:
	std::vector<uint8> Script;
	std::string Indents;
	std::ofstream Stream;
public:
	void DisassembleStructure(UFunction* Source);

	int32 ReadINT(int32& ScriptIndex);
	uint64 ReadQWORD(int32& ScriptIndex);
	uint8 ReadBYTE(int32& ScriptIndex);
	std::string ReadName(int32& ScriptIndex);
	uint16 ReadWORD(int32& ScriptIndex);
	float ReadFLOAT(int32& ScriptIndex);
	CodeSkipSizeType ReadSkipCount(int32& ScriptIndex);
	std::string ReadString(int32& ScriptIndex);
	std::string ReadString8(int32& ScriptIndex);
	std::string ReadString16(int32& ScriptIndex);

	uint8 SerializeExpr(int32& ScriptIndex);
	void ProcessCastByte(int32 CastType, int32& ScriptIndex);
	void ProcessCommon(int32& ScriptIndex, uint8 Opcode);

	void AddIndent()
	{
		Indents += ("  ");
	}

	void DropIndent()
	{
		// Blah, this is awful
		// Indents.LeftInline(Indents.Len() - 2);
		Indents = Indents.substr(2);
	}

	template <typename T>
	T* ReadPointer(int32& ScriptIndex)
	{
		return (T*)ReadQWORD(ScriptIndex);
	}
};