#include <iostream>

#include "memory.hpp"
#include "cpu.hpp"

int main()
{
    CPU cpu;
    Memory memory;
    memory.data = {0x01, 0x02};

    cpu.run(memory, 10);

    std::cout << cpu << std::endl;

    std::cout << memory << std::endl;
    return 0;
}

