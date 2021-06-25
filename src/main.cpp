#include <iostream>
#include <thread>
#include <chrono>

#include "semaphore.hpp"
#include "memory.hpp"
#include "cpu.hpp"

int main()
{
    CPU cpu;
    Memory memory;
    Semaphore sem;
    memory.data = {0xa9, 0x01, 0x8d, 0x00, 0x02, 0xa9, 0x05, 0x8d, 0x01, 0x02, 0xa9, 0x08, 0x8d, 0x02, 0x02};

    std::thread clock_thread{Semaphore::clock_function, &sem, 30};

    cpu.run(memory, sem);

    clock_thread.join();
    return 0;
}
