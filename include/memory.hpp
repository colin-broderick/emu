#ifndef MEMORY_H
#define MEMORY_H

#include <array>
#include <iostream>

#include "utils.hpp"

class Memory
{
    public:
        // Attributes -----------------------------------------------------------------------------------------------------------
        std::array<Byte, 0xffff> data;

        // Constructors ---------------------------------------------------------------------------------------------------------
        Memory();

        // Operators ------------------------------------------------------------------------------------------------------------
        Byte& operator[](int index);

        // General --------------------------------------------------------------------------------------------------------------
        void insert_rom(Byte start_address);
};

std::ostream& operator<<(std::ostream& stream, Memory& memory);

#endif
