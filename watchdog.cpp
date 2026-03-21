#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <conio.h>
#include <cstdlib>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>

void LogEvent(const std::string& message) {
    std::ofstream logFile("security_audit_log.txt", std::ios::app);
    if (logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        char timeStr[100];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        logFile << "[" << timeStr << "] " << message << std::endl;
        logFile.close();
    }
}

DWORD GetProcId(const wchar_t* procName) {
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W procEntry;
        procEntry.dwSize = sizeof(procEntry);
        if (Process32FirstW(hSnap, &procEntry)) {
            do {
                if (!_wcsicmp(procEntry.szExeFile, procName)) {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

int main() {
    SetConsoleTitleA("Watchdog Security Monitor - ACTIVE");
    std::cout << "--- Watchdog Security Monitor ---" << std::endl;

    DWORD targetPID = GetProcId(L"app.exe");
    if (targetPID == 0) {
        system("color 0C");
        std::cout << "[ERROR] app.exe not found. Is it running?" << std::endl;
        Sleep(2000);
        system("color 07");
        return 1;
    }

    system("color 0A");
    std::cout << "[SUCCESS] Found app.exe (PID: " << targetPID << ")" << std::endl;
    system("color 07");

    LogEvent("[WATCHDOG] INITIALIZED: Successfully attached to app.exe target (PID: " + std::to_string(targetPID) + ").");

    uintptr_t targetAddress;
    std::cout << "Enter the memory address of 'environmentState' (e.g., 0x00BFF78): ";
    std::cin >> std::hex >> targetAddress;

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, targetPID);
    if (hProcess == NULL) {
        std::cout << "[ERROR] Could not open process. Try running as Administrator." << std::endl;
        return 1;
    }

    int currentValue = 0;
    std::cout << "[INFO] Monitoring memory address..." << std::endl;

    std::stringstream ss;
    ss << std::hex << targetAddress;
    LogEvent("[WATCHDOG] MONITORING: Actively scanning virtual memory address 0x" + ss.str() + ".");

    // 4. The Active Monitoring Loop
    while (true) {
        ReadProcessMemory(hProcess, (BYTE*)targetAddress, &currentValue, sizeof(currentValue), nullptr);

        if (currentValue == 1) {

            // THE NEUTRALIZER
            int lockdownSignal = 99;
            WriteProcessMemory(hProcess, (LPVOID)targetAddress, &lockdownSignal, sizeof(lockdownSignal), nullptr);

            LogEvent("[WATCHDOG] SECURITY ALARM: Illegal state change detected at address 0x" + ss.str() + "!");
            LogEvent("[WATCHDOG] ACTION TAKEN: Neutralizing signal sent. Forcing safe shutdown sequence.");

            SetConsoleTitleA("!!! SECURITY BREACH DETECTED !!!");
            std::cout << "\n[VIOLATION] Illegal state change detected at address 0x" << std::hex << targetAddress << std::endl;
            std::cout << "[ACTION] Hack neutralized. Signaled app.exe to initiate safe shutdown." << std::endl;

            Sleep(2500);

            HANDLE hKill = OpenProcess(PROCESS_TERMINATE, FALSE, targetPID);
            if (hKill != NULL) {
                TerminateProcess(hKill, 1);
                CloseHandle(hKill);
            }

            std::cout << "\n>>> SYSTEM ALERT: Press ANY KEY to acknowledge threat and exit Watchdog <<<" << std::endl;

            bool isRedBackground = true;

            while (!_kbhit()) {
                if (isRedBackground) {
                    system("color 47");
                    Beep(880, 500);
                } else {
                    system("color 07");
                    Beep(659, 500);
                }
                isRedBackground = !isRedBackground;
            }

            _getch();
            LogEvent("[WATCHDOG] SYSTEM ALERT: Administrator acknowledged threat. Shutting down monitor.");
            system("color 07");
            break;
        }

        // THE FIX: Lightning Fast EDR scanning. Scans every 10ms.
        Sleep(10);
    }

    CloseHandle(hProcess);
    return 0;
}
