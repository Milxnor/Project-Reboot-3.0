#pragma once

#include <functional>
#include <MinHook/MinHook.h>

#include "memcury.h"
#include "Class.h"

#include "reboot.h"

struct FunctionHooks
{
    void* Original;
    void* Detour;
    bool IsHooked;
    std::string Name;
    int Index = -1;
    void** VFT = nullptr;
};

static inline std::vector<FunctionHooks> AllFunctionHooks;

inline void PatchByte(uint64 addr, uint8_t byte)
{
    DWORD dwProtection;
    VirtualProtect((PVOID)addr, 1, PAGE_EXECUTE_READWRITE, &dwProtection);

    *(uint8_t*)addr = byte;

    DWORD dwTemp;
    VirtualProtect((PVOID)addr, 1, dwProtection, &dwTemp);
}

inline void PatchBytes(uint64 addr, const std::vector<uint8_t>& Bytes)
{
    if (!addr)
        return;

    for (int i = 0; i < Bytes.size(); i++)
    {
        PatchByte(addr + i, Bytes.at(i));
    }
}

inline __int64 GetFunctionIdxOrPtr2(UFunction* Function)
{
    auto NativeAddr = __int64(Function->GetFunc());

    auto FuncName = Function->GetName();

    std::wstring ValidateWStr = (std::wstring(FuncName.begin(), FuncName.end()) + L"_Validate");
    const wchar_t* ValidateWCStr = ValidateWStr.c_str();
    bool bHasValidateFunc = Memcury::Scanner::FindStringRef(ValidateWCStr, false).Get();

    bool bFoundValidate = !bHasValidateFunc;

    __int64 RetAddr = 0;

    for (int i = 0; i < 2000; i++)
    {
        // std::cout << std::format("CURRENT 0x{:x}\n", __int64((uint8_t*)(NativeAddr + i)) - __int64(GetModuleHandleW(0)));

        if (!RetAddr && *(uint8_t*)(NativeAddr + i) == 0xC3)
        {
            RetAddr = NativeAddr + i;
            break;
        }
    }

    int i = 0;

    __int64 functionAddyOrOffset = 0;

    for (__int64 CurrentAddy = RetAddr; CurrentAddy != NativeAddr && i < 2000; CurrentAddy -= 1) // Find last call
    {
        // LOG_INFO(LogDev, "[{}] 0x{:x}", i, *(uint8_t*)CurrentAddy);

        /* if (*(uint8_t*)CurrentAddy == 0xE8) // BAD
        {
            // LOG_INFO(LogDev, "CurrentAddy 0x{:x}", CurrentAddy - __int64(GetModuleHandleW(0)));
            functionAddyOrOffset = (CurrentAddy + 1 + 4) + *(int*)(CurrentAddy + 1);
            break;
        }

        else */ if ((*(uint8_t*)(CurrentAddy) == 0xFF && *(uint8_t*)(CurrentAddy + 1) == 0x90) ||
            *(uint8_t*)(CurrentAddy) == 0xFF && *(uint8_t*)(CurrentAddy + 1) == 0x93)
        {
            auto SecondByte = *(uint8_t*)(CurrentAddy + 2);
            auto ThirdByte = *(uint8_t*)(CurrentAddy + 3);

            std::string bytes = GetBytes(CurrentAddy + 2, 2);

            std::string last2bytes;
            last2bytes += bytes[3];
            last2bytes += bytes[4];

            std::string neww;

            if (last2bytes != "00")
                neww = last2bytes;

            neww += bytes[0];
            neww += bytes[1];
            bytes = neww;

            functionAddyOrOffset = HexToDec(bytes);
            break;
        }

        i++;
    }

    return functionAddyOrOffset;
}

inline __int64 GetIndexFromVirtualFunctionCall(__int64 NativeAddr)
{
    std::string wtf = "";

    int shots = 0;

    bool bFoundFirstNumber = false;

    for (__int64 z = (NativeAddr + 5); z != (NativeAddr + 1); z -= 1)
    {
        auto anafa = (int)(*(uint8_t*)z);

        auto asfk = anafa < 10 ? "0" + std::format("{:x}", anafa) : std::format("{:x}", anafa);

        // std::cout << std::format("[{}] 0x{}\n", shots, asfk);

        if (*(uint8_t*)z == 0 ? bFoundFirstNumber : true)
        {
            wtf += asfk;
            bFoundFirstNumber = true;
        }

        shots++;
    }

    std::transform(wtf.begin(), wtf.end(), wtf.begin(), ::toupper);

    // LOG_INFO(LogDev, "wtf: {}", wtf);

    return HexToDec(wtf);
}

