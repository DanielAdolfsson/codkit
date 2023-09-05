#include <cassert>
#include <cstdio>
#include <windows.h>

#include <string>

typedef struct BASE_RELOCATION_ENTRY {
    USHORT Offset : 12;
    USHORT Type : 4;
} *PBASE_RELOCATION_ENTRY;

bool Injected;
char **Args;

extern "C" int mainCRTStartup();

static DWORD EntryPoint(void *image) {
    auto ntHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)image +
                                        ((PIMAGE_DOS_HEADER)image)->e_lfanew);

    for (auto impDesc =
             (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)image +
                                        ntHeader->OptionalHeader
                                            .DataDirectory
                                                [IMAGE_DIRECTORY_ENTRY_IMPORT]
                                            .VirtualAddress);
         impDesc->Name;
         impDesc++) {
        if (impDesc->Name & 0x80000000) {
            impDesc->Name &= ~0x80000000;
            continue;
        }

        auto name = (PCHAR)image + impDesc->Name;
        auto library = LoadLibraryA(name);

        // TODO: Fail if we couldn't load.

        if (library) {
            auto thunk =
                (PIMAGE_THUNK_DATA)((DWORD_PTR)image + impDesc->FirstThunk);

            while (thunk->u1.AddressOfData != 0) {
                if (IMAGE_SNAP_BY_ORDINAL(thunk->u1.Ordinal)) {
                    auto functionOrdinal =
                        (LPCSTR)IMAGE_ORDINAL(thunk->u1.Ordinal);
                    thunk->u1.Function =
                        (DWORD_PTR)GetProcAddress(library, functionOrdinal);
                } else {
                    auto functionName =
                        (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)image +
                                                thunk->u1.AddressOfData);
                    auto functionAddress =
                        (DWORD_PTR)GetProcAddress(library, functionName->Name);
                    thunk->u1.Function = functionAddress;
                }
                ++thunk;
            }
        }
    }

    Injected = true;
    mainCRTStartup();
    return 0;
}

/**
 * Inject into the specified process.
 *
 * @param dwProcessId
 * @return
 */
