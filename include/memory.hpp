#ifndef MEMORY_H
#define MEMORY_H

#include <array>
#include <iostream>

class Memory
{
    public:
        Memory();

        std::array<char, 64> data;
};

std::ostream& operator<<(std::ostream& stream, const Memory& memory);

#endif
