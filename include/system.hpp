#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>

#include "cpu.hpp"
#include "memory.hpp"
#include "semaphore.hpp"

class System
{
    public:
        // Attributes -----------------------------------------------------------------------------------------------------------
        CPU cpu;
        Memory memory;
        Semaphore sem;

        // Constructors ---------------------------------------------------------------------------------------------------------
        System();

        // General --------------------------------------------------------------------------------------------------------------
        void load_short_program(std::array<Byte, 128> program);
        void load_example_prog(unsigned int which);
        static inline void clock_function(Semaphore* cpu_sem, unsigned int cycles);
        void run();
};

#endif
