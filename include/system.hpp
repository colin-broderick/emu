#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>
#include <fstream>

#include "cpu.hpp"
#include "ppu.hpp"
#include "memory.hpp"

class System
{
    public:
        // Attributes -----------------------------------------------------------------------------------------------------------
        CPU cpu;
        PPU ppu;
        Memory memory;

        // Constructors ---------------------------------------------------------------------------------------------------------
        System();

        // General --------------------------------------------------------------------------------------------------------------
        void load_example_prog(unsigned int which);
        bool load_rom(const std::string& filename);
        void run();
};

#endif
