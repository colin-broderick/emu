#include <iostream>
#include <thread>
#include <chrono>

#include "semaphore.h"
#include "memory.hpp"
#include "cpu.hpp"

int main()
{
    CPU cpu;
    Memory memory;
    Semaphore sem;
    memory.data = {0x01, 0x02};

    std::thread clock_thread{Semaphore::clock_function, &sem};

    cpu.run(memory, sem);

    return 0;
}

