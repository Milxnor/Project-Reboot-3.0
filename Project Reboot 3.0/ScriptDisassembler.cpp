#include "ScriptDisassembler.h"

enum EExprToken425
{
	// Variable references.
	EX_LocalVariable = 0x00,	// A local variable.
	EX_InstanceVariable = 0x01,	// An object variable.
	EX_DefaultVariable = 0x02, // Default variable for a class context.
	//						= 0x03,
	EX_Return = 0x04,	// Return from function.
	//						= 0x05,
	EX_Jump = 0x06,	// Goto a local address in code.
	EX_JumpIfNot = 0x07,	// Goto if not expression.
	//						= 0x08,
	EX_Assert = 0x09,	// Assertion.
	//						= 0x0A,
	EX_Nothing = 0x0B,	// No operation.
	//						= 0x0C,
	//						= 0x0D,
	//						= 0x0E,
	EX_Let = 0x0F,	// Assign an arbitrary size value to a variable.
	//						= 0x10,
	//						= 0x11,
	EX_ClassContext = 0x12,	// Class default object context.
	EX_MetaCast = 0x13, // Metaclass cast.
	EX_LetBool = 0x14, // Let boolean variable.
	EX_EndParmValue = 0x15,	// end of default value for optional function parameter
	EX_EndFunctionParms = 0x16,	// End of function call parameters.
	EX_Self = 0x17,	// Self object.
	EX_Skip = 0x18,	// Skippable expression.
	EX_Context = 0x19,	// Call a function through an object context.
	EX_Context_FailSilent = 0x1A, // Call a function through an object context (can fail silently if the context is NULL; only generated for functions that don't have output or return values).
	EX_VirtualFunction = 0x1B,	// A function call with parameters.
	EX_FinalFunction = 0x1C,	// A prebound function call with parameters.
	EX_IntConst = 0x1D,	// Int constant.
	EX_FloatConst = 0x1E,	// Floating point constant.
	EX_StringConst = 0x1F,	// String constant.
	EX_ObjectConst = 0x20,	// An object constant.
	EX_NameConst = 0x21,	// A name constant.
	EX_RotationConst = 0x22,	// A rotation constant.
	EX_VectorConst = 0x23,	// A vector constant.
	EX_ByteConst = 0x24,	// A byte constant.
	EX_IntZero = 0x25,	// Zero.
	EX_IntOne = 0x26,	// One.
	EX_True = 0x27,	// Bool True.
	EX_False = 0x28,	// Bool False.
	EX_TextConst = 0x29, // FText constant
	EX_NoObject = 0x2A,	// NoObject.
	EX_TransformConst = 0x2B, // A transform constant
	EX_IntConstByte = 0x2C,	// Int constant that requires 1 byte.
	EX_NoInterface = 0x2D, // A null interface (similar to EX_NoObject, but for interfaces)
	EX_DynamicCast = 0x2E,	// Safe dynamic class casting.
	EX_StructConst = 0x2F, // An arbitrary UStruct constant
	EX_EndStructConst = 0x30, // End of UStruct constant
	EX_SetArray = 0x31, // Set the value of arbitrary array
	EX_EndArray = 0x32,
	//						= 0x33,
	EX_UnicodeStringConst = 0x34, // Unicode string constant.
	EX_Int64Const = 0x35,	// 64-bit integer constant.
	EX_UInt64Const = 0x36,	// 64-bit unsigned integer constant.
	//						= 0x37,
	EX_PrimitiveCast = 0x38,	// A casting operator for primitives which reads the type as the subsequent byte
	EX_SetSet = 0x39,
	EX_EndSet = 0x3A,
	EX_SetMap = 0x3B,
	EX_EndMap = 0x3C,
	EX_SetConst = 0x3D,
	EX_EndSetConst = 0x3E,
	EX_MapConst = 0x3F,
	EX_EndMapConst = 0x40,
	//						= 0x41,
	EX_StructMemberContext = 0x42, // Context expression to address a property within a struct
	EX_LetMulticastDelegate = 0x43, // Assignment to a multi-cast delegate
	EX_LetDelegate = 0x44, // Assignment to a delegate
	EX_LocalVirtualFunction = 0x45, // Special instructions to quickly call a virtual function that we know is going to run only locally
	EX_LocalFinalFunction = 0x46, // Special instructions to quickly call a final function that we know is going to run only locally
	//						= 0x47, // CST_ObjectToBool
	EX_LocalOutVariable = 0x48, // local out (pass by reference) function parameter
	//						= 0x49, // CST_InterfaceToBool
	EX_DeprecatedOp4A = 0x4A,
	EX_InstanceDelegate = 0x4B,	// const reference to a delegate or normal function object
	EX_PushExecutionFlow = 0x4C, // push an address on to the execution flow stack for future execution when a EX_PopExecutionFlow is executed.   Execution continues on normally and doesn't change to the pushed address.
	EX_PopExecutionFlow = 0x4D, // continue execution at the last address previously pushed onto the execution flow stack.
	EX_ComputedJump = 0x4E,	// Goto a local address in code, specified by an integer value.
	EX_PopExecutionFlowIfNot = 0x4F, // continue execution at the last address previously pushed onto the execution flow stack, if the condition is not true.
	EX_Breakpoint = 0x50, // Breakpoint.  Only observed in the editor, otherwise it behaves like EX_Nothing.
	EX_InterfaceContext = 0x51,	// Call a function through a native interface variable
	EX_ObjToInterfaceCast = 0x52,	// Converting an object reference to native interface variable
	EX_EndOfScript = 0x53, // Last byte in script code
	EX_CrossInterfaceCast = 0x54, // Converting an interface variable reference to native interface variable
	EX_InterfaceToObjCast = 0x55, // Converting an interface variable reference to an object
	//						= 0x56,
	//						= 0x57,
	//						= 0x58,
	//						= 0x59,
	EX_WireTracepoint = 0x5A, // Trace point.  Only observed in the editor, otherwise it behaves like EX_Nothing.
	EX_SkipOffsetConst = 0x5B, // A CodeSizeSkipOffset constant
	EX_AddMulticastDelegate = 0x5C, // Adds a delegate to a multicast delegate's targets
	EX_ClearMulticastDelegate = 0x5D, // Clears all delegates in a multicast target
	EX_Tracepoint = 0x5E, // Trace point.  Only observed in the editor, otherwise it behaves like EX_Nothing.
	EX_LetObj = 0x5F,	// assign to any object ref pointer
	EX_LetWeakObjPtr = 0x60, // assign to a weak object pointer
	EX_BindDelegate = 0x61, // bind object and name to delegate
	EX_RemoveMulticastDelegate = 0x62, // Remove a delegate from a multicast delegate's targets
	EX_CallMulticastDelegate = 0x63, // Call multicast delegate
	EX_LetValueOnPersistentFrame = 0x64,
	EX_ArrayConst = 0x65,
	EX_EndArrayConst = 0x66,
	EX_SoftObjectConst = 0x67,
	EX_CallMath = 0x68, // static pure function from on local call space
	EX_SwitchValue = 0x69,
	EX_InstrumentationEvent = 0x6A, // Instrumentation event
	EX_ArrayGetByRef = 0x6B,
	EX_ClassSparseDataVariable = 0x6C, // Sparse data variable
	EX_FieldPathConst = 0x6D,
	EX_Max = 0x100,
};


