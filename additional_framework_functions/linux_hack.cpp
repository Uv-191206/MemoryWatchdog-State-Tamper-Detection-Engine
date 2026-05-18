#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <unistd.h>     // Replaces windows.h
#include <sys/types.h>  // Provides pid_t
#include <sys/ptrace.h> // Provides ptrace for process manipulation
#include <sys/wait.h>   // Provides waitpid
#include <fcntl.h>      // Provides open(), O_WRONLY

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

    pid_t targetPID; // pid_t replaces DWORD
    uintptr_t targetAddress;

    std::cout << "Enter the Target PID: ";
    std::cin >> std::dec >> targetPID;

    std::cout << "Enter the Target Memory Address (Hex): ";
    std::cin >> std::hex >> targetAddress;

    std::stringstream ss;
    ss << std::hex << targetAddress;
    LogEvent("[HACK] INJECTION INITIATED: Attempting to overwrite memory at 0x" + ss.str() + " for PID " + std::to_string(targetPID) + ".");

    // Linux Security Note: Writing to another process requires root privileges (sudo) 
    // or specific capabilities (CAP_SYS_PTRACE).
    
    // Step 1: Attach to the target process using ptrace
    if (ptrace(PTRACE_ATTACH, targetPID, nullptr, nullptr) < 0) {
        std::cout << "[ERROR] Failed to attach to process. Ensure you run as sudo." << std::endl;
        LogEvent("[HACK] FAILED: Could not attach to target process.");
        return 1;
    }

    // Wait for the target process to stop so we can safely modify it
    waitpid(targetPID, nullptr, 0);

    int exploitValue = 1;
    std::cout << "Attempting to inject true state into memory..." << std::endl;

    // Step 2: Open the target process's memory file
    std::string memPath = "/proc/" + std::to_string(targetPID) + "/mem";
    int memFd = open(memPath.c_str(), O_WRONLY);

    bool success = false;
    if (memFd >= 0) {
        // Seek to the target memory address and write the value
        if (lseek64(memFd, targetAddress, SEEK_SET) != -1) {
            if (write(memFd, &exploitValue, sizeof(exploitValue)) == sizeof(exploitValue)) {
                success = true;
            }
        }
        close(memFd);
    }

    // Step 3: Detach from the process to let it resume execution
    ptrace(PTRACE_DETACH, targetPID, nullptr, nullptr);

    if (success) {
        std::cout << "[SUCCESS] Memory overwritten. Hidden event triggered." << std::endl;
        LogEvent("[HACK] PAYLOAD DELIVERED: Memory successfully overwritten. Hidden event triggered.");
    } else {
        std::cout << "[FAILED] Write operation blocked." << std::endl;
    }

    std::cout << "Press Enter to exit." << std::endl;
    std::cin.ignore();
    std::cin.get();
    return 0;
}
