    // https://github.com/kem0x/memcury

    #pragma once

    /*
       Memcury is a single-header file library for memory manipulation in C++.

       Containers:
           -PE::Address: A pointer container.
           -PE::Section: Portable executable section container for internal usage.

       Modules:
           -Scanner:
               -Constructors:
                   -Default: Takes a pointer to start the scanning from.
                   -FindPattern: Finds a pattern in memory.
                   -FindStringRef: Finds a string reference in memory, supports all types of strings.
               -Functions:
                   -SetTargetModule: Sets the target module for the scanner.
                   -ScanFor: Scans for a byte(s) near the current address.
                   -FindFunctionBoundary: Finds the boundary of a function near the current address.
                   -RelativeOffset: Gets the relative offset of the current address.
                   -AbsoluteOffset: Gets the absolute offset of the current address.
                   -GetAs: Gets the current address as a type.
                   -Get: Gets the current address as an int64.

           -TrampolineHook:
               -Constructors:
                   -Default: Takes a pointer pointer to the target function and a pointer to the hook function.
               -Functions:
                   -Commit: Commits the hook.
                   -Revert: Reverts the hook.
                   -Toggle: Toggles the hook on\off.

           -VEHHook:
               -Functions:
                   -Init: Initializes the VEH Hook system.
                   -AddHook: Adds a hook to the VEH Hook system.
                   -RemoveHook: Removes a hook from the VEH Hook system.
    */

    #include <string>
    #include <format>
    #include <vector>
    #include <stdexcept>
    #include <type_traits>
    #include <intrin.h>
    #include <Windows.h>
    #include <source_location>
    #include <DbgHelp.h>
    #pragma comment(lib, "Dbghelp.lib")