inline __int64 GetFunctionIdxOrPtr(UFunction* Function, bool bBreakWhenHitRet = false)
{
    if (!Function)
        return 0;

    auto NativeAddr = __int64(Function->GetFunc());

    auto FuncName = Function->GetName();

    std::wstring ValidateWStr = (std::wstring(FuncName.begin(), FuncName.end()) + L"_Validate");
    const wchar_t* ValidateWCStr = ValidateWStr.c_str();
    bool bHasValidateFunc = Memcury::Scanner::FindStringRef(ValidateWCStr, false).Get();

    // LOG_INFO(LogDev, "[{}] bHasValidateFunc: {}", Function->GetName(), bHasValidateFunc);
    // LOG_INFO(LogDev, "NativeAddr: 0x{:x}", __int64(NativeAddr) - __int64(GetModuleHandleW(0)));

    bool bFoundValidate = !bHasValidateFunc;

    __int64 RetAddr = 0;

    for (int i = 0; i < 2000; i++)
    {
        // LOG_INFO(LogDev, "0x{:x} {}", *(uint8_t*)(NativeAddr + i), bFoundValidate);

        if (Fortnite_Version >= 19) // We should NOT do this, instead, if we expect a validate and we don't find before C3, then search for 0x41 0xFF.
        {
            if ((*(uint8_t*)(NativeAddr + i) == 0x41 && *(uint8_t*)(NativeAddr + i + 1) == 0xFF)) // wtf s18+
            {
                LOG_INFO(LogDev, "Uhhhhhh report this to milxnor if u not on s19+ {}", Function->GetName());
                bFoundValidate = true;
                continue;
            }
        }

        if ((*(uint8_t*)(NativeAddr + i) == 0xFF && *(uint8_t*)(NativeAddr + i + 1) == 0x90) || // call qword ptr
            (*(uint8_t*)(NativeAddr + i) == 0xFF && *(uint8_t*)(NativeAddr + i + 1) == 0x93)) // call qword ptr
        {
            if (bFoundValidate)
            {
                return GetIndexFromVirtualFunctionCall(NativeAddr + i);
            }
            else
            {
                bFoundValidate = true;
                continue;
            }
        }

        if (*(uint8_t*)(NativeAddr + i) == 0x48 && *(uint8_t*)(NativeAddr + i + 1) == 0xFF && *(uint8_t*)(NativeAddr + i + 2) == 0xA0) // jmp qword ptr
        {
            if (bFoundValidate)
            {
                std::string wtf = "";

                int shots = 0;

                bool bFoundFirstNumber = false;

                for (__int64 z = (NativeAddr + i + 6); z != (NativeAddr + i + 2); z -= 1)
                {
                    auto anafa = (int)(*(uint8_t*)z);

                    auto asfk = anafa < 10 ? "0" + std::format("{:x}", anafa) : std::format("{:x}", anafa);

                    // std::cout << std::format("[{}] 0x{}\n", shots, asfk);

                    if (*(uint8_t*)z == 0 ? bFoundFirstNumber : true)
                    {
                        wtf += asfk;
                        bFoundFirstNumber = true;
                    }

                    shots++;
                }

                std::transform(wtf.begin(), wtf.end(), wtf.begin(), ::toupper);

                // LOG_INFO(LogDev, "wtf: {}", wtf);

                return HexToDec(wtf);
            }
        }

        if (!RetAddr && *(uint8_t*)(NativeAddr + i) == 0xC3)
        {
            RetAddr = NativeAddr + i;

            if (bBreakWhenHitRet)
                break;
        }
    }

    // The function isn't virtual

    __int64 functionAddy = 0;

    // LOG_INFO(LogDev, "not virtgual");

    if (RetAddr)
    {
        // LOG_INFO(LogDev, "RetAddr 0x{:x}", RetAddr - __int64(GetModuleHandleW(0)));

        int i = 0;

        for (__int64 CurrentAddy = RetAddr; CurrentAddy != NativeAddr && i < 2000; CurrentAddy -= 1) // Find last call
        {
            // LOG_INFO(LogDev, "[{}] 0x{:x}", i, *(uint8_t*)CurrentAddy);

            if (*(uint8_t*)CurrentAddy == 0xE8)
            {
                // LOG_INFO(LogDev, "CurrentAddy 0x{:x}", CurrentAddy - __int64(GetModuleHandleW(0)));
                functionAddy = (CurrentAddy + 1 + 4) + *(int*)(CurrentAddy + 1);
                break;
            }

            i++;
        }
    }

    return !functionAddy ? -1 : functionAddy;
}

namespace Hooking
{
	namespace MinHook
	{
		static bool Hook(void* Addr, void* Detour, void** Original = nullptr, std::string OptionalName = "Undefined")
		{
            LOG_INFO(LogDev, "Hooking 0x{:x}", __int64(Addr) - __int64(GetModuleHandleW(0)));
            void* Og;
			auto ret1 = MH_CreateHook(Addr, Detour, &Og);
			auto ret2 = MH_EnableHook(Addr);

            if (Original)
                *Original = Og;

            bool wasHookSuccessful = ret1 == MH_OK && ret2 == MH_OK;

            if (wasHookSuccessful)
                AllFunctionHooks.push_back(FunctionHooks(Og, Detour, true, OptionalName));

			return wasHookSuccessful;
		}

