#pragma once

#include <functional>
#include <MinHook/MinHook.h>

#include "memcury.h"
#include "Class.h"

inline __int64 GetFunctionIdxOrPtr2(UFunction* Function)
{
    auto NativeAddr = __int64(Function->GetFunc());

    auto FuncName = Function->GetName();

    std::cout << std::format("{} Exec: 0x{:x}\n", Function->GetName(), NativeAddr - __int64(GetModuleHandleW(0)));

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

    std::cout << std::format("RETT {}: 0x{:x}\n", Function->GetName(), RetAddr - __int64(GetModuleHandleW(0)));

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
            std::cout << "found vcall!\n";

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

    std::cout << "FOUND: " << functionAddyOrOffset << '\n';

    return functionAddyOrOffset;
}


inline __int64 GetFunctionIdxOrPtr(UFunction* Function)
{
    auto NativeAddr = __int64(Function->GetFunc());

    std::cout << std::format("{} Exec: 0x{:x}\n", Function->GetName(), NativeAddr - __int64(GetModuleHandleW(0)));

    auto FuncName = Function->GetName();

    std::wstring ValidateWStr = (std::wstring(FuncName.begin(), FuncName.end()) + L"_Validate");
    const wchar_t* ValidateWCStr = ValidateWStr.c_str();
    bool bHasValidateFunc = Memcury::Scanner::FindStringRef(ValidateWCStr, false).Get();

    LOG_INFO(LogDev, "[{}] bHasValidateFunc: {}", Function->GetName(), bHasValidateFunc);
    LOG_INFO(LogDev, "NativeAddr: 0x{:x}", __int64(NativeAddr) - __int64(GetModuleHandleW(0)));

    bool bFoundValidate = !bHasValidateFunc;

    __int64 RetAddr = 0;

    for (int i = 0; i < 2000; i++)
    {
        // LOG_INFO(LogDev, "0x{:x}", *(uint8_t*)(NativeAddr + i));

        if ((*(uint8_t*)(NativeAddr + i) == 0xFF && *(uint8_t*)(NativeAddr + i + 1) == 0x90) ||
            *(uint8_t*)(NativeAddr + i) == 0xFF && *(uint8_t*)(NativeAddr + i + 1) == 0x93)
        {
            if (bFoundValidate)
            {
                std::string wtf = "";

                int shots = 0;

                bool bFoundFirstNumber = false;

                for (__int64 z = (NativeAddr + i + 5); z != (NativeAddr + i + 1); z -= 1)
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

                LOG_INFO(LogDev, "wtf: {}", wtf);

                return HexToDec(wtf);
            }
            else
            {
                bFoundValidate = true;
                continue;
            }
        }

        if (!RetAddr && *(uint8_t*)(NativeAddr + i) == 0xC3)
        {
            RetAddr = NativeAddr + i;
            // break;
        }
    }

    // The function isn't virtual

    __int64 functionAddy = 0;

    if (RetAddr)
    {
        LOG_INFO(LogDev, "RetAddr 0x{:x}", RetAddr - __int64(GetModuleHandleW(0)));

        int i = 0;

        for (__int64 CurrentAddy = RetAddr; CurrentAddy != NativeAddr && i < 2000; CurrentAddy -= 1) // Find last call
        {
            LOG_INFO(LogDev, "[{}] 0x{:x}", i, *(uint8_t*)CurrentAddy);

            if (*(uint8_t*)CurrentAddy == 0xE8)
            {
                LOG_INFO(LogDev, "CurrentAddy 0x{:x}", CurrentAddy - __int64(GetModuleHandleW(0)));
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
		static bool Hook(void* Addr, void* Detour, void** Original = nullptr)
		{
			auto ret1 = MH_CreateHook(Addr, Detour, Original);
			auto ret2 = MH_EnableHook(Addr);
			return ret1 == MH_OK && ret2 == MH_OK;
		}

        static bool Hook(UObject* DefaultClass, UFunction* Function, void* Detour, void** Original = nullptr, bool bUseSecondMethod = true, bool bHookExec = false) // Native hook
		{
			auto Exec = Function->GetFunc();

			if (bHookExec)
				return Hook(Exec, Detour, Original);

            auto AddrOrIdx = bUseSecondMethod ? GetFunctionIdxOrPtr2(Function) : GetFunctionIdxOrPtr(Function);

            if (AddrOrIdx == -1)
            {
                LOG_ERROR(LogInit, "Failed to find anything for {}.", Function->GetName());
                return false;
            }

            if (IsBadReadPtr((void*)AddrOrIdx))
            {
                auto Idx = AddrOrIdx / 8;

                if (Original)
                    *Original = DefaultClass->VFTable[Idx];

                VirtualSwap(DefaultClass->VFTable, Idx, Detour);

                return true;
            }

			return Hook((PVOID)AddrOrIdx, Detour, Original);
		}

        static bool Unhook(void* Addr)
		{
			return MH_DisableHook((PVOID)Addr) == MH_OK;
		}
	}

    static bool Hook(UFunction* Function, std::function<void(UObject*, void*)> Detour) // ProcessEvent hook
	{
		return true;
	}
}