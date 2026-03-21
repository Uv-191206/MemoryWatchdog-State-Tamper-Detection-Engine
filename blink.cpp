#include <iostream>
#include <windows.h> // Required for Sleep() and system() commands

int main() {
    // Set a title for the terminal window
    SetConsoleTitleA("Emergency Alert");

    std::cout << "Blinking active. Press Ctrl+C to stop." << std::endl;

    // Infinite loop until manually exited
    while (true) {
        // Color 47: Red background (4), White text (7)
        system("color 47");
        Sleep(500); // Delay for 500ms

        // Color 07: Black background (0), White text (7)
        system("color 07");
        Sleep(500);
    }

    return 0;
}