        static bool PatchCall(void* Addr, void* Detour/*, void** Original = nullptr*/)
        {
            // int64_t delta = targetAddr - (instrAddr + 5);
            // *(int32_t*)(instrAddr + 1) = static_cast<int32_t>(delta);
        }

        static bool Hook(UObject* DefaultClass, UFunction* Function, void* Detour, void** Original = nullptr, bool bUseSecondMethod = true, bool bHookExec = false, bool bOverride = true, bool bBreakWhenRet = false) // Native hook
		{
            if (!bOverride)
                return false;

            if (!Function)
                return false;

            auto FunctionName = Function->GetName();

            if (!DefaultClass || !DefaultClass->VFTable)
            {
                LOG_WARN(LogHook, "DefaultClass or the vtable for function {} is null! ({})", FunctionName, __int64(DefaultClass));
                return false;
            }

			auto& Exec = Function->GetFunc();

            if (bHookExec)
            {
                LOG_INFO(LogDev, "Hooking Exec {} at 0x{:x}", FunctionName, __int64(Exec) - __int64(GetModuleHandleW(0)));

                if (Original)
                    *Original = Exec;

                Exec = Detour;
                return true;
            }

            auto AddrOrIdx = bUseSecondMethod ? GetFunctionIdxOrPtr2(Function) : GetFunctionIdxOrPtr(Function, bBreakWhenRet);

            if (AddrOrIdx == -1)
            {
                LOG_ERROR(LogInit, "Failed to find anything for {}.", FunctionName);
                return false;
            }

            if (IsBadReadPtr((void*)AddrOrIdx))
            {
                auto Idx = AddrOrIdx / 8;

                AllFunctionHooks.push_back(FunctionHooks(DefaultClass->VFTable[Idx], Detour, true, FunctionName, Idx, DefaultClass->VFTable));

                if (Original)
                    *Original = DefaultClass->VFTable[Idx];

                LOG_INFO(LogDev, "Hooking {} with Idx 0x{:x} (0x{:x})", FunctionName, AddrOrIdx, __int64(DefaultClass->VFTable[Idx]) - __int64(GetModuleHandleW(0)));

                VirtualSwap(DefaultClass->VFTable, Idx, Detour); // we should loop thrugh all objects and check if they inherit from the DefaultClass if so also swap that

                return true;
            }

			return Hook((PVOID)AddrOrIdx, Detour, Original, FunctionName);
		}

        static bool Unhook(void* Addr)
		{
			return MH_DisableHook((PVOID)Addr) == MH_OK;
		}

        /* static bool Unhook(void** Addr, void* Original) // I got brain damaged
        {
            Unhook(Addr);
            *Addr = Original;
        } */
	}
}

static inline void ChangeBytesThing(uint8_t* instrAddr, uint8_t* DetourAddr, int Offset)
{
    int64_t delta = DetourAddr - (instrAddr + Offset + 4);
    auto addr = (int32_t*)(instrAddr + Offset);
    DWORD dwProtection;
    VirtualProtect((PVOID)addr, 4, PAGE_EXECUTE_READWRITE, &dwProtection);

    *addr = static_cast<int32_t>(delta);

    DWORD dwTemp;
    VirtualProtect((PVOID)addr, 1, dwProtection, &dwTemp);
}

enum ERelativeOffsets
{
    CALL = 1,
    LEA = 3
};

static inline void HookInstruction(uint64 instrAddr, void* Detour, const std::string& FunctionToReplace, ERelativeOffsets Offset, UObject* DefaultClass = nullptr) // we need better name
{
    if (!instrAddr)
        return;

    auto UFunc = FindObject<UFunction>(FunctionToReplace);

    uint64 FunctionAddr = __int64(UFunc->GetFunc()); // GetFunctionIdxOrPtr(FindObject<UFunction>(FunctionToReplace));

    if (IsBadReadPtr((void*)FunctionAddr))
    {
        auto Idx = FunctionAddr / 8;

        FunctionAddr = (uint64)DefaultClass->VFTable[Idx];
    }

    if (__int64(instrAddr) - FunctionAddr < 0) // We do not want the FunctionAddr (detour) to be less than where we are replacing.
    {
        LOG_ERROR(LogDev, "Hooking Instruction will not work! Function is after ({})!", FunctionToReplace);
        return;
    }

    Hooking::MinHook::Hook((PVOID)FunctionAddr, Detour, nullptr, FunctionToReplace);

    ChangeBytesThing((uint8_t*)instrAddr, (uint8_t*)FunctionAddr, (int)Offset);
}