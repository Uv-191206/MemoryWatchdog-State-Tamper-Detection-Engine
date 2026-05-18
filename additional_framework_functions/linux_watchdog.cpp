#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unistd.h>     // Replaces windows.h
#include <sys/types.h>  // Provides pid_t
#include <sys/ptrace.h> // Provides ptrace
#include <sys/wait.h>   // Provides waitpid
#include <fcntl.h>      // Provides open()
#include <dirent.h>     // Provides directory scanning for /proc
#include <signal.h>     // Provides kill()
#include <termios.h>    // For non-blocking keyboard input

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

// Linux equivalent to GetProcId: Iterates through /proc to match process name
pid_t GetProcId(const std::string& procName) {
    DIR* dir = opendir("/proc");
    if (!dir) return 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Check if directory name is numeric (a PID)
        std::string name(entry->d_name);
        if (std::all_of(name.begin(), name.end(), ::isdigit)) {
            std::string cmdPath = "/proc/" + name + "/comm";
            std::ifstream commFile(cmdPath);
            if (commFile.is_open()) {
                std::string currentName;
                std::getline(commFile, currentName);
                if (currentName == procName) {
                    closedir(dir);
                    return std::stoi(name);
                }
            }
        }
    }
    closedir(dir);
    return 0;
}

// Linux equivalent to _kbhit() using termios
bool kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF) {
        ungetc(ch, std::stdin);
        return true;
    }
    return false;
}

int main() {
    // Set console title via ANSI sequence
    std::cout << "\033]0;Watchdog Security Monitor - ACTIVE\007";
    std::cout << "--- Watchdog Security Monitor ---" << std::endl;

    // Looking for the Linux target (e.g., "app" instead of "app.exe")
    pid_t targetPID = GetProcId("app");
    if (targetPID == 0) {
        std::cout << "\033[31m[ERROR] app not found. Is it running?\033[0m" << std::endl;
        usleep(2000000);
        return 1;
    }

    std::cout << "\033[32m[SUCCESS] Found app (PID: " << targetPID << ")\033[0m" << std::endl;

    LogEvent("[WATCHDOG] INITIALIZED: Successfully attached to app target (PID: " + std::to_string(targetPID) + ").");

    uintptr_t targetAddress;
    std::cout << "Enter the memory address of 'environmentState' (e.g., 0x00BFF78): ";
    std::cin >> std::hex >> targetAddress;

    std::stringstream ss;
    ss << std::hex << targetAddress;
    LogEvent("[WATCHDOG] MONITORING: Actively scanning virtual memory address 0x" + ss.str() + ".");

    std::string memPath = "/proc/" + std::to_string(targetPID) + "/mem";
    int currentValue = 0;
    std::cout << "[INFO] Monitoring memory address..." << std::endl;

    // 4. The Active Monitoring Loop
    while (true) {
        bool accessSuccess = false;

        // Attach, read/write via /proc/[PID]/mem, and detach
        if (ptrace(PTRACE_ATTACH, targetPID, nullptr, nullptr) >= 0) {
            waitpid(targetPID, nullptr, 0);

            int memFd = open(memPath.c_str(), O_RDWR);
            if (memFd >= 0) {
                if (lseek64(memFd, targetAddress, SEEK_SET) != -1) {
                    if (read(memFd, &currentValue, sizeof(currentValue)) == sizeof(currentValue)) {
                        accessSuccess = true;
                    }
                }

                if (accessSuccess && currentValue == 1) {
                    // THE NEUTRALIZER
                    int lockdownSignal = 99;
                    lseek64(memFd, targetAddress, SEEK_SET);
                    write(memFd, &lockdownSignal, sizeof(lockdownSignal));

                    LogEvent("[WATCHDOG] SECURITY ALARM: Illegal state change detected at address 0x" + ss.str() + "!");
                    LogEvent("[WATCHDOG] ACTION TAKEN: Neutralizing signal sent. Forcing safe shutdown sequence.");

                    std::cout << "\033]0;!!! SECURITY BREACH DETECTED !!!\007";
                    std::cout << "\n\033[31m[VIOLATION] Illegal state change detected at address 0x" << std::hex << targetAddress << "\033[0m" << std::endl;
                    std::cout << "[ACTION] Hack neutralized. Signaled app to initiate safe shutdown." << std::endl;

                    close(memFd);
                    ptrace(PTRACE_DETACH, targetPID, nullptr, nullptr);

                    usleep(2500000);

                    // Terminate process with SIGKILL (replaces TerminateProcess)
                    kill(targetPID, SIGKILL);

                    std::cout << "\n>>> SYSTEM ALERT: Press ANY KEY to acknowledge threat and exit Watchdog <<<" << std::endl;

                    bool isRedBackground = true;
                    while (!kbhit()) {
                        if (isRedBackground) {
                            // Red background ANSI escape code, ring terminal bell escape sequence (\a)
                            std::cout << "\033[41m\033[37m" << "\a" << std::flush;
                        } else {
                            // Reset terminal attributes
                            std::cout << "\033[0m" << "\a" << std::flush;
                        }
                        isRedBackground = !isRedBackground;
                        usleep(500000); // Toggle alert every 500ms
                    }

                    // Consume the keypress
                    getchar();
                    LogEvent("[WATCHDOG] SYSTEM ALERT: Administrator acknowledged threat. Shutting down monitor.");
                    std::cout << "\033[0m" << std::endl; // Reset color cleanly
                    break;
                }
                close(memFd);
            }
            ptrace(PTRACE_DETACH, targetPID, nullptr, nullptr);
        }

        // Lightning Fast scanning: Scans every 10ms (10,000 microseconds)
        usleep(10000);
    }

    return 0;
}
