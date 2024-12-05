
#include <windows.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <thread>
#include <atomic>

#define PIPE_NAME "\\\\.\\pipe\\MyPipe"
#define BUFFER_SIZE 1024

std::atomic<int> client_counter(1);

void logMessage(const std::string& logEntry) {
    std::ofstream logFile("server_log.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << logEntry << std::endl;
        logFile.close();
    }
    else {
        std::cerr << "�� ������� ������� ���� �������." << std::endl;
    }
}

void handle_client(HANDLE hPipe, int client_id) {
    char buffer[BUFFER_SIZE];
    DWORD bytesRead;

    while (true) {
        if (!ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) {
            std::cerr << "������ ������ �� ������ ��� ������� " << client_id << ". ������: " << GetLastError() << std::endl;
            break;
        }

        buffer[bytesRead] = '\0'; 

        std::time_t currentTime = std::time(nullptr);
        struct tm localTime;
        localtime_s(&localTime, &currentTime);
        char timeString[100];
        std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &localTime);
        std::string logEntry = "�����: " + std::string(timeString) + ", ������ " + std::to_string(client_id) + ": " + buffer;
        logMessage(logEntry);

        std::cout << "������ " << client_id << " �������� ���������: " << buffer << std::endl;

        
        if (std::strcmp(buffer, "exit") == 0) {
            std::cout << "������ ��������� ������ ��� ������� " << client_id << "." << std::endl;
            break;
        }
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}

void server() {
    while (true) {
 
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            BUFFER_SIZE,
            BUFFER_SIZE,
            0,
            nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            std::cerr << "�� ������� ������� ����������� �����. ������: " << GetLastError() << std::endl;
            return;
        }

        std::cout << "�������� ����������� �������..." << std::endl;

        if (ConnectNamedPipe(hPipe, nullptr)) {
            int client_id = client_counter++;
            std::cout << "������ " << client_id << " ���������." << std::endl;

           
            std::thread client_thread(handle_client, hPipe, client_id);
            client_thread.detach();  
        }
        else {
            CloseHandle(hPipe);
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    server();
    return 0;
}
