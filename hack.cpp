#include <iostream>
#include <windows.h>
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

int main() {
    std::cout << "--- State Injector ---" << std::endl;

    DWORD targetPID;
    uintptr_t targetAddress;

    std::cout << "Enter the Target PID: ";
    std::cin >> std::dec >> targetPID;

    std::cout << "Enter the Target Memory Address (Hex): ";
    std::cin >> std::hex >> targetAddress;

    std::stringstream ss;
    ss << std::hex << targetAddress;
    LogEvent("[HACK] INJECTION INITIATED: Attempting to overwrite memory at 0x" + ss.str() + " for PID " + std::to_string(targetPID) + ".");

    HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, targetPID);
    if (hProcess == NULL) {
        std::cout << "[ERROR] Failed to get handle. Code: " << GetLastError() << std::endl;
        LogEvent("[HACK] FAILED: Could not obtain handle to target process.");
        return 1;
    }

    // UPGRADE: Changed to 'int 1' to match the new environmentState logic
    int exploitValue = 1;
    std::cout << "Attempting to inject true state into memory..." << std::endl;

    BOOL success = WriteProcessMemory(hProcess, (LPVOID)targetAddress, &exploitValue, sizeof(exploitValue), nullptr);

    if (success) {
        std::cout << "[SUCCESS] Memory overwritten. Hidden event triggered." << std::endl;
        LogEvent("[HACK] PAYLOAD DELIVERED: Memory successfully overwritten. Hidden event triggered.");
    } else {
        std::cout << "[FAILED] Write operation blocked." << std::endl;
    }

    CloseHandle(hProcess);
    std::cout << "Press Enter to exit." << std::endl;
    std::cin.ignore();
    std::cin.get();
    return 0;
}