void FKismetBytecodeDisassembler::DisassembleStructure(UFunction* Source)
{
	// Script.Empty();
	// Script.Append(Source->Script);

	for (int i = 0; i < Source->GetScript().Num(); i++)
	{
		Script.push_back(Source->GetScript().at(i));
	}

	int32 ScriptIndex = 0;
	while (ScriptIndex < Script.size())
	{
		Stream << std::format("Label_0x{:x}", ScriptIndex);

		AddIndent();
		SerializeExpr(ScriptIndex);
		DropIndent();
	}
}

uint8 FKismetBytecodeDisassembler::SerializeExpr(int32& ScriptIndex)
{
	AddIndent();

	uint8 Opcode = Script[ScriptIndex];
	ScriptIndex++;

	ProcessCommon(ScriptIndex, Opcode);

	DropIndent();

	return Opcode;
}


int32 FKismetBytecodeDisassembler::ReadINT(int32& ScriptIndex)
{
	int32 Value = Script[ScriptIndex]; ++ScriptIndex;
	Value = Value | ((int32)Script[ScriptIndex] << 8); ++ScriptIndex;
	Value = Value | ((int32)Script[ScriptIndex] << 16); ++ScriptIndex;
	Value = Value | ((int32)Script[ScriptIndex] << 24); ++ScriptIndex;

	return Value;
}

uint64 FKismetBytecodeDisassembler::ReadQWORD(int32& ScriptIndex)
{
	uint64 Value = Script[ScriptIndex]; ++ScriptIndex;
	Value = Value | ((uint64)Script[ScriptIndex] << 8); ++ScriptIndex;
	Value = Value | ((uint64)Script[ScriptIndex] << 16); ++ScriptIndex;
	Value = Value | ((uint64)Script[ScriptIndex] << 24); ++ScriptIndex;
	Value = Value | ((uint64)Script[ScriptIndex] << 32); ++ScriptIndex;
	Value = Value | ((uint64)Script[ScriptIndex] << 40); ++ScriptIndex;
	Value = Value | ((uint64)Script[ScriptIndex] << 48); ++ScriptIndex;
	Value = Value | ((uint64)Script[ScriptIndex] << 56); ++ScriptIndex;

	return Value;
}

struct FScriptName // todo mve
{
	uint32_t ComparisonIndex;
	uint32_t DisplayIndex;
	uint32_t Number = 0;
};

uint8 FKismetBytecodeDisassembler::ReadBYTE(int32& ScriptIndex)
{
	uint8 Value = Script[ScriptIndex]; ++ScriptIndex;

	return Value;
}

std::string FKismetBytecodeDisassembler::ReadName(int32& ScriptIndex)
{
	const FScriptName ConstValue = *(FScriptName*)(Script.data() + ScriptIndex);
	ScriptIndex += sizeof(FScriptName);

	FName Name;
	Name.ComparisonIndex.Value = ConstValue.ComparisonIndex;
	Name.Number = ConstValue.Number;

	return Name.ToString();
}

uint16 FKismetBytecodeDisassembler::ReadWORD(int32& ScriptIndex)
{
	uint16 Value = Script[ScriptIndex]; ++ScriptIndex;
	Value = Value | ((uint16)Script[ScriptIndex] << 8); ++ScriptIndex;
	return Value;
}

float FKismetBytecodeDisassembler::ReadFLOAT(int32& ScriptIndex)
{
	union { float f; int32 i; } Result;
	Result.i = ReadINT(ScriptIndex);
	return Result.f;
}

CodeSkipSizeType FKismetBytecodeDisassembler::ReadSkipCount(int32& ScriptIndex)
{
#if SCRIPT_LIMIT_BYTECODE_TO_64KB
	return ReadWORD(ScriptIndex);
#else
	static_assert(sizeof(CodeSkipSizeType) == 4, "Update this code as size changed.");
	return ReadINT(ScriptIndex);
#endif
}

std::string FKismetBytecodeDisassembler::ReadString(int32& ScriptIndex)
{
	const uint8 Opcode = Script[ScriptIndex++];

	switch (Opcode)
	{
	case EX_StringConst:
		return ReadString8(ScriptIndex);

	case EX_UnicodeStringConst:
		return ReadString16(ScriptIndex);

	default:
		// checkf(false, TEXT("FKismetBytecodeDisassembler::ReadString - Unexpected opcode. Expected %d or %d, got %d"), (int)EX_StringConst, (int)EX_UnicodeStringConst, (int)Opcode);
		break;
	}

	return std::string();
}

std::string FKismetBytecodeDisassembler::ReadString8(int32& ScriptIndex)
{
	std::string Result;

	do
	{
		Result += (ANSICHAR)ReadBYTE(ScriptIndex);
	} while (Script[ScriptIndex - 1] != 0);

	return Result;
}

std::string FKismetBytecodeDisassembler::ReadString16(int32& ScriptIndex)
{
	std::string Result;

	do
	{
		Result += (TCHAR)ReadWORD(ScriptIndex);
	} while ((Script[ScriptIndex - 1] != 0) || (Script[ScriptIndex - 2] != 0));

	// Inline combine any surrogate pairs in the data when loading into a UTF-32 string
	// StringConv::InlineCombineSurrogates(Result);

	return Result;
}

void FKismetBytecodeDisassembler::ProcessCastByte(int32 CastType, int32& ScriptIndex)
{
	// Expression of cast
	SerializeExpr(ScriptIndex);
}

