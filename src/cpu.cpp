#include <iostream>
#include <iomanip>

#include "cpu.hpp"
#include "memory.hpp"
#include "semaphore.h"

CPU::CPU()
{
    IP = 0;
    SP = 63;
}

void CPU::run(Memory& memory, Semaphore& sem)
{
    while (true)
    {
        // Read next instruction according to value of IP.
        // Costs one cycle.
        // Increments IP by one.
        char instruction = get_instruction(memory);
        sem.wait();
        IP++;

        switch (instruction)
        {
            case INSTR_LDA:
                A = get_instruction(memory);
                sem.wait();
                IP++;
                break;
            case INSTR_MOV:
                break;
        }
    }
}

char CPU::get_instruction(Memory& memory)
{
    return memory.data[IP];
}

std::ostream& operator<<(std::ostream& stream, const CPU& cpu)
{
    stream << "A: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.A;
    stream << "   R1: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.R1;
    stream << "   R2: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.R2;
    stream << "   R3: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.R3;
    stream << "   IP: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.IP;
    stream << "   SP: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.SP;
    return stream;
}
