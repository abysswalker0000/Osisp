#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <Windows.h>

void processMemoryMappedFile(LPVOID mappedView, SIZE_T start, SIZE_T end, size_t& count) {
    char* data = static_cast<char*>(mappedView);
    for (SIZE_T i = start; i < end; ++i) {
        if (data[i] == 'a') {
            ++count;
        }
    }
}

// мапед
void memoryMappedFileProcessing(const char* filename, int threadCount) {
    HANDLE hFile = CreateFileA(
        filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    DWORD fileSizeHigh;
    DWORD fileSizeLow = GetFileSize(hFile, &fileSizeHigh);
    SIZE_T fileSize = ((SIZE_T)fileSizeHigh << 32) | fileSizeLow;

    HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMapping) {
        std::cerr << "Error creating file mapping." << std::endl;
        CloseHandle(hFile);
        return;
    }

    LPVOID mappedView = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!mappedView) {
        std::cerr << "Error mapping view of file." << std::endl;
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }


    SIZE_T chunkSize = fileSize / threadCount;
    std::vector<std::thread> threads;
    std::vector<size_t> counts(threadCount, 0);  

    for (int i = 0; i < threadCount; ++i) {
        SIZE_T start = i * chunkSize;
        SIZE_T end = (i == threadCount - 1) ? fileSize : start + chunkSize;
        threads.emplace_back(processMemoryMappedFile, mappedView, start, end, std::ref(counts[i]));
    }
    for (auto& thread : threads) {
        thread.join();
    }

    size_t totalCount = 0;
    for (const auto& count : counts) {
        totalCount += count;
    }

    std::cout << "Total number of 'a' in file (memory-mapped): " << totalCount << std::endl;

    UnmapViewOfFile(mappedView);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

// традиционный
void traditionalFileProcessing(const char* filename, int threadCount) {
    FILE* file;
    if (fopen_s(&file, filename, "rb") != 0 || !file) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    fseek(file, 0, SEEK_END);
    SIZE_T fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = new char[fileSize];
    fread(buffer, 1, fileSize, file);
    fclose(file);

    SIZE_T chunkSize = fileSize / threadCount;
    std::vector<std::thread> threads;
    std::vector<size_t> counts(threadCount, 0); 

    for (int i = 0; i < threadCount; ++i) {
        SIZE_T start = i * chunkSize;
        SIZE_T end = (i == threadCount - 1) ? fileSize : start + chunkSize;
        threads.emplace_back([&](SIZE_T start, SIZE_T end, size_t& count) {
            for (SIZE_T i = start; i < end; ++i) {
                if (buffer[i] == 'a') {
                    ++count;
                }
            }
            }, start, end, std::ref(counts[i]));
    }


    for (auto& thread : threads) {
        thread.join();
    }


    size_t totalCount = 0;
    for (const auto& count : counts) {
        totalCount += count;
    }

    std::cout << "Total number of 'a' in file (traditional): " << totalCount << std::endl;

    delete[] buffer;
}


template<typename Func>
void measureExecutionTime(const char* filename, Func&& func, int threadCount) {
    auto start = std::chrono::high_resolution_clock::now();
    func(filename, threadCount);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;
}

int main() {
    const char* filename = "test.txt";
    int threadCount = 20; 

    std::cout << "Memory-mapped file processing:" << std::endl;
    measureExecutionTime(filename, memoryMappedFileProcessing, threadCount);

    std::cout << "\nTraditional file processing:" << std::endl;
    measureExecutionTime(filename, traditionalFileProcessing, threadCount);

    return 0;
}