namespace EExprToken
{
	static uint8 GetPrimitiveCast()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_PrimitiveCast;
	}

	static uint8 GetEndSet()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_EndSet;
	}

	static uint8 GetSetConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_SetConst;
	}

	static uint8 GetEndSetConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_EndSetConst;
	}

	static uint8 GetSetMap()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_SetMap;
	}

	static uint8 GetEndMap()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_EndMap;
	}

	static uint8 GetMapConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_MapConst;
	}

	static uint8 GetEndMapConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_EndMapConst;
	}

	static uint8 GetObjToInterfaceCast()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_ObjToInterfaceCast;
	}

	static uint8 GetCrossInterfaceCast()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_CrossInterfaceCast;
	}

	static uint8 GetInterfaceToObjCast()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_InterfaceToObjCast;
	}

	static uint8 GetLet()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Let;
	}

	static uint8 GetSetSet()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_SetSet;
	}

	static uint8 GetLetObj()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_LetObj;
	}

	static uint8 GetLetWeakObjectPtr()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_LetWeakObjPtr;
	}

	static uint8 GetLetBool()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_LetBool;
	}

	static uint8 GetLetValueOnPersistentFrame()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_LetValueOnPersistentFrame;
	}

	static uint8 GetStructMemberContext()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_StructMemberContext;
	}

	static uint8 GetLetDelegate()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_LetDelegate;
	}

	static uint8 GetLocalVirtualFunction()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_LocalVirtualFunction;
	}

	static uint8 GetLocalFinalFunction()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_LocalFinalFunction;
	}

	static uint8 GetLetMulticastDelegate()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_LetMulticastDelegate;
	}

	static uint8 GetComputedJump()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_ComputedJump;
	}

	static uint8 GetJump()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Jump;
	}

	static uint8 GetLocalVariable()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_LocalVariable;
	}

	static uint8 GetDefaultVariable()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_DefaultVariable;
	}

	static uint8 GetInstanceVariable()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_InstanceVariable;
	}

	static uint8 GetLocalOutVariable()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_LocalOutVariable;
	}

	static uint8 GetClassSparseDataVariable()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_ClassSparseDataVariable;
	}

	static uint8 GetInterfaceContext()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_InterfaceContext;
	}

	static uint8 GetDeprecatedOp4A()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_DeprecatedOp4A;
	}

	static uint8 GetNothing()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Nothing;
	}

	static uint8 GetEndOfScript()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_EndOfScript;
	}

	static uint8 GetEndFunctionParms()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_EndFunctionParms;
	}

	static uint8 GetEndStructConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_EndStructConst;
	}

	static uint8 GetEndArray()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_EndArray;
	}

	static uint8 GetEndArrayConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_EndArrayConst;
	}

	static uint8 GetIntZero()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_IntZero;
	}

	static uint8 GetIntOne()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_IntOne;
	}

	static uint8 GetTrue()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_True;
	}

	static uint8 GetFalse()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_False;
	}

	static uint8 GetNoObject()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_NoObject;
	}

	static uint8 GetNoInterface()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_NoInterface;
	}

	static uint8 GetSelf()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Self;
	}

	static uint8 GetEndParmValue()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_EndParmValue;
	}

	static uint8 GetReturn()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Return;
	}

	static uint8 GetCallMath()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_CallMath;
	}

	static uint8 GetFinalFunction()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_FinalFunction;
	}

	static uint8 GetCallMulticastDelegate()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_CallMulticastDelegate;
	}

	static uint8 GetVirtaulFunction()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_VirtualFunction;
	}

	static uint8 GetClassContext()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_ClassContext;
	}

	static uint8 GetContext()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Context;
	}

	static uint8 GetContext_FailSilent()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Context_FailSilent;
	}

	static uint8 GetIntConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_IntConst;
	}

	static uint8 GetSkipOffsetConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_SkipOffsetConst;
	}

	static uint8 GetFloatConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_FloatConst;
	}

	static uint8 GetStringConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_StringConst;
	}

	static uint8 GetUnicodeStringConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_UnicodeStringConst;
	}

	static uint8 GetTextConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_TextConst;
	}

	static uint8 GetObjectConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_ObjectConst;
	}

	static uint8 GetSoftObjectConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_SoftObjectConst;
	}

	static uint8 GetFieldPathConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_FieldPathConst;
	}

	static uint8 GetNameConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_NameConst;
	}

	static uint8 GetRotationConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_RotationConst;
	}

	static uint8 GetVectorConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_VectorConst;
	}

	static uint8 GetTransformConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_TransformConst;
	}

	static uint8 GetStructConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_StructConst;
	}

	static uint8 GetSetArray()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_SetArray;
	}

	static uint8 GetArrayConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_ArrayConst;
	}

	static uint8 GetByteConst()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_ByteConst;
	}

	static uint8 GetIntConstByte()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_IntConstByte;
	}

	static uint8 GetMetaCast()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_MetaCast;
	}

	static uint8 GetDynamicCast()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_DynamicCast;
	}

	static uint8 GetJumpIfNot()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_JumpIfNot;
	}

	static uint8 GetAssert()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Assert;
	}

	static uint8 GetSkip()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Skip;
	}

	static uint8 GetInstanceDelegate()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_InstanceDelegate;
	}

	static uint8 GetAddMulticastDelegate()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_AddMulticastDelegate;
	}

	static uint8 GetRemoveMulticastDelegate()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_RemoveMulticastDelegate;
	}

	static uint8 GetClearMulticastDelegate()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_ClearMulticastDelegate;
	}

	static uint8 GetBindDelegate()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_BindDelegate;
	}

	static uint8 GetPushExecutionFlow()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_PushExecutionFlow;
	}

	static uint8 GetPopExecutionFlowIfNot()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_PopExecutionFlowIfNot;
	}

	static uint8 GetBreakpoint()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Breakpoint;
	}

	static uint8 GetWireTracepoint()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_WireTracepoint;
	}

	static uint8 GetInstrumentationEvent()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_InstrumentationEvent;
	}

	static uint8 GetTracepoint()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_Tracepoint;
	}

	static uint8 GetSwitchValue()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_SwitchValue;
	}

	static uint8 GetArrayGetByRef()
	{
		if (Engine_Version == 425)
			return EExprToken425::EX_ArrayGetByRef;
	}
}

std::string GetNameSafe(void* Property)
{
	return GetFNameOfProp(Property)->ToString();
}

