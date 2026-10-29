#include "common.hpp"

#include <string>
#include <iostream>

#ifdef _WIN32
// Windows-версия
int main() {
    HANDLE hPipeRead, hPipeWrite;
    SECURITY_ATTRIBUTES saAttr;
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFOA siStartInfo;   
    BOOL bSuccess = FALSE;
    std::string filename;

    std::cout << "[Parent PID " << GetCurrentProcessId() << "] Started WIN." << std::endl;

    std::cout << "[Parent PID " << GetCurrentProcessId() << "] Enter output filename: ";
    if (!std::getline(std::cin, filename)) {
        ERR_EXIT("getline filename");
    }

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        ERR_EXIT("CreatePipe");
    }

    if (!SetHandleInformation(hPipeWrite, HANDLE_FLAG_INHERIT, 0)) {
        ERR_EXIT("SetHandleInformation");
    }

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdInput = hPipeRead;
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    std::string cmdLine = "child.exe " + filename;

    bSuccess = CreateProcessA(
        NULL,                   
        (LPSTR)cmdLine.c_str(),
        NULL,                   
        NULL,                   
        TRUE,                   
        0,                      
        NULL,                   
        NULL,                   
        &siStartInfo,           
        &piProcInfo             
    );

    if (!bSuccess) {
        ERR_EXIT("CreateProcess");
    }

    CloseHandle(hPipeRead);

    std::cout << "[Parent PID " << GetCurrentProcessId() 
              << "] Enter numbers (space separated) or 'quit' to exit:" << std::endl;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "quit") break;
        line += "\r\n"; 

        DWORD dwWritten;
        if (!WriteFile(hPipeWrite, line.c_str(), (DWORD)line.size(), &dwWritten, NULL)) {
            ERR_EXIT("WriteFile");
        }
    }

    CloseHandle(hPipeWrite);

    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(piProcInfo.hProcess, &exitCode);
    std::cout << "[Parent PID " << GetCurrentProcessId() 
              << "] Child (PID " << piProcInfo.dwProcessId 
              << ") exited with status " << exitCode << std::endl;

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    return 0;
}

#else
int main() {
    int pipefd[2];
    pid_t cpid;
    std::string filename;
    pid_t parent_pid = getpid();

    std::cout << "[Parent PID" << parent_pid << "] Started WSL." << std::endl;

    std::cout << "[Parent PID" << parent_pid << "] Enter output filename: ";
    if (!std::getline(std::cin, filename)) {
        ERR_EXIT("getline filename");
    }

    if (pipe(pipefd) == -1) {
        ERR_EXIT("pipe");
    }

    cpid = fork();
    if (cpid == -1) {
        ERR_EXIT("fork");
    }

    if (cpid == 0) {
        if (close(pipefd[1]) == -1) ERR_EXIT("close write-end");
        if (dup2(pipefd[0], STDIN_FILENO) == -1) ERR_EXIT("dup2");
        if (close(pipefd[0]) == -1) ERR_EXIT("close old read-end");

        execl("./child", "child", filename.c_str(), (char *)NULL);
        ERR_EXIT("execl");
    } else {
        if (close(pipefd[0]) == -1) ERR_EXIT("close read-end");

        std::cout << "[Parent PID" << parent_pid 
                  << "] Enter numbers (space separated) or 'quit' to exit:" << std::endl;

        std::string line;
        while (std::getline(std::cin, line)) {
            if (line == "quit") break;
            line += '\n';

            const char *data = line.c_str();
            ssize_t len = static_cast<ssize_t>(line.size());
            ssize_t written = 0;
            while (written < len) {
                ssize_t w = write(pipefd[1], data + written, len - written);
                if (w == -1) {
                    if (errno == EINTR) continue;
                    ERR_EXIT("write pipe");
                }
                written += w;
            }
        }

        if (close(pipefd[1]) == -1) ERR_EXIT("close write-end");

        int status;
        if (waitpid(cpid, &status, 0) == -1) {
            ERR_EXIT("waitpid");
        }

        if (WIFEXITED(status)) {
            std::cout << "[Parent  PID" << parent_pid 
                      << "] Child (PID " << cpid 
                      << ") exited with status " 
                      << WEXITSTATUS(status) << std::endl;
        } else {
            std::cout << "[Parent PID" << parent_pid 
                      << "] Child terminated abnormally" << std::endl;
        }
    }

    return 0;
}
#endif