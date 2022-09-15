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

int main() {
    std::string sProcessName;
    std::cout << "Process name: ";
    std::cin >> sProcessName;
    std::string sDLLPath;
    std::cout << "DLL path: ";
    std::cin >> sDLLPath;
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
            }
            CloseHandle(hProcess);
        }
    } else {
        std::cout << "Process not found" << std::endl;
    }
    return 0;
}