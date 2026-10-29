#include "common.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "[Child PID " 
#ifdef _WIN32
                  << GetCurrentProcessId()
#else
                  << getpid()
#endif
                  << "] Usage: " << argv[0] << " <filename>" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string filename = argv[1];

#ifdef _WIN32
    pid_t child_pid = GetCurrentProcessId();
#else
    pid_t child_pid = getpid();
#endif

    std::ofstream out_file(filename);
    if (!out_file) {
        std::cerr << "[Child PID " << child_pid 
                  << "] Cannot open file '" << filename << "'" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[Child PID " << child_pid 
              << "] Started. Reading from stdin, writing to '" 
              << filename << "'" << std::endl;

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        float value = 0.0f;
        float sum = 0.0f;
        int count = 0;

        while (iss >> value) {
            sum += value;
            ++count;
        }

        if (count > 0) {
            out_file << "Sum of " << count << " numbers: " << sum << "\n";
            std::cout << "[Child PID " << child_pid 
                      << "] Processed line, found " << count 
                      << " numbers, sum = " << sum << std::endl;
        } else {
            out_file << "No valid numbers found in line.\n";
            std::cout << "[Child PID " << child_pid 
                      << "] Processed line, no numbers found." << std::endl;
        }

        out_file.flush();
    }

    std::cout << "[Child PID " << child_pid << "] Finished and exited." << std::endl;
    return EXIT_SUCCESS;
}