#ifndef SYSTEM_H
#define SYSTEM_H

#include "cpu.hpp"
#include "memory.hpp"
#include "semaphore.hpp"

class System
{
    public:
        System();

        void load_example_prog(unsigned int which);

        void run();

        CPU cpu;
        Memory memory;
        Semaphore sem;
};

#endif
