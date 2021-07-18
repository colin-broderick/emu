#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>
#include <fstream>

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
        void load_example_prog(unsigned int which);
        static inline void clock_function(Semaphore* cpu_sem, unsigned int cycles);
        bool load_rom(const std::string& filename);
        void run();
};

#endif
