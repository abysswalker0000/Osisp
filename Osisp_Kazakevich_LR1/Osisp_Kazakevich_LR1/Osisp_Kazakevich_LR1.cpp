#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <chrono>

const int NUM_ITERATIONS = 1000;

DWORD WINAPI SortVectorMultipleTimes(LPVOID lpParam)
{
    std::vector<int>* vec = (std::vector<int>*)lpParam;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_ITERATIONS; i++) {

        std::random_shuffle(vec->begin(), vec->end());


        std::sort(vec->begin(), vec->end());


        if ((i + 1) % 100 == 0 && (i + 1) != NUM_ITERATIONS) {
            auto current_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed_time = current_time - start_time;

            std::cout << "Thread ID: " << GetCurrentThreadId() << " - Iteration " << i + 1 << " - Elapsed time: " << elapsed_time.count() << " ms.\n";
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> total_time = end_time - start_time;

    std::cout << "Thread ID: " << GetCurrentThreadId() << " - Total time for " << NUM_ITERATIONS << " iterations: " << total_time.count() << " ms.\n";
    return 0;
}

int main()
{
    srand((unsigned)time(0));

    std::vector<int> vec1(10000);
    std::vector<int> vec2(10000);
    std::vector<int> vec3(10000);

    for (int i = 0; i < 10000; i++) {
        int random_value = rand() % 10000;
        vec1[i] = random_value;
        vec2[i] = random_value;
        vec3[i] = random_value;
    }

    HANDLE threads[3];

    threads[0] = CreateThread(NULL, 0, SortVectorMultipleTimes, &vec1, 0, NULL);
    SetThreadPriority(threads[0], THREAD_PRIORITY_LOWEST);
    std::cout << "Created thread with ID: " << GetThreadId(threads[0]) << " - Priority: LOWEST\n";

    threads[1] = CreateThread(NULL, 0, SortVectorMultipleTimes, &vec2, 0, NULL);
    SetThreadPriority(threads[1], THREAD_PRIORITY_HIGHEST);
    std::cout << "Created thread with ID: " << GetThreadId(threads[1]) << " - Priority: HIGHEST\n";

    threads[2] = CreateThread(NULL, 0, SortVectorMultipleTimes, &vec3, 0, NULL);
    SetThreadPriority(threads[2], THREAD_PRIORITY_NORMAL);
    std::cout << "Created thread with ID: " << GetThreadId(threads[2]) << " - Priority: NORMAL\n";


    WaitForMultipleObjects(3, threads, TRUE, INFINITE);

    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    CloseHandle(threads[2]);

    return 0;
}