static bool Inject(HANDLE hProcess, char *args[]) {
    CCHAR path[MAX_PATH];

    if (GetModuleFileNameA(nullptr, path, sizeof(path)) == 0)
        return false;

    auto fp = CreateFileA(path,
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

    if (fp == nullptr)
        return false;

    auto size = GetFileSize(fp, nullptr);

    auto pData = VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);

    DWORD bytesRead;
    if (!ReadFile(fp, pData, size, &bytesRead, nullptr)) {
        CloseHandle(fp);
        return false;
    }

    CloseHandle(fp);

    if (bytesRead != size)
        return false;

    auto ntHeader =
        PIMAGE_NT_HEADERS(PCHAR(pData) + PIMAGE_DOS_HEADER(pData)->e_lfanew);

    // Calculate the total number of bytes we need for the arguments.
    unsigned long totalArgsSize = sizeof(char *);
    int totalArgs = 0;
    for (char **p = args; *p; p++) {
        totalArgsSize += sizeof(char *) + strlen(*p) + 1;
        totalArgs++;
    }

    auto imageSize = ntHeader->OptionalHeader.SizeOfImage + totalArgsSize;

    auto pTempImage =
        VirtualAlloc(nullptr, imageSize, MEM_COMMIT, PAGE_READWRITE);

    mempcpy(pTempImage, pData, ntHeader->OptionalHeader.SizeOfHeaders);

    auto firstSection = IMAGE_FIRST_SECTION(ntHeader);

    for (auto i = 0; i < ntHeader->FileHeader.NumberOfSections; i++) {
        auto pSection = &firstSection[i];
        memcpy(PCHAR(pTempImage) + pSection->VirtualAddress,
               PCHAR(pData) + pSection->PointerToRawData,
               pSection->SizeOfRawData);
        // printf("%-10s  %lx - %lx\n", pSection->Name,
        // pSection->VirtualAddress, pSection->VirtualAddress +
        // pSection->SizeOfRawData);
    }

    // Open target process.

    auto pTargetImage = VirtualAllocEx(hProcess,
                                       nullptr,
                                       imageSize,
                                       MEM_COMMIT,
                                       PAGE_EXECUTE_READWRITE);

    // Calculate the total number of bytes we need for the arguments.

    DWORD_PTR offset = ntHeader->OptionalHeader.SizeOfImage +
                       totalArgs * sizeof(char *) + sizeof(char *);
    auto argv =
        (char **)((DWORD_PTR)pTempImage + ntHeader->OptionalHeader.SizeOfImage);
    auto arg = 0;
    while (args[arg]) {
        auto len = strlen(args[arg]);
        memcpy((char *)pTempImage + offset, args[arg], len);
        *((char *)pTempImage + offset + len) = 0;
        argv[arg++] = (char *)(pTargetImage) + offset;
        offset += len + 1;
    }
    argv[arg] = nullptr;

    // Relocate

    auto relocationTable =
        (PIMAGE_BASE_RELOCATION)((DWORD_PTR)pTempImage +
                                 ntHeader->OptionalHeader
                                     .DataDirectory
                                         [IMAGE_DIRECTORY_ENTRY_BASERELOC]
                                     .VirtualAddress);
    DWORD relocationEntriesCount = 0;
    PDWORD_PTR patchedAddress;
    PBASE_RELOCATION_ENTRY relocationRVA = NULL;
    DWORD_PTR delta =
        (DWORD_PTR)pTargetImage - ntHeader->OptionalHeader.ImageBase;

    while (relocationTable->SizeOfBlock > 0) {
        relocationEntriesCount =
            (relocationTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) /
            sizeof(USHORT);
        relocationRVA = (PBASE_RELOCATION_ENTRY)(relocationTable + 1);

        for (short i = 0; i < relocationEntriesCount; i++) {
            if (relocationRVA[i].Type == IMAGE_REL_BASED_ABSOLUTE) {
                continue;
            }

            if (relocationRVA[i].Type != IMAGE_REL_BASED_HIGHLOW) {
                printf("Expected IMAGE_REL_BASED_HIGHLOW, not %d\n",
                       relocationRVA[i].Type);
                continue;
            }

            patchedAddress = (PDWORD_PTR)((DWORD_PTR)pTempImage +
                                          relocationTable->VirtualAddress +
                                          relocationRVA[i].Offset);
            *patchedAddress += delta;
        }
        relocationTable =
            (PIMAGE_BASE_RELOCATION)((DWORD_PTR)relocationTable +
                                     relocationTable->SizeOfBlock);
    }

    // Fixup imports for KERNEL32.DLL and USER32.DLL.

    auto importDesc =
        (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)pTempImage +
                                   ntHeader->OptionalHeader
                                       .DataDirectory
                                           [IMAGE_DIRECTORY_ENTRY_IMPORT]
                                       .VirtualAddress);

    for (; importDesc->Name; importDesc++) {
        auto name = (PCHAR)pTempImage + importDesc->Name;

        if (strcasecmp(name, "KERNEL32.DLL") != 0 &&
            strcasecmp(name, "USER32.DLL") != 0)
            continue;

        // We'll mark this to import as already loaded.
        importDesc->Name |= 0x80000000;

        auto module = LoadLibraryA(name);
        assert(module != nullptr);

        for (auto thunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)pTempImage +
                                              importDesc->FirstThunk);
             thunk->u1.AddressOfData != 0;
             thunk++) {
            if (IMAGE_SNAP_BY_ORDINAL(thunk->u1.Ordinal)) {
                auto functionOrdinal = (LPCSTR)IMAGE_ORDINAL(thunk->u1.Ordinal);
                thunk->u1.Function =
                    (DWORD_PTR)GetProcAddress(module, functionOrdinal);
            } else {
                auto functionName =
                    (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)pTempImage +
                                            thunk->u1.AddressOfData);
                auto functionAddress =
                    (DWORD_PTR)GetProcAddress(module, functionName->Name);
                thunk->u1.Function = functionAddress;
            }
        }
    }

    auto hModule = GetModuleHandleA(nullptr);

    // Some variables we'd like to update.

#define TARGET_REF(N)                                                          \
    *(decltype(&N))((DWORD_PTR)&N - (DWORD_PTR)hModule + (DWORD_PTR)pTempImage)

    TARGET_REF(Args) = (char **)((DWORD_PTR)pTargetImage +
                                 ntHeader->OptionalHeader.SizeOfImage);

    // Write process memory.

    if (!WriteProcessMemory(hProcess,
                            pTargetImage,
                            pTempImage,
                            imageSize,
                            nullptr))
        return false;

    /*

    printf("hModule        %lx\n", hModule);
    printf("pTargetImage   %lx\n", pTargetImage);
    printf("Entry          %lx\n", (DWORD_PTR)EntryPoint - (DWORD_PTR)hModule);
    printf("StartAddr      %lx\n", ((DWORD_PTR)EntryPoint - (DWORD_PTR)hModule +
    (DWORD_PTR)pTargetImage));

    */

    CreateRemoteThread(hProcess,
                       nullptr,
                       0,
                       (LPTHREAD_START_ROUTINE)((DWORD_PTR)EntryPoint -
                                                (DWORD_PTR)hModule +
                                                (DWORD_PTR)pTargetImage),
                       pTargetImage,
                       0,
                       nullptr);

    return true;
}

bool Inject(DWORD processId, char *args[]) {
    auto hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, processId);

    if (hProcess == nullptr)
        return false;

    auto result = Inject(hProcess, args);

    CloseHandle(hProcess);

    return result;
}

bool ExecuteAndInject(const std::string &commandLine, char *args[]) {
    STARTUPINFO si{};
    PROCESS_INFORMATION pi{};

    if (!CreateProcessA(nullptr,
                        const_cast<char *>(commandLine.c_str()),
                        nullptr,
                        nullptr,
                        false,
                        CREATE_SUSPENDED,
                        nullptr,
                        nullptr,
                        &si,
                        &pi))
        return false;

    if (!Inject(pi.hProcess, args)) {
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        return false;
    }

    ResumeThread(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

// void ExecuteAndInject()
