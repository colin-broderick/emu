#include <iostream>
#include <iomanip>

#include "cpu.hpp"
#include "memory.hpp"
#include "semaphore.h"

#define DEBUG 1

CPU::CPU()
{
    IP = 0;
    SP = 63;
}

void CPU::LDA_set_CPU_flags()
{
    if (A && 0b10000000)
    {
        N = true;
    }
    if (A == 0)
    {
        Z = true;
    }
}

void CPU::run(Memory& memory, Semaphore& sem)
{
    while (true)
    {
        std::cout << (int)memory.data[0x0202] << "\n";
        if (DEBUG)
        {
            std::cout << *this << "\n";
            std::cout << memory << "\n";
        }
        // std::cout << "go\n";
        // Grab an instruction from RAM.
        Byte instruction = get_byte(memory);

        // What we just did costs a single CPU cycle, so consume one cycle.
        // We don't continue from here until another CPU cycle becomes available.
        sem.wait();

        // We increment the instruction pointer to point to the next byte in memory.
        IP++;

        if (DEBUG)
        {
            std::cout << *this << "\n";
            std::cout << memory << "\n";
        }

        switch (instruction)
        {
            case INSTR_LDA_IMMEDIATE:
                // If the instruction is LDA, we grab the next byte and store it in the acc.
                A = get_byte(memory);

                // Set CPU flags as appropriate.
                LDA_set_CPU_flags();

                // This consumes another clock cycle, so we wait here.
                sem.wait();

                // And we increment the IP again.
                IP++;

                if (DEBUG)
                {
                    std::cout << *this << "\n";
                    std::cout << memory << "\n";
                }

                break;

            case INSTR_LDA_ZEROPAGE:
                {
                    // Get the address and incrememnt PC
                    Byte data_address = get_byte(memory);
                    sem.wait();
                    IP++;
                    if (DEBUG)
                    {
                        std::cout << *this << "\n";
                        std::cout << memory << "\n";
                    }

                    // Load from the address into RAM
                    A = get_byte(memory, data_address);
                    LDA_set_CPU_flags();
                    sem.wait();
                    
                }
                break;

            case INSTR_LDA_ZEROPAGE_X:
                {
                    // get address
                    Byte data_address = get_byte(memory);
                    sem.wait();
                    IP++;

                    // add X
                    data_address += X;
                    sem.wait();

                    // load into A
                    A = get_byte(memory, data_address);
                    LDA_set_CPU_flags();
                    sem.wait();
                }
                break;

            case INSTR_LDA_ABSOLUTE:
                {
                    // Load address
                    Word data_address = get_word(memory);
                    sem.wait();
                    IP++;
                    sem.wait();
                    IP++;
                    
                    // Load from address into A.
                    A = get_byte(memory, data_address);
                    LDA_set_CPU_flags();
                    sem.wait();
                }
                break;

            case INSTR_STA_ABSOLUTE:
                {
                    // Load address
                    Word data_address = get_word(memory);
                    sem.wait();
                    IP++;
                    sem.wait();
                    IP++;

                    // Set value of memory address to A.
                    set_byte(memory, data_address, A);
                    sem.wait();
                }
                break;

            default:
                std::cout << "Unknown instruction: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)instruction << "\n";
                sem.wait();
                IP++;
                break;
        }
    }
}

void CPU::set_byte(Memory& memory, Word address, Byte value)
{
    std::cout << "Set memory address " << (int)address << " to value " << (int)value << "\n";
    std::cout << memory.data[address] << "\n";
    memory.data[address] = value;
    std::cout << memory.data[address] << "\n";

}

Byte CPU::get_byte(Memory& memory)
{
    return memory.data[IP];
}

Byte CPU::get_byte(Memory& memory, const Byte address)
{
    return memory.data[address];
}

Byte CPU::get_byte(Memory& memory, const Word address)
{
    return memory.data[address];
}

Word CPU::get_word(Memory& memory)
{
    Word val1 = (Word)memory.data[IP];
    Word val2 = (Word)memory.data[IP+1];
    Word val3 = (val1 << 8) | val2;
    std::cout << "get_word" << "\n";
    std::cout << val1 << " " << val2 << " " << val3 << "\n";
    return val3;
}




std::ostream& operator<<(std::ostream& stream, const CPU& cpu)
{
    stream << "A: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.A;
    stream << "   X: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.X;
    stream << "   Y: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.Y;
    stream << "   IP: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.IP;
    stream << "   SP: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.SP;
    return stream;
}
