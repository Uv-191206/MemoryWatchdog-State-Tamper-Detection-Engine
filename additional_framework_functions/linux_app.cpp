#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <unistd.h>     // Replaces windows.h (provides getpid())
#include <sys/types.h>  // Provides pid_t

void LogEvent(const std::string& message) {
    std::ofstream logFile("security_audit_log.txt", std::ios::app);
    if (logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        char timeStr[100];
        // std::localtime is safe, but std::localtime_r is preferred on Linux for thread safety
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        logFile << "[" << timeStr << "] " << message << std::endl;
        logFile.close();
    }
}

int main() {
    // Linux terminals use ANSI escape codes to set window titles instead of SetConsoleTitleA
    std::cout << "\033]0;Shipwreck Environment (Target)\007";

    int collectedWood = 0;
    int environmentState = 0;

    // pid_t replaces DWORD, getpid() replaces GetCurrentProcessId()
    pid_t pid = getpid();

    std::cout << "--- Shipwreck Island Survival ---" << std::endl;
    std::cout << "[INFO] Process ID (PID): " << pid << std::endl;
    std::cout << "[INFO] Address of 'environmentState': 0x" << std::hex << (uintptr_t)&environmentState << std::endl;
    std::cout << "---------------------------------" << std::endl;

    LogEvent("[APP] SYSTEM STARTUP: Shipwreck Island Environment initialized. (PID: " + std::to_string(pid) + ")");
    std::stringstream ss;
    ss << std::hex << (uintptr_t)&environmentState;
    LogEvent("[APP] STATE GENERATED: 'environmentState' allocated at 0x" + ss.str() + ".");

    // The Game Loop
    while (true) {

        // 1. Did the hack slip through?
        if (environmentState == 1) {
            std::cout << "\n[CRITICAL EVENT] The raft is built! You escaped the island!" << std::endl;
            break;
        }

        // 2. Did the Watchdog save us?
        else if (environmentState == 99) {
            std::cout << "\n\n[SYSTEM] WATCHDOG INTERVENTION DETECTED." << std::endl;
            std::cout << "[SYSTEM] Tampered state neutralized. Changes reversed." << std::endl;
            std::cout << "[SYSTEM] Process over / return 0 success. You may exit safely." << std::endl;
            LogEvent("[APP] SAFE SHUTDOWN: Watchdog signal received. Exiting cleanly.");
            break;
        }

        std::cout << "Exploring... Wood: " << std::dec << collectedWood << ". State: " << environmentState << "    \r" << std::flush;

        // usleep takes microseconds. 800 milliseconds = 800,000 microseconds.
        usleep(800000);
    }

    std::cout << "\nGame Over. Press Enter to exit." << std::endl;
    std::cin.get();
    return 0;
}