#include "../Project Reboot 3.0/log.h"
#include "../Project Reboot 3.0/inc.h"


    #define MemcuryAssert(cond)                                              \
        if (!(cond))                                                         \
        {                                                                    \
            MessageBoxA(nullptr, #cond, __FUNCTION__, MB_ICONERROR | MB_OK); \
            Memcury::Safety::FreezeCurrentThread();                          \
        }

    #define MemcuryAssertM(cond, msg)                                      \
        if (!(cond))                                                       \
        {                                                                  \
            MessageBoxA(nullptr, msg, __FUNCTION__, MB_ICONERROR | MB_OK); \
        }

    #define MemcuryThrow(msg)                                          \
        MessageBoxA(nullptr, msg, __FUNCTION__, MB_ICONERROR | MB_OK); \
        Memcury::Safety::FreezeCurrentThread();

    namespace Memcury
    {
        extern "C" IMAGE_DOS_HEADER __ImageBase;

        inline auto GetCurrentModule() -> HMODULE
        {
            return reinterpret_cast<HMODULE>(&__ImageBase);
        }

        namespace Util
        {
            template <typename T>
            constexpr static auto IsInRange(T value, T min, T max) -> bool
            {
                return value >= min && value < max;
            }

            constexpr auto StrHash(const char* str, int h = 0) -> unsigned int
            {
                return !str[h] ? 5381 : (StrHash(str, h + 1) * 33) ^ str[h];
            }

            inline auto IsSamePage(void* A, void* B) -> bool
            {
                MEMORY_BASIC_INFORMATION InfoA;
                if (!VirtualQuery(A, &InfoA, sizeof(InfoA)))
                {
                    return true;
                }

                MEMORY_BASIC_INFORMATION InfoB;
                if (!VirtualQuery(B, &InfoB, sizeof(InfoB)))
                {
                    return true;
                }

                return InfoA.BaseAddress == InfoB.BaseAddress;
            }

            inline auto GetModuleStartAndEnd() -> std::pair<uintptr_t, uintptr_t>
            {
                auto HModule = GetCurrentModule();
                auto NTHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>((uintptr_t)HModule + reinterpret_cast<PIMAGE_DOS_HEADER>((uintptr_t)HModule)->e_lfanew);

                uintptr_t dllStart = (uintptr_t)HModule;
                uintptr_t dllEnd = (uintptr_t)HModule + NTHeaders->OptionalHeader.SizeOfImage;

                return { dllStart, dllEnd };
            }

            inline auto CopyToClipboard(std::string str)
            {
                auto mem = GlobalAlloc(GMEM_FIXED, str.size() + 1);
                memcpy(mem, str.c_str(), str.size() + 1);

                OpenClipboard(nullptr);
                EmptyClipboard();
                SetClipboardData(CF_TEXT, mem);
                CloseClipboard();

                GlobalFree(mem);
            }
        }

        namespace Safety
        {
            enum class ExceptionMode
            {
                None,
                CatchDllExceptionsOnly,
                CatchAllExceptions
            };

            static auto FreezeCurrentThread() -> void
            {
                SuspendThread(GetCurrentThread());
            }

            static auto PrintStack(CONTEXT* ctx) -> void
            {
                STACKFRAME64 stack;
                memset(&stack, 0, sizeof(STACKFRAME64));

                auto process = GetCurrentProcess();
                auto thread = GetCurrentThread();

                SymInitialize(process, NULL, TRUE);

                bool result;
                DWORD64 displacement = 0;

                char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)]{ 0 };
                char name[256]{ 0 };
                char module[256]{ 0 };

                PSYMBOL_INFO symbolInfo = (PSYMBOL_INFO)buffer;

                for (ULONG frame = 0;; frame++)
                {
                    result = StackWalk64(
                        IMAGE_FILE_MACHINE_AMD64,
                        process,
                        thread,
                        &stack,
                        ctx,
                        NULL,
                        SymFunctionTableAccess64,
                        SymGetModuleBase64,
                        NULL);

                    if (!result)
                        break;

                    symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
                    symbolInfo->MaxNameLen = MAX_SYM_NAME;
                    SymFromAddr(process, (ULONG64)stack.AddrPC.Offset, &displacement, symbolInfo);

                    HMODULE hModule = NULL;
                    lstrcpyA(module, "");
                    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (const wchar_t*)(stack.AddrPC.Offset), &hModule);

                    if (hModule != NULL)
                        GetModuleFileNameA(hModule, module, 256);

                    printf("[%lu] Name: %s - Address: %p  - Module: %s\n", frame, symbolInfo->Name, (void*)symbolInfo->Address, module);
                }
            }

            template <ExceptionMode mode>
            auto MemcuryGlobalHandler(EXCEPTION_POINTERS* ExceptionInfo) -> long
            {
                auto [dllStart, dllEnd] = Util::GetModuleStartAndEnd();

                if constexpr (mode == ExceptionMode::CatchDllExceptionsOnly)
                {
                    if (!Util::IsInRange(ExceptionInfo->ContextRecord->Rip, dllStart, dllEnd))
                    {
                        return EXCEPTION_CONTINUE_SEARCH;
                    }
                }

                auto message = std::format("Memcury caught an exception at [{:x}]\nPress Yes if you want the address to be copied to your clipboard", ExceptionInfo->ContextRecord->Rip);
                if (MessageBoxA(nullptr, message.c_str(), "Error", MB_ICONERROR | MB_YESNO) == IDYES)
                {
                    std::string clip = std::format("{:x}", ExceptionInfo->ContextRecord->Rip);
                    Util::CopyToClipboard(clip);
                }

                PrintStack(ExceptionInfo->ContextRecord);

                FreezeCurrentThread();

                return EXCEPTION_EXECUTE_HANDLER;
            }

            template <ExceptionMode mode>
            static auto SetExceptionMode() -> void
            {
                SetUnhandledExceptionFilter(MemcuryGlobalHandler<mode>);
            }
        }

        namespace Globals
        {
            constexpr const bool bLogging = true;

            static const char* moduleName = nullptr;
        }

        namespace ASM
        {
            //@todo: this whole namespace needs a rework, should somehow make this more modern and less ugly.
            enum MNEMONIC : uint8_t
            {
                JMP_REL8 = 0xEB,
                JMP_REL32 = 0xE9,
                JMP_EAX = 0xE0,
                CALL = 0xE8,
                LEA = 0x8D,
                CDQ = 0x99,
                CMOVL = 0x4C,
                CMOVS = 0x48,
                CMOVNS = 0x49,
                NOP = 0x90,
                INT3 = 0xCC,
                RETN_REL8 = 0xC2,
                RETN = 0xC3,
                NONE = 0x00
            };

            constexpr int SIZE_OF_JMP_RELATIVE_INSTRUCTION = 5;
            constexpr int SIZE_OF_JMP_ABSLOUTE_INSTRUCTION = 13;

            constexpr auto MnemonicToString(MNEMONIC e) -> const char*
            {
                switch (e)
                {
                case JMP_REL8:
                    return "JMP_REL8";
                case JMP_REL32:
                    return "JMP_REL32";
                case JMP_EAX:
                    return "JMP_EAX";
                case CALL:
                    return "CALL";
                case LEA:
                    return "LEA";
                case CDQ:
                    return "CDQ";
                case CMOVL:
                    return "CMOVL";
                case CMOVS:
                    return "CMOVS";
                case CMOVNS:
                    return "CMOVNS";
                case NOP:
                    return "NOP";
                case INT3:
                    return "INT3";
                case RETN_REL8:
                    return "RETN_REL8";
                case RETN:
                    return "RETN";
                case NONE:
                    return "NONE";
                default:
                    return "UNKNOWN";
                }
            }

            constexpr auto Mnemonic(const char* s) -> MNEMONIC
            {
                switch (Util::StrHash(s))
                {
                case Util::StrHash("JMP_REL8"):
                    return JMP_REL8;
                case Util::StrHash("JMP_REL32"):
                    return JMP_REL32;
                case Util::StrHash("JMP_EAX"):
                    return JMP_EAX;
                case Util::StrHash("CALL"):
                    return CALL;
                case Util::StrHash("LEA"):
                    return LEA;
                case Util::StrHash("CDQ"):
                    return CDQ;
                case Util::StrHash("CMOVL"):
                    return CMOVL;
                case Util::StrHash("CMOVS"):
                    return CMOVS;
                case Util::StrHash("CMOVNS"):
                    return CMOVNS;
                case Util::StrHash("NOP"):
                    return NOP;
                case Util::StrHash("INT3"):
                    return INT3;
                case Util::StrHash("RETN_REL8"):
                    return RETN_REL8;
                case Util::StrHash("RETN"):
                    return RETN;
                default:
                    return NONE;
                }
            }

            inline auto byteIsA(uint8_t byte, MNEMONIC opcode) -> bool
            {
                return byte == opcode;
            }

            inline auto byteIsAscii(uint8_t byte) -> bool
            {
                static constexpr bool isAscii[0x100] = {
                    false, false, false, false, false, false, false, false, false, true, true, false, false, true, false, false,
                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                    true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,
                    true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,
                    true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,
                    true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,
                    true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,
                    true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false,
                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
                };

                return isAscii[byte];
            }

            inline bool isJump(uint8_t byte)
            {
                return byte >= 0x70 && byte <= 0x7F;
            }

            static auto pattern2bytes(const char* pattern) -> std::vector<int>
            {
                auto bytes = std::vector<int>{};
                const auto start = const_cast<char*>(pattern);
                const auto end = const_cast<char*>(pattern) + strlen(pattern);

                for (auto current = start; current < end; ++current)
                {
                    if (*current == '?')
                    {
                        ++current;
                        if (*current == '?')
                            ++current;
                        bytes.push_back(-1);
                    }
                    else
                    {
                        bytes.push_back(strtoul(current, &current, 16));
                    }
                }
                return bytes;
            }
        }

        namespace PE
        {
            inline auto SetCurrentModule(const char* moduleName) -> void
            {
                Globals::moduleName = moduleName;
            }

            inline auto GetModuleBase() -> uintptr_t
            {
                return reinterpret_cast<uintptr_t>(GetModuleHandleA(Globals::moduleName));
            }

            inline auto GetDOSHeader() -> PIMAGE_DOS_HEADER
            {
                return reinterpret_cast<PIMAGE_DOS_HEADER>(GetModuleBase());
            }

            inline auto GetNTHeaders() -> PIMAGE_NT_HEADERS
            {
                return reinterpret_cast<PIMAGE_NT_HEADERS>(GetModuleBase() + GetDOSHeader()->e_lfanew);
            }

            class Address
            {
                uintptr_t _address;

            public:
                Address()
                {
                    _address = 0;
                }

                Address(uintptr_t address)
                    : _address(address)
                {
                }

                Address(void* address)
                    : _address(reinterpret_cast<uintptr_t>(address))
                {
                }

                auto operator=(uintptr_t address) -> Address
                {
                    _address = address;
                    return *this;
                }

                auto operator=(void* address) -> Address
                {
                    _address = reinterpret_cast<uintptr_t>(address);
                    return *this;
                }

                auto operator+(uintptr_t offset) -> Address
                {
                    return Address(_address + offset);
                }

                bool operator>(uintptr_t offset)
                {
                    return _address > offset;
                }

                bool operator>(Address address)
                {
                    return _address > address._address;
                }

                bool operator<(uintptr_t offset)
                {
                    return _address < offset;
                }

                bool operator<(Address address)
                {
                    return _address < address._address;
                }

                bool operator>=(uintptr_t offset)
                {
                    return _address >= offset;
                }

                bool operator>=(Address address)
                {
                    return _address >= address._address;
                }

                bool operator<=(uintptr_t offset)
                {
                    return _address <= offset;
                }

                bool operator<=(Address address)
                {
                    return _address <= address._address;
                }

                bool operator==(uintptr_t offset)
                {
                    return _address == offset;
                }

                bool operator==(Address address)
                {
                    return _address == address._address;
                }

                bool operator!=(uintptr_t offset)
                {
                    return _address != offset;
                }

                bool operator!=(Address address)
                {
                    return _address != address._address;
                }

                auto RelativeOffset(uint32_t offset) -> Address
                {
                    _address = ((_address + offset + 4) + *(int32_t*)(_address + offset));
                    return *this;
                }

                auto AbsoluteOffset(uint32_t offset) -> Address
                {
                    _address = _address + offset;
                    return *this;
                }

                auto Jump() -> Address
                {
                    if (ASM::isJump(*reinterpret_cast<UINT8*>(_address)))
                    {
                        UINT8 toSkip = *reinterpret_cast<UINT8*>(_address + 1);
                        _address = _address + 2 + toSkip;
                    }

                    return *this;
                }

                auto Get() -> uintptr_t
                {
                    return _address;
                }

                template <typename T>
                auto GetAs() -> T
                {
                    return reinterpret_cast<T>(_address);
                }

                auto IsValid() -> bool
                {
                    return _address != 0;
                }
            };

            class Section
            {
            public:
                std::string sectionName;
                IMAGE_SECTION_HEADER rawSection;

                static auto GetAllSections() -> std::vector<Section>
                {
                    std::vector<Section> sections;

                    auto sectionsSize = GetNTHeaders()->FileHeader.NumberOfSections;
                    auto section = IMAGE_FIRST_SECTION(GetNTHeaders());

                    for (WORD i = 0; i < sectionsSize; i++, section++)
                    {
                        auto secName = std::string((char*)section->Name);

                        sections.push_back({ secName, *section });
                    }

                    return sections;
                }

                static auto GetSection(std::string sectionName) -> Section
                {
                    for (auto& section : GetAllSections())
                    {
                        if (section.sectionName == sectionName)
                        {
                            return section;
                        }
                    }

                    MemcuryThrow("Section not found");
                    return Section{};
                }

                auto GetSectionSize() -> uint32_t
                {
                    return rawSection.Misc.VirtualSize;
                }

                auto GetSectionStart() -> Address
                {
                    return Address(GetModuleBase() + rawSection.VirtualAddress);
                }

                auto GetSectionEnd() -> Address
                {
                    return Address(GetSectionStart() + GetSectionSize());
                }

                auto isInSection(Address address) -> bool
                {
                    return address >= GetSectionStart() && address < GetSectionEnd();
                }
            };
        }

        class Scanner
        {
            PE::Address _address;

        public:
            Scanner(PE::Address address)
                : _address(address)
            {
            }

            static auto SetTargetModule(const char* moduleName) -> void
            {
                PE::SetCurrentModule(moduleName);
            }

            static auto FindPatternEx(HANDLE handle, const char* pattern, const char* mask, uint64_t begin, uint64_t end) -> Scanner
            {
                auto scan = [](const char* pattern, const char* mask, char* begin, unsigned int size) -> char*
                {
                    size_t patternLen = strlen(mask);
                    for (unsigned int i = 0; i < size - patternLen; i++)
                    {
                        bool found = true;
                        for (unsigned int j = 0; j < patternLen; j++)
                        {
                            if (mask[j] != '?' && pattern[j] != *(begin + i + j))
                            {
                                found = false;
                                break;
                            }
                        }

                        if (found)
                            return (begin + i);
                    }
                    return nullptr;
                };

                uint64_t match = NULL;
                SIZE_T bytesRead;
                char* buffer = nullptr;
                MEMORY_BASIC_INFORMATION mbi = { 0 };

                uint64_t curr = begin;

                for (uint64_t curr = begin; curr < end; curr += mbi.RegionSize)
                {
                    if (!VirtualQueryEx(handle, (void*)curr, &mbi, sizeof(mbi)))
                        continue;

                    if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS)
                        continue;

                    buffer = new char[mbi.RegionSize];

                    if (ReadProcessMemory(handle, mbi.BaseAddress, buffer, mbi.RegionSize, &bytesRead))
                    {
                        char* internalAddr = scan(pattern, mask, buffer, (unsigned int)bytesRead);

                        if (internalAddr != nullptr)
                        {
                            match = curr + (uint64_t)(internalAddr - buffer);
                            break;
                        }
                    }
                }
                delete[] buffer;

                MemcuryAssertM(match != 0, "FindPatternEx return nullptr");

                return Scanner(match);
            }

            static auto FindPatternEx(HANDLE handle, const char* sig) -> Scanner
            {
                char pattern[100];
                char mask[100];

                char lastChar = ' ';
                unsigned int j = 0;

                for (unsigned int i = 0; i < strlen(sig); i++)
                {
                    if ((sig[i] == '?' || sig[i] == '*') && (lastChar != '?' && lastChar != '*'))
                    {
                        pattern[j] = mask[j] = '?';
                        j++;
                    }

                    else if (isspace(lastChar))
                    {
                        pattern[j] = lastChar = (char)strtol(&sig[i], 0, 16);
                        mask[j] = 'x';
                        j++;
                    }
                    lastChar = sig[i];
                }
                pattern[j] = mask[j] = '\0';

                auto module = (uint64_t)GetModuleHandle(nullptr);

                return FindPatternEx(handle, pattern, mask, module, module + Memcury::PE::GetNTHeaders()->OptionalHeader.SizeOfImage);
            }

            static auto FindPattern(const char* signature, bool bWarnIfNotFound = true) -> Scanner
            {
                PE::Address add{ nullptr };

                const auto sizeOfImage = PE::GetNTHeaders()->OptionalHeader.SizeOfImage;
                auto patternBytes = ASM::pattern2bytes(signature);
                const auto scanBytes = reinterpret_cast<std::uint8_t*>(PE::GetModuleBase());

                const auto s = patternBytes.size();
                const auto d = patternBytes.data();

                for (auto i = 0ul; i < sizeOfImage - s; ++i)
                {
                    bool found = true;
                    for (auto j = 0ul; j < s; ++j)
                    {
                        if (scanBytes[i + j] != d[j] && d[j] != -1)
                        {
                            found = false;
                            break;
                        }
                    }

                    if (found)
                    {
                        add = reinterpret_cast<uintptr_t>(&scanBytes[i]);
                        break;
                    }
                }

                // MemcuryAssertM(add != 0, "FindPattern return nullptr");

                if (bWarnIfNotFound)
                {
                    if (add == 0)
                    {
                        LOG_WARN(LogMemory, "Failed to find {}", signature);
                        // MessageBoxA(0, ("FindPattern " + std::string(signature) + " null").c_str(), "Memcury", MB_ICONERROR);
                    }
                }

                return Scanner(add);
            }

            static auto FindPointerRef(void* Pointer, int useRefNum = 0, bool bUseFirstResult = false, bool bWarnIfNotFound = true) -> Scanner // credit me and ender
            {
                PE::Address add{ nullptr };

                if (!Pointer)
                    return Scanner(add);

                auto textSection = PE::Section::GetSection(".text");

                const auto scanBytes = reinterpret_cast<std::uint8_t*>(textSection.GetSectionStart().Get());

                int aa = 0;

                // scan only text section
                for (DWORD i = 0x0; i < textSection.GetSectionSize(); i++)
                {
                    if ((scanBytes[i] == ASM::CMOVL || scanBytes[i] == ASM::CMOVS) && (scanBytes[i + 1] == ASM::LEA || scanBytes[i + 1] == 0x8B))
                    {
                        if (PE::Address(&scanBytes[i]).RelativeOffset(3).GetAs<void*>() == Pointer)
                        {
                            add = PE::Address(&scanBytes[i]);

                            // LOG_INFO(LogDev, "2add: 0x{:x}", add.Get() - __int64(GetModuleHandleW(0)));

                            if (bUseFirstResult)
                                return Scanner(add);

                            /* if (++aa > useRefNum)
                                break; */
                        }
                    }

                    if (scanBytes[i] == ASM::CALL)
                    {
                        if (PE::Address(&scanBytes[i]).RelativeOffset(1).GetAs<void*>() == Pointer)
                        {
                            add = PE::Address(&scanBytes[i]);

                            // LOG_INFO(LogDev, "1add: 0x{:x}", add.Get() - __int64(GetModuleHandleW(0)));

                            if (bUseFirstResult)
                                return Scanner(add);

                            /* if (++aa > useRefNum)
                                break; */
                        }
                    }
                }
                
                if (bWarnIfNotFound)
                {
                    if (add == 0)
                    {
                        LOG_WARN(LogMemory, "FindPointerRef return nullptr");
                        // MessageBoxA(0, "FindPointerRef return nullptr", "Memcury", MB_OK);
                    }
                    else
                    {
                        // MessageBoxA(0, std::format("FindPointerRef return 0x{:x}", add.Get() - __int64(GetModuleHandleW(0))).c_str(), "Memcury", MB_OK);
                    }
                }
                
                return Scanner(add);
            }

            // Supports wide and normal strings both std and pointers
            template <typename T = const wchar_t*>
            static auto FindStringRef(T string, bool bWarnIfNotFound = true, int useRefNum = 0, bool bIsInFunc = false, bool bSkunky = false) -> Scanner
            {
                PE::Address add{ nullptr };

                constexpr auto bIsWide = std::is_same<T, const wchar_t*>::value;
                constexpr auto bIsChar = std::is_same<T, const char*>::value;

                constexpr auto bIsPtr = bIsWide || bIsChar;

                auto textSection = PE::Section::GetSection(".text");
                auto rdataSection = PE::Section::GetSection(".rdata");

                const auto scanBytes = reinterpret_cast<std::uint8_t*>(textSection.GetSectionStart().Get());

                int aa = 0;

                // scan only text section
                for (DWORD i = 0x0; i < textSection.GetSectionSize(); i++)
                {
                    if ((scanBytes[i] == ASM::CMOVL || scanBytes[i] == ASM::CMOVS) && scanBytes[i + 1] == ASM::LEA)
                    {
                        auto stringAdd = PE::Address(&scanBytes[i]).RelativeOffset(3);

                        // Check if the string is in the .rdata section
                        if (rdataSection.isInSection(stringAdd))
                        {
                            auto strBytes = stringAdd.GetAs<std::uint8_t*>();

                            // Check if the first char is printable
                            if (ASM::byteIsAscii(strBytes[0]))
                            {
                                if constexpr (!bIsPtr)
                                {
                                    // typedef T::value_type char_type;
                                    using char_type = std::decay_t<std::remove_pointer_t<T>>;

                                    auto lea = stringAdd.GetAs<char_type*>();

                                    T leaT(lea);

                                    if (leaT == string)
                                    {
                                        add = PE::Address(&scanBytes[i]);

                                        if (++aa > useRefNum)
                                            break;
                                    }
                                }
                                else
                                {
                                    auto lea = stringAdd.GetAs<T>();

                                    if constexpr (bIsWide)
                                    {
                                        if (wcscmp(string, lea) == 0)
                                        {
                                            add = PE::Address(&scanBytes[i]);

                                            if (++aa > useRefNum)
                                                break;
                                        }
                                    }
                                    else
                                    {
                                        if (strcmp(string, lea) == 0)
                                        {
                                            add = PE::Address(&scanBytes[i]);

                                            if (++aa > useRefNum)
                                                break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // MemcuryAssertM(add != 0, "FindStringRef return nullptr");

                if (bWarnIfNotFound)
                {
                    if (add == 0)
                    {
                        if constexpr (bIsWide)
                        {
                            std::wstring wstr = std::wstring(string);
                            LOG_WARN(LogMemory, "Failed to find String {}", std::string(wstr.begin(), wstr.end()));

                            // auto aaa = (L"failed FindStringRef " + std::wstring(string));
                            // MessageBoxA(0, std::string(aaa.begin(), aaa.end()).c_str(), "Memcury", MB_ICONERROR);
                        }
                        else
                        {
                            LOG_WARN(LogMemory, "Failed to find String {}", string);
                            // MessageBoxA(0, ("failed FindStringRef " + std::string(string)).c_str(), "Memcury", MB_ICONERROR);
                        }
                    }
                }

                if (add.Get())
                {
                    if (bIsInFunc)
                    {
                        for (int i = 0; i < 300; i++)
                        {
                            if (!bSkunky ? (*(uint8_t*)(add.Get() - i) == 0x48 && *(uint8_t*)(add.Get() - i + 1) == 0x83) : 
                                (*(uint8_t*)(add.Get() - i) == 0x4C && *(uint8_t*)(add.Get() - i + 1) == 0x8B && *(uint8_t*)(add.Get() - i + 2) == 0xDC))
                            {
                                // MessageBoxA(0, std::format("0x{:x}", (__int64(add.Get() - i) - __int64(GetModuleHandleW(0)))).c_str(), "Memcury", MB_OK);

                                auto beginFunc = Scanner(add.Get() - i);

                                auto ref = FindPointerRef(beginFunc.GetAs<void*>());

                                return ref;
                            }
                        }
                    }
                }

                return Scanner(add);
            }

            auto Jump() -> Scanner
            {
                _address.Jump();
                return *this;
            }

            inline auto ScanFor(std::vector<uint8_t> opcodesToFind, bool forward = true, int toSkip = 0) -> Scanner
            {
                const auto scanBytes = _address.GetAs<std::uint8_t*>();

                for (auto i = (forward ? 1 : -1); forward ? (i < 2048) : (i > -2048); forward ? i++ : i--)
                {
                    bool found = true;

                    for (int k = 0; k < opcodesToFind.size() && found; k++)
                    {
                        auto& currentOpcode = opcodesToFind[k];

                        if (currentOpcode == -1)
                            continue;

                        // std::cout << std::format("[{} {}] 0x{:x}\n", i, k, currentOpcode);

                        found = currentOpcode == scanBytes[i + k];
                    }

                    if (found)
                    {
                        _address = &scanBytes[i];
                        if (toSkip != 0)
                        {
                            return ScanFor(opcodesToFind, forward, toSkip - 1);
                        }

                        break;
                    }
                }

                return *this;
            }

            auto FindFunctionBoundary(bool forward = false) -> Scanner
            {
                const auto scanBytes = _address.GetAs<std::uint8_t*>();

                for (auto i = (forward ? 1 : -1); forward ? (i < 2048) : (i > -2048); forward ? i++ : i--)
                {
                    if ( // ASM::byteIsA(scanBytes[i], ASM::MNEMONIC::JMP_REL8) ||
                         // ASM::byteIsA(scanBytes[i], ASM::MNEMONIC::JMP_REL32) ||
                         // ASM::byteIsA(scanBytes[i], ASM::MNEMONIC::JMP_EAX) ||
                        ASM::byteIsA(scanBytes[i], ASM::MNEMONIC::RETN_REL8) || ASM::byteIsA(scanBytes[i], ASM::MNEMONIC::RETN) || ASM::byteIsA(scanBytes[i], ASM::MNEMONIC::INT3))
                    {
                        _address = (uintptr_t)&scanBytes[i + 1];
                        break;
                    }
                }

                return *this;
            }

            auto RelativeOffset(uint32_t offset) -> Scanner
            {
                if (!_address.Get())
                {
                    LOG_WARN(LogMemory, "Trying to relative offset an invalid offset!");
                    return *this;
                }

                _address.RelativeOffset(offset);

                return *this;
            }

            auto AbsoluteOffset(uint32_t offset) -> Scanner
            {
                _address.AbsoluteOffset(offset);

                return *this;
            }

            template <typename T>
            auto GetAs() -> T
            {
                return _address.GetAs<T>();
            }

            auto Get() -> uintptr_t
            {
                return _address.Get();
            }

            auto IsValid() -> bool
            {
                return _address.IsValid();
            }
        };

        /* Bad don't use it tbh... */
        class TrampolineHook
        {
            void** originalFunctionPtr;
            PE::Address originalFunction;
            PE::Address hookFunction;
            PE::Address allocatedPage;
            std::vector<uint8_t> restore;

            void PointToCodeIfNot(PE::Address& ptr)
            {
                auto bytes = ptr.GetAs<std::uint8_t*>();

                if (ASM::byteIsA(bytes[0], ASM::MNEMONIC::JMP_REL32))
                {
                    ptr = bytes + 5 + *(int32_t*)&bytes[1];
                }
            }

            void* AllocatePageNearAddress(void* targetAddr)
            {
                SYSTEM_INFO sysInfo;
                GetSystemInfo(&sysInfo);
                const uint64_t PAGE_SIZE = sysInfo.dwPageSize;

                uint64_t startAddr = (uint64_t(targetAddr) & ~(PAGE_SIZE - 1)); // round down to nearest page boundary
                uint64_t minAddr = fmin(startAddr - 0x7FFFFF00, (uint64_t)sysInfo.lpMinimumApplicationAddress);
                uint64_t maxAddr = fmax(startAddr + 0x7FFFFF00, (uint64_t)sysInfo.lpMaximumApplicationAddress);

                uint64_t startPage = (startAddr - (startAddr % PAGE_SIZE));

                for (uint64_t pageOffset = 1; pageOffset; pageOffset++)
                {
                    uint64_t byteOffset = pageOffset * PAGE_SIZE;
                    uint64_t highAddr = startPage + byteOffset;
                    uint64_t lowAddr = (startPage > byteOffset) ? startPage - byteOffset : 0;

                    bool needsExit = highAddr > maxAddr && lowAddr < minAddr;

                    if (highAddr < maxAddr)
                    {
                        void* outAddr = VirtualAlloc((void*)highAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                        if (outAddr)
                            return outAddr;
                    }

                    if (lowAddr > minAddr)
                    {
                        void* outAddr = VirtualAlloc((void*)lowAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                        if (outAddr != nullptr)
                            return outAddr;
                    }

                    if (needsExit)
                    {
                        break;
                    }
                }

                return nullptr;
            }

            void WriteAbsoluteJump(void* jumpLocation, void* destination)
            {
                uint8_t absJumpInstructions[] = {
                    ASM::Mnemonic("CMOVNS"), 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov r10, addr
                    0x41, 0xFF, 0xE2 // jmp r10
                };

                auto destination64 = (uint64_t)destination;
                memcpy(&absJumpInstructions[2], &destination64, sizeof(destination64));
                memcpy(jumpLocation, absJumpInstructions, sizeof(absJumpInstructions));
            }

            uintptr_t PrepareRestore()
            {
                /*
                    This is not a correct way to do it at all, since not all functions sub from the stack
                    This needs so much more tests, but it works for now.
                */

                Scanner scanner(originalFunction);
                scanner.ScanFor({ 0x48, 0x83, 0xEC }); // sub rsp

                auto restoreSize = scanner.Get() - originalFunction.Get();

                MemcuryAssert(restoreSize > 0 && restoreSize < 0x100);

                restore.reserve(restoreSize);
                for (auto i = 0; i < restoreSize; i++)
                {
                    restore.push_back(originalFunction.GetAs<uint8_t*>()[i]);
                }

                return restoreSize;
            }

            void WriteRestore()
            {
                auto restorePtr = allocatedPage + ASM::SIZE_OF_JMP_ABSLOUTE_INSTRUCTION + 2;

                memcpy(restorePtr.GetAs<void*>(), restore.data(), restore.size());

                *originalFunctionPtr = restorePtr.GetAs<void*>();

                // Write a jump back to where the execution should resume
                restorePtr.AbsoluteOffset((uint32_t)restore.size());

                auto contuineExecution = originalFunction + restore.size();

                WriteAbsoluteJump(restorePtr.GetAs<void*>(), contuineExecution.GetAs<void*>());
            }

            auto PrepareJMPInstruction(uint64_t dst)
            {
                uint8_t bytes[5] = { ASM::Mnemonic("JMP_REL32"), 0x0, 0x0, 0x0, 0x0 };

                const uint64_t relAddr = dst - (originalFunction.Get() + ASM::SIZE_OF_JMP_RELATIVE_INSTRUCTION);
                memcpy(bytes + 1, &relAddr, 4);

                return std::move(bytes);
            }

            bool IsHooked()
            {
                return originalFunction.GetAs<uint8_t*>()[0] == ASM::Mnemonic("JMP_REL32");
            }

        public:
            TrampolineHook(void** originalFunction, void* hookFunction)
            {
                this->originalFunctionPtr = originalFunction;

                this->originalFunction = *originalFunction;
                this->hookFunction = hookFunction;

                PointToCodeIfNot(this->originalFunction);
                PointToCodeIfNot(this->hookFunction);
            };

            bool Commit()
            {
                auto fnStart = originalFunction.GetAs<void*>();

                auto restoreSize = PrepareRestore();

                if (!allocatedPage.IsValid())
                {
                    allocatedPage = AllocatePageNearAddress(fnStart);
                }

                memset(allocatedPage.GetAs<void*>(), ASM::MNEMONIC::INT3, 0x1000);

                WriteAbsoluteJump(allocatedPage.GetAs<void*>(), hookFunction.GetAs<void*>());

                DWORD oldProtect;
                VirtualProtect(fnStart, 1024, PAGE_EXECUTE_READWRITE, &oldProtect);

                auto jmpInstruction = PrepareJMPInstruction(allocatedPage.Get());

                WriteRestore();

                memset(fnStart, ASM::MNEMONIC::INT3, restoreSize);
                memcpy(fnStart, jmpInstruction, ASM::SIZE_OF_JMP_RELATIVE_INSTRUCTION);

                return true;
            }

            bool Revert()
            {
                auto fnStart = originalFunction.GetAs<void*>();

                DWORD oldProtect;
                VirtualProtect(fnStart, 1024, PAGE_EXECUTE_READWRITE, &oldProtect);

                memcpy(fnStart, restore.data(), restore.size());

                *originalFunctionPtr = originalFunction.GetAs<void*>();

                // VirtualFree(allocatedPage.GetAs<void*>(), 0x1000, MEM_RELEASE);

                return true;
            }

            auto Toggle()
            {
                if (IsHooked())
                    Revert();
                else
                    Commit();

                return IsHooked();
            }
        };

        namespace VEHHook
        {
            struct HOOK_INFO
            {
                void* Original;
                void* Detour;

                HOOK_INFO(void* Original, void* Detour)
                    : Original(Original)
                    , Detour(Detour)
                {
                }
            };

            inline std::vector<HOOK_INFO> Hooks;
            inline std::vector<DWORD> HookProtections;
            inline HANDLE ExceptionHandler;

            inline long Handler(EXCEPTION_POINTERS* Exception)
            {
                if (Exception->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION)
                {
                    auto Itr = std::find_if(Hooks.begin(), Hooks.end(), [Rip = Exception->ContextRecord->Rip](const HOOK_INFO& Hook)
                    { return Hook.Original == (void*)Rip; });
                    if (Itr != Hooks.end())
                    {
                        Exception->ContextRecord->Rip = (uintptr_t)Itr->Detour;
                    }

                    Exception->ContextRecord->EFlags |= 0x100; // SINGLE_STEP_FLAG

                    return EXCEPTION_CONTINUE_EXECUTION;
                }
                else if (Exception->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
                {
                    // TODO: find a way to only vp the function that about to get executed
                    for (auto& Hook : Hooks)
                    {
                        DWORD dwOldProtect;
                        VirtualProtect(Hook.Original, 1, PAGE_EXECUTE_READ | PAGE_GUARD, &dwOldProtect);
                    }

                    return EXCEPTION_CONTINUE_EXECUTION;
                }

                return EXCEPTION_CONTINUE_SEARCH;
            }

            inline bool Init()
            {
                if (ExceptionHandler == nullptr)
                {
                    ExceptionHandler = AddVectoredExceptionHandler(true, (PVECTORED_EXCEPTION_HANDLER)Handler);
                }
                return ExceptionHandler != nullptr;
            }

            inline bool AddHook(void* Target, void* Detour)
            {
                if (ExceptionHandler == nullptr)
                {
                    return false;
                }

                if (Util::IsSamePage(Target, Detour))
                {
                    return false;
                }

                if (!VirtualProtect(Target, 1, PAGE_EXECUTE_READ | PAGE_GUARD, &HookProtections.emplace_back()))
                {
                    HookProtections.pop_back();
                    return false;
                }

                Hooks.emplace_back(Target, Detour);
                return true;
            }

            inline bool RemoveHook(void* Original)
            {
                auto Itr = std::find_if(Hooks.begin(), Hooks.end(), [Original](const HOOK_INFO& Hook)
                    { return Hook.Original == Original; });

                if (Itr == Hooks.end())
                {
                    return false;
                }

                const auto ProtItr = HookProtections.begin() + std::distance(Hooks.begin(), Itr);
                Hooks.erase(Itr);

                DWORD dwOldProtect;
                bool Ret = VirtualProtect(Original, 1, *ProtItr, &dwOldProtect);
                HookProtections.erase(ProtItr);

                return false;
            }
        }
    }

    inline int HexToDec(std::string hexValue)
    {
        int len = hexValue.size();
        // Initializing base value to 1, i.e 16 ^ 0
        int base = 1;
        int dec_value = 0;
        // extracting characters as digits from last
        // character
        for (int i = len - 1; i >= 0; i--) {
            if (hexValue[i] >= '0' && hexValue[i] <= '9') {
                dec_value += (int(hexValue[i]) - 48) * base;


                // incrementing base by power
                base = base * 16;
            }
            // if character lies in 'A' - 'F' , converting
            // it to integral 10 - 15 by subtracting 55
            // from ASCII value
            else if (hexValue[i] >= 'A' && hexValue[i] <= 'F') {
                dec_value += (int(hexValue[i]) - 55) * base;


                // incrementing base by power
                base = base * 16;
            }
        }
        return dec_value;
    }

    inline std::string GetBytes(uintptr_t Address, int count = 10) {
        std::string Bytes;

        for (int i = 0; i < count; i++) {
            auto byte = *(uint8_t*)(Address + i) & 0xff;
            auto Byte = (byte == 0) ? "? " : std::format("{:x} ", byte);

            if (Byte.length() == 2 && byte != 0) // 2 because of the space
                Byte = "0" + Byte;

            Bytes += Byte;
        }

        std::transform(Bytes.begin(), Bytes.end(), Bytes.begin(), ::toupper);

        return Bytes;
    }

    inline bool IsBadReadPtr(void* p)
    {
        MEMORY_BASIC_INFORMATION mbi = { 0 };
        if (::VirtualQuery(p, &mbi, sizeof(mbi)))
        {
            DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
            bool b = !(mbi.Protect & mask);
            // check the page is not a guard page
            if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

            return b;
        }
        return true;
    }

    static void VirtualSwap(void** VTable, int Idx, void* NewFunc)
    {
        DWORD dwProtection;
        VirtualProtect(&VTable[Idx], 8, PAGE_EXECUTE_READWRITE, &dwProtection);

        VTable[Idx] = NewFunc;

        DWORD dwTemp;
        VirtualProtect(&VTable[Idx], 8, dwProtection, &dwTemp);
    }

    inline uintptr_t FindNameRef(const wchar_t* Name, int skip = 0, bool bWarnStringNotFound = true)
    {
        auto StringRef = Memcury::Scanner::FindStringRef(Name, true, skip);

        if (!StringRef.Get())
            return 0;

        auto FunctionPtr = StringRef.ScanFor({ 0x48, 0x8D, 0x0D }).RelativeOffset(3).GetAs<void*>();

        auto PtrRef = Memcury::Scanner::FindPointerRef(FunctionPtr);

        return PtrRef.Get();
    }

    // Finds a string ref, then goes searches xref of the function that it's in and returns that address.
    inline uintptr_t FindFunctionCall(const wchar_t* Name, const std::vector<uint8_t>& Bytes = std::vector<uint8_t>{ 0x48, 0x89, 0x5C }, int skip = 0, bool bWarnStringNotFound = true) // credit ender & me
    {
        auto NameRef = FindNameRef(Name, skip, bWarnStringNotFound);

        if (!NameRef)
            return 0;

        return Memcury::Scanner(NameRef).ScanFor(Bytes, false).Get();
    }

    inline bool IsNullSub(uint64 Addr) 
    {
        // if (*(uint8_t*)(Addr) == 0xEB && *(uint8_t*)(Addr + 1) == 0xF7) // positive sp value has been detected, the output may be wrong!
           // return true;

        return *(uint8_t*)(Addr) == 0xC3 || *(uint8_t*)(Addr) == 0xC2;
    }

