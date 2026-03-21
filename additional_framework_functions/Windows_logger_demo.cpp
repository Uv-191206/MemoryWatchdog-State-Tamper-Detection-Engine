#include <iostream>
#include <fstream> // Required for file handling
#include <string>
#include <ctime>   // Required for getting the system time

// A custom function we can call anytime we want to save a log
void LogEvent(const std::string& message) {
    // 1. Open the file in "Append" mode (std::ios::app)
    // If the file doesn't exist, C++ will create it automatically.
    std::ofstream logFile("security_audit_log.txt", std::ios::app);

    if (logFile.is_open()) {
        // 2. Get the current computer time
        std::time_t now = std::time(nullptr);
        char timeStr[100];
        // Format the time cleanly (e.g., "2026-03-18 14:30:05")
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        // 3. Write to the file just like you write to std::cout
        logFile << "[" << timeStr << "] " << message << std::endl;

        // 4. Always close the door behind you to save the file
        logFile.close();
    } else {
        std::cout << "Error: Could not open log file!" << std::endl;
    }
}

int main() {
    std::cout << "Running test program... check your folder for the log file." << std::endl;

    LogEvent("SYSTEM STARTUP: Initialization sequence begun.");
    LogEvent("INFO: Scanning for rogue processes.");
    LogEvent("CRITICAL: Unauthorized memory access detected!");

    return 0;
}
