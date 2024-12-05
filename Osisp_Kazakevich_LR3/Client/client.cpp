
#include <windows.h>
#include <iostream>
#include <cstring>

#define PIPE_NAME "\\\\.\\pipe\\MyPipe"
#define BUFFER_SIZE 1024

void client() {
    HANDLE hPipe;
    char buffer[BUFFER_SIZE];
    DWORD bytesWritten;

    hPipe = CreateFileA(
        PIPE_NAME,            
        GENERIC_WRITE,  
        0,                   
        nullptr,            
        OPEN_EXISTING,        
        0,                    
        nullptr               
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Не удалось подключиться к каналу. Ошибка: " << GetLastError() << std::endl;
        return;
    }

    for (int i = 0; i < 10; ++i) {
        std::snprintf(buffer, sizeof(buffer), "Привет от клиента, сообщение %d", i);


        if (!WriteFile(hPipe, buffer, std::strlen(buffer), &bytesWritten, nullptr)) {
            std::cerr << "Ошибка записи в канал. Ошибка: " << GetLastError() << std::endl;
            break;
        }

        Sleep(1000); 
    }

    strcpy_s(buffer, sizeof(buffer), "exit");
    WriteFile(hPipe, buffer, std::strlen(buffer), &bytesWritten, nullptr);

    CloseHandle(hPipe);
}

int main() {
    setlocale(LC_ALL, "Russian");
    client();
    return 0;
}