void FKismetBytecodeDisassembler::ProcessCommon(int32& ScriptIndex, uint8 Opcode)
{
	using FProperty = void;

	if (Opcode == EExprToken::GetPrimitiveCast())
	{
		// A type conversion.
		uint8 ConversionType = ReadBYTE(ScriptIndex);
		Stream << std::format("{} PrimitiveCast of type {}", Indents, ConversionType);
		AddIndent();

		Stream << std::format("{} Argument:", Indents);
		ProcessCastByte(ConversionType, ScriptIndex);

		//@TODO:
		//Ar.Logf(TEXT("%s Expression:"), *Indents);
		//SerializeExpr( ScriptIndex );
	}
	else if (Opcode == EExprToken::GetSetSet())
	{
		Stream << std::format("{} set set", Indents);
		SerializeExpr(ScriptIndex);
		ReadINT(ScriptIndex);
		while (SerializeExpr(ScriptIndex) != EX_EndSet)
		{
			// Set contents
		}
	}
	else if (Opcode == EExprToken::GetEndSet())
	{
		Stream << std::format("{} EX_EndSet", Indents);
	}
	else if (Opcode == EExprToken::GetSetConst())
	{
		FProperty* InnerProp = ReadPointer<FProperty>(ScriptIndex);
		int32 Num = ReadINT(ScriptIndex);
		Stream << std::format("{} set set const - elements number: {}, inner property: {}", Indents, Num, GetNameSafe(InnerProp));
		while (SerializeExpr(ScriptIndex) != EX_EndSetConst)
		{
			// Set contents
		}
	}
	else if (Opcode == EExprToken::GetEndSetConst())
	{
		Stream << std::format("{} EX_EndSetConst", Indents);
	}
	else if (Opcode == EExprToken::GetSetMap())
	{
		Stream << std::format("{} set map", Indents);
		SerializeExpr(ScriptIndex);
		ReadINT(ScriptIndex);
		while (SerializeExpr(ScriptIndex) != EX_EndMap)
		{
			// Map contents
		}
	}
	else if (Opcode == EExprToken::GetEndMap())
	{
		Stream << std::format("{} EX_EndMap", Indents);
	}
	else if (Opcode == EExprToken::GetMapConst())
	{
		FProperty* KeyProp = ReadPointer<FProperty>(ScriptIndex);
		FProperty* ValProp = ReadPointer<FProperty>(ScriptIndex);
		int32 Num = ReadINT(ScriptIndex);
		Stream << std::format("{} set map const - elements number: {}, key property: {}, val property: {}", Indents, Num, GetNameSafe(KeyProp), GetNameSafe(ValProp));
		while (SerializeExpr(ScriptIndex) != EX_EndMapConst)
		{
			// Map contents
		}
	}
	else if (Opcode == EExprToken::GetEndMapConst())
	{
		Stream << std::format("{} EX_EndMapConst", Indents);
	}
	else if (Opcode == EExprToken::GetObjToInterfaceCast())
	{
		// A conversion from an object variable to a native interface variable.
		// We use a different bytecode to avoid the branching each time we process a cast token

		// the interface class to convert to
		UClass* InterfaceClass = ReadPointer<UClass>(ScriptIndex);
		Stream << std::format("{} ObjToInterfaceCast to {}", Indents, InterfaceClass->GetName());

		SerializeExpr(ScriptIndex);
	}
	else if (Opcode == EExprToken::GetCrossInterfaceCast())
	{
		// A conversion from one interface variable to a different interface variable.
		// We use a different bytecode to avoid the branching each time we process a cast token

		// the interface class to convert to
		UClass* InterfaceClass = ReadPointer<UClass>(ScriptIndex);
		Stream << std::format("{} InterfaceToInterfaceCast to {}", Indents, InterfaceClass->GetName());

		SerializeExpr(ScriptIndex);
	}
	else if (Opcode == EExprToken::GetInterfaceToObjCast())
	{
		// A conversion from an interface variable to a object variable.
		// We use a different bytecode to avoid the branching each time we process a cast token

		// the interface class to convert to
		UClass* ObjectClass = ReadPointer<UClass>(ScriptIndex);
		Stream << std::format("{} InterfaceToObjCast to {}", Indents, ObjectClass->GetName());

		SerializeExpr(ScriptIndex);
	}
	else if (Opcode == EExprToken::GetLet())
	{
		Stream << std::format("{} Let (Variable = Expression)", Indents);
		AddIndent();

		ReadPointer<FProperty>(ScriptIndex);

		// Variable expr.
		Stream << std::format("{} Variable:", Indents);
		SerializeExpr(ScriptIndex);

		// Assignment expr.
		Stream << std::format("{} Expression:", Indents);
		SerializeExpr(ScriptIndex);

		DropIndent();
	}
	else if (Opcode == EExprToken::GetLetObj() || Opcode == EExprToken::GetLetWeakObjectPtr())
	{
		if (Opcode == EExprToken::GetLetObj())
		{
			Stream << std::format("{} Let Obj (Variable = Expression):", Indents);
		}
		else
		{
			Stream << std::format("{} Let WeakObjPtr (Variable = Expression):", Indents);
		}

		AddIndent();

		// Variable expr.
		Stream << std::format("{} Variable:", Indents);
		SerializeExpr(ScriptIndex);

		// Assignment expr.
		Stream << std::format("{} Expression:", Indents);
		SerializeExpr(ScriptIndex);

		DropIndent();
	}
	else if (Opcode == EExprToken::GetLetBool())
	{
		Stream << std::format("{} LetBool (Variable = Expression):", Indents);
		AddIndent();

		// Variable expr.
		Stream << std::format("{} Variable:", Indents);
		SerializeExpr(ScriptIndex);

		// Assignment expr.
		Stream << std::format("{} Expression:", Indents);
		SerializeExpr(ScriptIndex);

		DropIndent();
	}
	/*
	case EX_LetValueOnPersistentFrame:
	{
		Ar.Logf(TEXT("%s $%X: LetValueOnPersistentFrame"), *Indents, (int32)Opcode);
		AddIndent();

		auto Prop = ReadPointer<FProperty>(ScriptIndex);
		Ar.Logf(TEXT("%s Destination variable: %s, offset: %d"), *Indents, *GetNameSafe(Prop),
			Prop ? Prop->GetOffset_ForDebug() : 0);

		Ar.Logf(TEXT("%s Expression:"), *Indents);
		SerializeExpr(ScriptIndex);

		DropIndent();

		break;
	}
	case EX_StructMemberContext:
	{
		Ar.Logf(TEXT("%s $%X: Struct member context "), *Indents, (int32)Opcode);
		AddIndent();

		FProperty* Prop = ReadPointer<FProperty>(ScriptIndex);

		Ar.Logf(TEXT("%s Expression within struct %s, offset %d"), *Indents, *(Prop->GetName()),
			Prop->GetOffset_ForDebug()); // although that isn't a UFunction, we are not going to indirect the props of a struct, so this should be fine

		Ar.Logf(TEXT("%s Expression to struct:"), *Indents);
		SerializeExpr(ScriptIndex);

		DropIndent();

		break;
	}
	case EX_LetDelegate:
	{
		Ar.Logf(TEXT("%s $%X: LetDelegate (Variable = Expression)"), *Indents, (int32)Opcode);
		AddIndent();

		// Variable expr.
		Ar.Logf(TEXT("%s Variable:"), *Indents);
		SerializeExpr(ScriptIndex);

		// Assignment expr.
		Ar.Logf(TEXT("%s Expression:"), *Indents);
		SerializeExpr(ScriptIndex);

		DropIndent();
		break;
	}
	case EX_LocalVirtualFunction:
	{
		FString FunctionName = ReadName(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Local Virtual Script Function named %s"), *Indents, (int32)Opcode, *FunctionName);

		while (SerializeExpr(ScriptIndex) != EX_EndFunctionParms)
		{
		}
		break;
	}
	case EX_LocalFinalFunction:
	{
		UStruct* StackNode = ReadPointer<UStruct>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Local Final Script Function (stack node %s::%s)"), *Indents, (int32)Opcode, StackNode ? *StackNode->GetOuter()->GetName() : TEXT("(null)"), StackNode ? *StackNode->GetName() : TEXT("(null)"));

		while (SerializeExpr(ScriptIndex) != EX_EndFunctionParms)
		{
			// Params
		}
		break;
	}
	case EX_LetMulticastDelegate:
	{
		Ar.Logf(TEXT("%s $%X: LetMulticastDelegate (Variable = Expression)"), *Indents, (int32)Opcode);
		AddIndent();

		// Variable expr.
		Ar.Logf(TEXT("%s Variable:"), *Indents);
		SerializeExpr(ScriptIndex);

		// Assignment expr.
		Ar.Logf(TEXT("%s Expression:"), *Indents);
		SerializeExpr(ScriptIndex);

		DropIndent();
		break;
	}

	case EX_ComputedJump:
	{
		Ar.Logf(TEXT("%s $%X: Computed Jump, offset specified by expression:"), *Indents, (int32)Opcode);

		AddIndent();
		SerializeExpr(ScriptIndex);
		DropIndent();

		break;
	}

	case EX_Jump:
	{
		CodeSkipSizeType SkipCount = ReadSkipCount(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Jump to offset 0x%X"), *Indents, (int32)Opcode, SkipCount);
		break;
	}
	case EX_LocalVariable:
	{
		FProperty* PropertyPtr = ReadPointer<FProperty>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Local variable named %s"), *Indents, (int32)Opcode, PropertyPtr ? *PropertyPtr->GetName() : TEXT("(null)"));
		break;
	}
	case EX_DefaultVariable:
	{
		FProperty* PropertyPtr = ReadPointer<FProperty>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Default variable named %s"), *Indents, (int32)Opcode, PropertyPtr ? *PropertyPtr->GetName() : TEXT("(null)"));
		break;
	}
	case EX_InstanceVariable:
	{
		FProperty* PropertyPtr = ReadPointer<FProperty>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Instance variable named %s"), *Indents, (int32)Opcode, PropertyPtr ? *PropertyPtr->GetName() : TEXT("(null)"));
		break;
	}
	case EX_LocalOutVariable:
	{
		FProperty* PropertyPtr = ReadPointer<FProperty>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Local out variable named %s"), *Indents, (int32)Opcode, PropertyPtr ? *PropertyPtr->GetName() : TEXT("(null)"));
		break;
	}
	case EX_ClassSparseDataVariable:
	{
		FProperty* PropertyPtr = ReadPointer<FProperty>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Class sparse data variable named %s"), *Indents, (int32)Opcode, PropertyPtr ? *PropertyPtr->GetName() : TEXT("(null)"));
		break;
	}
	case EX_InterfaceContext:
	{
		Ar.Logf(TEXT("%s $%X: EX_InterfaceContext:"), *Indents, (int32)Opcode);
		SerializeExpr(ScriptIndex);
		break;
	}
	case EX_DeprecatedOp4A:
	{
		Ar.Logf(TEXT("%s $%X: This opcode has been removed and does nothing."), *Indents, (int32)Opcode);
		break;
	}
	case EX_Nothing:
	{
		Ar.Logf(TEXT("%s $%X: EX_Nothing"), *Indents, (int32)Opcode);
		break;
	}
	case EX_EndOfScript:
	{
		Ar.Logf(TEXT("%s $%X: EX_EndOfScript"), *Indents, (int32)Opcode);
		break;
	}
	case EX_EndFunctionParms:
	{
		Ar.Logf(TEXT("%s $%X: EX_EndFunctionParms"), *Indents, (int32)Opcode);
		break;
	}
	case EX_EndStructConst:
	{
		Ar.Logf(TEXT("%s $%X: EX_EndStructConst"), *Indents, (int32)Opcode);
		break;
	}
	case EX_EndArray:
	{
		Ar.Logf(TEXT("%s $%X: EX_EndArray"), *Indents, (int32)Opcode);
		break;
	}
	case EX_EndArrayConst:
	{
		Ar.Logf(TEXT("%s $%X: EX_EndArrayConst"), *Indents, (int32)Opcode);
		break;
	}
	case EX_IntZero:
	{
		Ar.Logf(TEXT("%s $%X: EX_IntZero"), *Indents, (int32)Opcode);
		break;
	}
	case EX_IntOne:
	{
		Ar.Logf(TEXT("%s $%X: EX_IntOne"), *Indents, (int32)Opcode);
		break;
	}
	case EX_True:
	{
		Ar.Logf(TEXT("%s $%X: EX_True"), *Indents, (int32)Opcode);
		break;
	}
	case EX_False:
	{
		Ar.Logf(TEXT("%s $%X: EX_False"), *Indents, (int32)Opcode);
		break;
	}
	case EX_NoObject:
	{
		Ar.Logf(TEXT("%s $%X: EX_NoObject"), *Indents, (int32)Opcode);
		break;
	}
	case EX_NoInterface:
	{
		Ar.Logf(TEXT("%s $%X: EX_NoObject"), *Indents, (int32)Opcode);
		break;
	}
	case EX_Self:
	{
		Ar.Logf(TEXT("%s $%X: EX_Self"), *Indents, (int32)Opcode);
		break;
	}
	case EX_EndParmValue:
	{
		Ar.Logf(TEXT("%s $%X: EX_EndParmValue"), *Indents, (int32)Opcode);
		break;
	}
	case EX_Return:
	{
		Ar.Logf(TEXT("%s $%X: Return expression"), *Indents, (int32)Opcode);

		SerializeExpr(ScriptIndex); // Return expression.
		break;
	}
	case EX_CallMath:
	{
		UStruct* StackNode = ReadPointer<UStruct>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Call Math (stack node %s::%s)"), *Indents, (int32)Opcode, *GetNameSafe(StackNode ? StackNode->GetOuter() : nullptr), *GetNameSafe(StackNode));

		while (SerializeExpr(ScriptIndex) != EX_EndFunctionParms)
		{
			// Params
		}
		break;
	}
	case EX_FinalFunction:
	{
		UStruct* StackNode = ReadPointer<UStruct>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Final Function (stack node %s::%s)"), *Indents, (int32)Opcode, StackNode ? *StackNode->GetOuter()->GetName() : TEXT("(null)"), StackNode ? *StackNode->GetName() : TEXT("(null)"));

		while (SerializeExpr(ScriptIndex) != EX_EndFunctionParms)
		{
			// Params
		}
		break;
	}
	case EX_CallMulticastDelegate:
	{
		UStruct* StackNode = ReadPointer<UStruct>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: CallMulticastDelegate (signature %s::%s) delegate:"), *Indents, (int32)Opcode, StackNode ? *StackNode->GetOuter()->GetName() : TEXT("(null)"), StackNode ? *StackNode->GetName() : TEXT("(null)"));
		SerializeExpr(ScriptIndex);
		Ar.Logf(TEXT("Params:"));
		while (SerializeExpr(ScriptIndex) != EX_EndFunctionParms)
		{
			// Params
		}
		break;
	}
	case EX_VirtualFunction:
	{
		FString FunctionName = ReadName(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: Virtual Function named %s"), *Indents, (int32)Opcode, *FunctionName);

		while (SerializeExpr(ScriptIndex) != EX_EndFunctionParms)
		{
		}
		break;
	}
	case EX_ClassContext:
	case EX_Context:
	case EX_Context_FailSilent:
	{
		Ar.Logf(TEXT("%s $%X: %s"), *Indents, (int32)Opcode, Opcode == EX_ClassContext ? TEXT("Class Context") : TEXT("Context"));
		AddIndent();

		// Object expression.
		Ar.Logf(TEXT("%s ObjectExpression:"), *Indents);
		SerializeExpr(ScriptIndex);

		if (Opcode == EX_Context_FailSilent)
		{
			Ar.Logf(TEXT(" Can fail silently on access none "));
		}

		// Code offset for NULL expressions.
		CodeSkipSizeType SkipCount = ReadSkipCount(ScriptIndex);
		Ar.Logf(TEXT("%s Skip Bytes: 0x%X"), *Indents, SkipCount);

		// Property corresponding to the r-value data, in case the l-value needs to be mem-zero'd
		FField* Field = ReadPointer<FField>(ScriptIndex);
		Ar.Logf(TEXT("%s R-Value Property: %s"), *Indents, Field ? *Field->GetName() : TEXT("(null)"));

		// Context expression.
		Ar.Logf(TEXT("%s ContextExpression:"), *Indents);
		SerializeExpr(ScriptIndex);

		DropIndent();
		break;
	}
	case EX_IntConst:
	{
		int32 ConstValue = ReadINT(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: literal int32 %d"), *Indents, (int32)Opcode, ConstValue);
		break;
	}
	case EX_SkipOffsetConst:
	{
		CodeSkipSizeType ConstValue = ReadSkipCount(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: literal CodeSkipSizeType 0x%X"), *Indents, (int32)Opcode, ConstValue);
		break;
	}
	case EX_FloatConst:
	{
		float ConstValue = ReadFLOAT(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: literal float %f"), *Indents, (int32)Opcode, ConstValue);
		break;
	}
	case EX_StringConst:
	{
		FString ConstValue = ReadString8(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: literal ansi string \"%s\""), *Indents, (int32)Opcode, *ConstValue);
		break;
	}
	case EX_UnicodeStringConst:
	{
		FString ConstValue = ReadString16(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: literal unicode string \"%s\""), *Indents, (int32)Opcode, *ConstValue);
		break;
	}
	case EX_TextConst:
	{
		// What kind of text are we dealing with?
		const EBlueprintTextLiteralType TextLiteralType = (EBlueprintTextLiteralType)Script[ScriptIndex++];

		switch (TextLiteralType)
		{
		case EBlueprintTextLiteralType::Empty:
		{
			Ar.Logf(TEXT("%s $%X: literal text - empty"), *Indents, (int32)Opcode);
		}
		break;

		case EBlueprintTextLiteralType::LocalizedText:
		{
			const FString SourceString = ReadString(ScriptIndex);
			const FString KeyString = ReadString(ScriptIndex);
			const FString Namespace = ReadString(ScriptIndex);
			Ar.Logf(TEXT("%s $%X: literal text - localized text { namespace: \"%s\", key: \"%s\", source: \"%s\" }"), *Indents, (int32)Opcode, *Namespace, *KeyString, *SourceString);
		}
		break;

		case EBlueprintTextLiteralType::InvariantText:
		{
			const FString SourceString = ReadString(ScriptIndex);
			Ar.Logf(TEXT("%s $%X: literal text - invariant text: \"%s\""), *Indents, (int32)Opcode, *SourceString);
		}
		break;

		case EBlueprintTextLiteralType::LiteralString:
		{
			const FString SourceString = ReadString(ScriptIndex);
			Ar.Logf(TEXT("%s $%X: literal text - literal string: \"%s\""), *Indents, (int32)Opcode, *SourceString);
		}
		break;

		case EBlueprintTextLiteralType::StringTableEntry:
		{
			ReadPointer<UObject>(ScriptIndex); // String Table asset (if any)
			const FString TableIdString = ReadString(ScriptIndex);
			const FString KeyString = ReadString(ScriptIndex);
			Ar.Logf(TEXT("%s $%X: literal text - string table entry { tableid: \"%s\", key: \"%s\" }"), *Indents, (int32)Opcode, *TableIdString, *KeyString);
		}
		break;

		default:
			checkf(false, TEXT("Unknown EBlueprintTextLiteralType! Please update FKismetBytecodeDisassembler::ProcessCommon to handle this type of text."));
			break;
		}
		break;
	}
	case EX_ObjectConst:
	{
		UObject* Pointer = ReadPointer<UObject>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: EX_ObjectConst (%p:%s)"), *Indents, (int32)Opcode, Pointer, *Pointer->GetFullName());
		break;
	}
	case EX_SoftObjectConst:
	{
		Ar.Logf(TEXT("%s $%X: EX_SoftObjectConst"), *Indents, (int32)Opcode);
		SerializeExpr(ScriptIndex);
		break;
	}
	case EX_FieldPathConst:
	{
		Ar.Logf(TEXT("%s $%X: EX_FieldPathConst"), *Indents, (int32)Opcode);
		SerializeExpr(ScriptIndex);
		break;
	}
	case EX_NameConst:
	{
		FString ConstValue = ReadName(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: literal name %s"), *Indents, (int32)Opcode, *ConstValue);
		break;
	}
	case EX_RotationConst:
	{
		float Pitch = ReadFLOAT(ScriptIndex);
		float Yaw = ReadFLOAT(ScriptIndex);
		float Roll = ReadFLOAT(ScriptIndex);

		Ar.Logf(TEXT("%s $%X: literal rotation (%f,%f,%f)"), *Indents, (int32)Opcode, Pitch, Yaw, Roll);
		break;
	}
	case EX_VectorConst:
	{
		float X = ReadFLOAT(ScriptIndex);
		float Y = ReadFLOAT(ScriptIndex);
		float Z = ReadFLOAT(ScriptIndex);

		Ar.Logf(TEXT("%s $%X: literal vector (%f,%f,%f)"), *Indents, (int32)Opcode, X, Y, Z);
		break;
	}
	case EX_TransformConst:
	{

		float RotX = ReadFLOAT(ScriptIndex);
		float RotY = ReadFLOAT(ScriptIndex);
		float RotZ = ReadFLOAT(ScriptIndex);
		float RotW = ReadFLOAT(ScriptIndex);

		float TransX = ReadFLOAT(ScriptIndex);
		float TransY = ReadFLOAT(ScriptIndex);
		float TransZ = ReadFLOAT(ScriptIndex);

		float ScaleX = ReadFLOAT(ScriptIndex);
		float ScaleY = ReadFLOAT(ScriptIndex);
		float ScaleZ = ReadFLOAT(ScriptIndex);

		Ar.Logf(TEXT("%s $%X: literal transform R(%f,%f,%f,%f) T(%f,%f,%f) S(%f,%f,%f)"), *Indents, (int32)Opcode, TransX, TransY, TransZ, RotX, RotY, RotZ, RotW, ScaleX, ScaleY, ScaleZ);
		break;
	}
	case EX_StructConst:
	{
		UScriptStruct* Struct = ReadPointer<UScriptStruct>(ScriptIndex);
		int32 SerializedSize = ReadINT(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: literal struct %s (serialized size: %d)"), *Indents, (int32)Opcode, *Struct->GetName(), SerializedSize);
		while (SerializeExpr(ScriptIndex) != EX_EndStructConst)
		{
			// struct contents
		}
		break;
	}
	case EX_SetArray:
	{
		Ar.Logf(TEXT("%s $%X: set array"), *Indents, (int32)Opcode);
		SerializeExpr(ScriptIndex);
		while (SerializeExpr(ScriptIndex) != EX_EndArray)
		{
			// Array contents
		}
		break;
	}
	case EX_ArrayConst:
	{
		FProperty* InnerProp = ReadPointer<FProperty>(ScriptIndex);
		int32 Num = ReadINT(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: set array const - elements number: %d, inner property: %s"), *Indents, (int32)Opcode, Num, *GetNameSafe(InnerProp));
		while (SerializeExpr(ScriptIndex) != EX_EndArrayConst)
		{
			// Array contents
		}
		break;
	}
	case EX_ByteConst:
	{
		uint8 ConstValue = ReadBYTE(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: literal byte %d"), *Indents, (int32)Opcode, ConstValue);
		break;
	}
	case EX_IntConstByte:
	{
		int32 ConstValue = ReadBYTE(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: literal int %d"), *Indents, (int32)Opcode, ConstValue);
		break;
	}
	case EX_MetaCast:
	{
		UClass* Class = ReadPointer<UClass>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: MetaCast to %s of expr:"), *Indents, (int32)Opcode, *Class->GetName());
		SerializeExpr(ScriptIndex);
		break;
	}
	case EX_DynamicCast:
	{
		UClass* Class = ReadPointer<UClass>(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: DynamicCast to %s of expr:"), *Indents, (int32)Opcode, *Class->GetName());
		SerializeExpr(ScriptIndex);
		break;
	}
	case EX_JumpIfNot:
	{
		// Code offset.
		CodeSkipSizeType SkipCount = ReadSkipCount(ScriptIndex);

		Ar.Logf(TEXT("%s $%X: Jump to offset 0x%X if not expr:"), *Indents, (int32)Opcode, SkipCount);

		// Boolean expr.
		SerializeExpr(ScriptIndex);
		break;
	}
	case EX_Assert:
	{
		uint16 LineNumber = ReadWORD(ScriptIndex);
		uint8 InDebugMode = ReadBYTE(ScriptIndex);

		Ar.Logf(TEXT("%s $%X: assert at line %d, in debug mode = %d with expr:"), *Indents, (int32)Opcode, LineNumber, InDebugMode);
		SerializeExpr(ScriptIndex); // Assert expr.
		break;
	}
	case EX_Skip:
	{
		CodeSkipSizeType W = ReadSkipCount(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: possibly skip 0x%X bytes of expr:"), *Indents, (int32)Opcode, W);

		// Expression to possibly skip.
		SerializeExpr(ScriptIndex);

		break;
	}
	case EX_InstanceDelegate:
	{
		// the name of the function assigned to the delegate.
		FString FuncName = ReadName(ScriptIndex);

		Ar.Logf(TEXT("%s $%X: instance delegate function named %s"), *Indents, (int32)Opcode, *FuncName);
		break;
	}
	case EX_AddMulticastDelegate:
	{
		Ar.Logf(TEXT("%s $%X: Add MC delegate"), *Indents, (int32)Opcode);
		SerializeExpr(ScriptIndex);
		SerializeExpr(ScriptIndex);
		break;
	}
	case EX_RemoveMulticastDelegate:
	{
		Ar.Logf(TEXT("%s $%X: Remove MC delegate"), *Indents, (int32)Opcode);
		SerializeExpr(ScriptIndex);
		SerializeExpr(ScriptIndex);
		break;
	}
	case EX_ClearMulticastDelegate:
	{
		Ar.Logf(TEXT("%s $%X: Clear MC delegate"), *Indents, (int32)Opcode);
		SerializeExpr(ScriptIndex);
		break;
	}
	case EX_BindDelegate:
	{
		// the name of the function assigned to the delegate.
		FString FuncName = ReadName(ScriptIndex);

		Ar.Logf(TEXT("%s $%X: BindDelegate '%s' "), *Indents, (int32)Opcode, *FuncName);

		Ar.Logf(TEXT("%s Delegate:"), *Indents);
		SerializeExpr(ScriptIndex);

		Ar.Logf(TEXT("%s Object:"), *Indents);
		SerializeExpr(ScriptIndex);

		break;
	}
	case EX_PushExecutionFlow:
	{
		CodeSkipSizeType SkipCount = ReadSkipCount(ScriptIndex);
		Ar.Logf(TEXT("%s $%X: FlowStack.Push(0x%X);"), *Indents, (int32)Opcode, SkipCount);
		break;
	}
	case EX_PopExecutionFlow:
	{
		Ar.Logf(TEXT("%s $%X: if (FlowStack.Num()) { jump to statement at FlowStack.Pop(); } else { ERROR!!! }"), *Indents, (int32)Opcode);
		break;
	}
	case EX_PopExecutionFlowIfNot:
	{
		Ar.Logf(TEXT("%s $%X: if (!condition) { if (FlowStack.Num()) { jump to statement at FlowStack.Pop(); } else { ERROR!!! } }"), *Indents, (int32)Opcode);
		// Boolean expr.
		SerializeExpr(ScriptIndex);
		break;
	}
	case EX_Breakpoint:
	{
		Ar.Logf(TEXT("%s $%X: <<< BREAKPOINT >>>"), *Indents, (int32)Opcode);
		break;
	}
	case EX_WireTracepoint:
	{
		Ar.Logf(TEXT("%s $%X: .. wire debug site .."), *Indents, (int32)Opcode);
		break;
	}
	case EX_InstrumentationEvent:
	{
		const uint8 EventType = ReadBYTE(ScriptIndex);
		switch (EventType)
		{
		case EScriptInstrumentation::InlineEvent:
			Ar.Logf(TEXT("%s $%X: .. instrumented inline event .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::Stop:
			Ar.Logf(TEXT("%s $%X: .. instrumented event stop .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::PureNodeEntry:
			Ar.Logf(TEXT("%s $%X: .. instrumented pure node entry site .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::NodeDebugSite:
			Ar.Logf(TEXT("%s $%X: .. instrumented debug site .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::NodeEntry:
			Ar.Logf(TEXT("%s $%X: .. instrumented wire entry site .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::NodeExit:
			Ar.Logf(TEXT("%s $%X: .. instrumented wire exit site .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::PushState:
			Ar.Logf(TEXT("%s $%X: .. push execution state .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::RestoreState:
			Ar.Logf(TEXT("%s $%X: .. restore execution state .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::ResetState:
			Ar.Logf(TEXT("%s $%X: .. reset execution state .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::SuspendState:
			Ar.Logf(TEXT("%s $%X: .. suspend execution state .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::PopState:
			Ar.Logf(TEXT("%s $%X: .. pop execution state .."), *Indents, (int32)Opcode);
			break;
		case EScriptInstrumentation::TunnelEndOfThread:
			Ar.Logf(TEXT("%s $%X: .. tunnel end of thread .."), *Indents, (int32)Opcode);
			break;
		}
		break;
	}
	case EX_Tracepoint:
	{
		Ar.Logf(TEXT("%s $%X: .. debug site .."), *Indents, (int32)Opcode);
		break;
	}
	case EX_SwitchValue:
	{
		const auto NumCases = ReadWORD(ScriptIndex);
		const auto AfterSkip = ReadSkipCount(ScriptIndex);

		Ar.Logf(TEXT("%s $%X: Switch Value %d cases, end in 0x%X"), *Indents, (int32)Opcode, NumCases, AfterSkip);
		AddIndent();
		Ar.Logf(TEXT("%s Index:"), *Indents);
		SerializeExpr(ScriptIndex);

		for (uint16 CaseIndex = 0; CaseIndex < NumCases; ++CaseIndex)
		{
			Ar.Logf(TEXT("%s [%d] Case Index (label: 0x%X):"), *Indents, CaseIndex, ScriptIndex);
			SerializeExpr(ScriptIndex);	// case index value term
			const auto OffsetToNextCase = ReadSkipCount(ScriptIndex);
			Ar.Logf(TEXT("%s [%d] Offset to the next case: 0x%X"), *Indents, CaseIndex, OffsetToNextCase);
			Ar.Logf(TEXT("%s [%d] Case Result:"), *Indents, CaseIndex);
			SerializeExpr(ScriptIndex);	// case term
		}

		Ar.Logf(TEXT("%s Default result (label: 0x%X):"), *Indents, ScriptIndex);
		SerializeExpr(ScriptIndex);
		Ar.Logf(TEXT("%s (label: 0x%X)"), *Indents, ScriptIndex);
		DropIndent();
		break;
	}
	case EX_ArrayGetByRef:
	{
		Ar.Logf(TEXT("%s $%X: Array Get-by-Ref Index"), *Indents, (int32)Opcode);
		AddIndent();
		SerializeExpr(ScriptIndex);
		SerializeExpr(ScriptIndex);
		DropIndent();
		break;
	}
	default:
	{
		// This should never occur.
		// UE_LOG(LogScriptDisassembler, Warning, TEXT("Unknown bytecode 0x%02X; ignoring it"), (uint8)Opcode);
		break;
	}
	}
	*/
}