#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcessID(std::string sProcessName) {
    DWORD dwProcessID = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &entry)) {
            while (Process32Next(hSnapshot, &entry)) {
                std::wstring wstr(entry.szExeFile);
                std::string str(wstr.begin(), wstr.end());
                if (str == sProcessName) {
                    dwProcessID = entry.th32ProcessID;
                    break;
                }
            }
        }
        CloseHandle(hSnapshot);
    }
    return dwProcessID;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "usage: injector.exe <process.exe> <hack.dll>\n";
        std::cout << "process.exe: target process into which the DLL ";
        std::cout << "file will be injected\n";
        std::cout << "hack.dll: path to DLL file\n";
        return 0;
    }
    std::string sProcessName = argv[2];
    std::string sDLLPath = argv[3];
    DWORD dwProcessID = GetProcessID(sProcessName);
    if (dwProcessID) {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, dwProcessID);
        if (hProcess) {
            LPVOID DLLAddress = VirtualAllocEx(hProcess, 0,
                sDLLPath.length(), MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE);
            WriteProcessMemory(hProcess, DLLAddress, sDLLPath.c_str(),
                sDLLPath.length() + 1, 0);
            HANDLE hThread = CreateRemoteThread(hProcess, 0, 0,
                (LPTHREAD_START_ROUTINE)LoadLibraryA, DLLAddress, 0, 0);
            if (hThread) {
                CloseHandle(hThread);
            } else {
                std::cout << "Failed to open DLL thread" << std::endl;
            }
            CloseHandle(hProcess);
        } else {
            std::cout << "Failed to open process" << std::endl;
        }
        std::cout << "DLL injected successfully" << std::endl;
    } else {
        std::cout << "Process not found" << std::endl;
    }
    return 0;
}