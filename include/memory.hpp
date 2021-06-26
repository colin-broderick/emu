#ifndef MEMORY_H
#define MEMORY_H

#include <array>
#include <iostream>

#include "utils.hpp"

class Memory
{
    public:
        Memory();

        void insert_rom(Byte start_address);

        std::array<Byte, 0xffff> data;
};

std::ostream& operator<<(std::ostream& stream, const Memory& memory);

#endif
