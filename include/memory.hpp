#ifndef MEMORY_H
#define MEMORY_H

#include <array>
#include <iostream>
#include <mutex>

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
        friend std::ostream& operator<<(std::ostream& stream, Memory& memory);

        // General --------------------------------------------------------------------------------------------------------------
        void insert_rom(Byte start_address);

    private:
        std::mutex mem_lock;
};

std::ostream& operator<<(std::ostream& stream, Memory& memory);

#endif
