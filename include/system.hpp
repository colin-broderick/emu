#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>
#include <fstream>

#include "cpu.hpp"
#include "memory.hpp"

class System
{
public:
    // Attributes -----------------------------------------------------------------------------------------------------------
    Cpu6502 cpu;
    Memory memory;

    // Constructors ---------------------------------------------------------------------------------------------------------
    System();

    // General --------------------------------------------------------------------------------------------------------------
    void load_example_prog(uint32_t which);
    bool load_rom(const std::string &filename);
    void run();
};

#endif
