#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <random>

const int BUFFER_SIZE = 10;
const int NUM_WRITERS = 3;
const int NUM_READERS = 5;

int buffer[BUFFER_SIZE];
bool block_written[BUFFER_SIZE] = { false };
std::mutex bufferMutexes[BUFFER_SIZE];     // ћьютексы дл€ каждого блока буфера
std::mutex slotMutex;                      // ћьютекс дл€ синхронизации заполненных и пустых слотов
std::condition_variable slotCondition;     // ”словна€ переменна€ дл€ ожидани€ доступных слотов

int empty_slots = BUFFER_SIZE;
int filled_slots = 0;

std::atomic<long long> total_read_time(0);
std::atomic<long long> total_write_time(0);

void writer(int writer_id) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

    for (int i = 0; i < 5; ++i) {
        auto start_time = std::chrono::high_resolution_clock::now();  

        std::unique_lock<std::mutex> lock(slotMutex);
        slotCondition.wait(lock, [] { return empty_slots > 0; });

        int block_index = writer_id % BUFFER_SIZE; 
        {
            std::lock_guard<std::mutex> blockLock(bufferMutexes[block_index]);
            buffer[block_index] = dist(gen); 
            block_written[block_index] = true; 
            std::cout << "Writer " << writer_id << " wrote " << buffer[block_index] << " to block " << block_index << std::endl;
            --empty_slots;
            ++filled_slots;
        }

        slotCondition.notify_all();
        lock.unlock();

        auto end_time = std::chrono::high_resolution_clock::now();  
        total_write_time += std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void reader(int reader_id) {
    for (int i = 0; i < 5; ++i) {
        auto start_time = std::chrono::high_resolution_clock::now();  

        std::unique_lock<std::mutex> lock(slotMutex);
        slotCondition.wait(lock, [] { return filled_slots > 0; });

        int block_index = reader_id % BUFFER_SIZE; 
        {
            std::lock_guard<std::mutex> blockLock(bufferMutexes[block_index]);
            if (block_written[block_index]) {
                std::cout << "Reader " << reader_id << " read " << buffer[block_index] << " from block " << block_index << std::endl;
                block_written[block_index] = false; // —брасываем флаг тк прочитан
                ++empty_slots;
                --filled_slots;
            }
            else {
                std::cout << "Reader " << reader_id << " skipped block " << block_index << " (not written yet)" << std::endl;
            }
        }

        slotCondition.notify_all(); 
        lock.unlock();

        auto end_time = std::chrono::high_resolution_clock::now();  
        total_read_time += std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::this_thread::sleep_for(std::chrono::milliseconds(150)); 
    }
}

int main() {
    std::vector<std::thread> writers;
    std::vector<std::thread> readers;

    for (int i = 0; i < NUM_WRITERS; ++i) {
        writers.emplace_back(writer, i);
    }
    for (int i = 0; i < NUM_READERS; ++i) {
        readers.emplace_back(reader, i);
    }
    for (auto& w : writers) {
        w.join();
    }

    for (auto& r : readers) {
        r.join();
    }
    long long total_time = total_read_time + total_write_time;
    long long avg_read_time = total_read_time / (NUM_READERS * 5);
    long long avg_write_time = total_write_time / (NUM_WRITERS * 5);

    std::cout << "All writers and readers have completed their tasks." << std::endl;
    std::cout << "Total read time: " << total_read_time << " ms" << std::endl;
    std::cout << "Total write time: " << total_write_time << " ms" << std::endl;
    std::cout << "Average read time per read: " << avg_read_time << " ms" << std::endl;
    std::cout << "Average write time per write: " << avg_write_time << " ms" << std::endl;
    std::cout << "Total time spent on both read and write operations: " << total_time << " ms" << std::endl;

    return 0;
}
