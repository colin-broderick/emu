#include <iostream>
#include <iomanip>

#include "cpu.hpp"
#include "memory.hpp"
#include "semaphore.hpp"

#define DEBUG 1

CPU::CPU()
{
    IP = 0x0000;
    SP = 0x01FF;
}

void CPU::LDA_set_CPU_flags()
{
    N = (A & 0b10000000);
    Z = (A == 0);
}

void CPU::ORA_set_CPU_flags()
{
    LDA_set_CPU_flags();
}

void CPU::LDX_set_CPU_flags()
{
    N = (X & 0b10000000);
    Z = (X == 0);
}

void CPU::LDY_set_CPU_flags()
{
    N = (Y & 0b10000000);
    Z = (Y == 0);
}

void CPU::TAX_set_CPU_flags()
{
    LDX_set_CPU_flags();
}

void CPU::TXA_set_CPU_flags()
{
    LDA_set_CPU_flags();
}

void CPU::run(Memory& memory)
{
    while (true)
    {
        #if DEBUG
        std::cout << *this << "\n";
        std::cout << memory << "\n";
        #endif

        // TODO Interrupt handler should go here.

        // Grab an instruction from RAM.
        Byte instruction = get_byte(memory);

        // What we just did costs a single CPU cycle, so consume one cycle.
        // We don't continue from here until another CPU cycle becomes available.
        sem.wait();

        // We increment the instruction pointer to point to the next byte in memory.
        IP++;

        switch (instruction)
        {
            case INSTR_6502_LDA_IMMEDIATE:
                {
                    // If the instruction is LDA, we grab the next byte and store it in the acc.
                    A = get_byte(memory);

                    // Set CPU flags as appropriate.
                    LDA_set_CPU_flags();

                    // This consumes another clock cycle, so we wait here.
                    sem.wait();

                    // And we increment the IP again.
                    IP++;
                }
                break;

            case INSTR_6502_LDA_ZEROPAGE:
                {
                    // Get the address and incrememnt PC
                    Byte data_address = get_byte(memory);
                    sem.wait();
                    IP++;

                    // Load from the address into RAM
                    A = get_byte(memory, data_address);
                    LDA_set_CPU_flags();
                    sem.wait();
                    
                }
                break;

            case INSTR_6502_LDA_ZEROPAGE_X:
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

            case INSTR_6502_LDA_ABSOLUTE:
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

            case INSTR_6502_LDA_ABSOLUTE_X:
                {

                    Word load_from_address = get_word(memory);
                    Byte page1 = load_from_address >> 8;
                    load_from_address += X;
                    Byte page2 = load_from_address >> 8;
                    
                    IP++;
                    IP++;

                    A = get_byte(memory, load_from_address);

                    LDA_set_CPU_flags();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (page1 != page2)
                    {
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_LDA_ABSOLUTE_Y:
                {

                    Word load_from_address = get_word(memory);
                    Byte page1 = load_from_address >> 8;
                    load_from_address += Y;
                    Byte page2 = load_from_address >> 8;
                    
                    IP++;
                    IP++;

                    A = get_byte(memory, load_from_address);

                    LDA_set_CPU_flags();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (page1 != page2)
                    {
                        sem.wait();
                    }
                }
                break;
            
            case INSTR_6502_LDA_INDIRECT_X:
                {
                    // Read next byte to get indirect address and jump over it
                    Byte indirect_address = get_byte(memory) + X;
                    sem.wait();
                    IP++;

                    // Get the data from other address
                    Word target_address = get_word_zpg_wrap(memory, indirect_address);
                    sem.wait();

                    // Get the data to be ORed with A.
                    A = get_byte(memory, target_address);
                    sem.wait();

                    LDA_set_CPU_flags();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_LDA_INDIRECT_Y:
                {
                    Byte indirect_address = get_byte(memory);
                    IP++;

                    Word address = get_word(memory, indirect_address);
                    Byte page1 = address >> 8;
                    address += Y;
                    Byte page2 = address >> 8;

                    LDA_set_CPU_flags();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();

                    if (page1 != page2)
                    {
                        sem.wait();
                    }
                }
                break;

            case INSTR_6502_LDY_IMMEDIATE:
                {
                    // If the instruction is LDA, we grab the next byte and store it in the acc.
                    Y = get_byte(memory);
                    LDY_set_CPU_flags();
                    sem.wait();
                    IP++;
                }
                break;

            case INSTR_6502_STA_ABSOLUTE:
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

            case INSTR_6502_STA_ABSOLUTE_Y:
                {
                    // get address in next two bytes
                    Word target_address = get_word(memory);
                    sem.wait();
                    IP++;
                    sem.wait();
                    IP++;

                    // Add Y to address
                    target_address += Y;
                    sem.wait();

                    // Store A in address
                    memory[target_address] = A;
                    sem.wait();
                }
                break;

            case INSTR_6502_STA_ZEROPAGE:
                {
                    // get address
                    Byte data_address = get_byte(memory);
                    sem.wait();
                    IP++;
                    
                    // get data from address
                    set_byte(memory, data_address, A);
                    sem.wait();
                }
                break;

            case INSTR_6502_STX_ABSOLUTE:
                {
                    // Load address
                    Word data_address = get_word(memory);
                    sem.wait();
                    IP++;
                    sem.wait();
                    IP++;

                    // Set value of memory address to A.
                    set_byte(memory, data_address, X);
                    sem.wait();
                }
                break;

            case INSTR_6502_STY_ABSOLUTE:
                {
                    // Load address
                    Word data_address = get_word(memory);
                    sem.wait();
                    IP++;
                    sem.wait();
                    IP++;

                    // Set value of memory address to A.
                    set_byte(memory, data_address, Y);
                    sem.wait();
                }
                break;

            case INSTR_6502_TAX:
                // Copy A into X.
                X = A;
                TAX_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_TXA:
                A = X;
                TXA_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_INX:
                // Increment X.
                X++;
                LDX_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_INY:
                Y++;
                LDY_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_LDX_IMMEDIATE:
                // Load data into X.
                X = get_byte(memory);
                LDX_set_CPU_flags();
                sem.wait();
                IP++;
                break;

            case INSTR_6502_DEX:
                X--;
                LDX_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_DEY:
                Y--;
                LDY_set_CPU_flags();
                sem.wait();
                break;

            case INSTR_6502_CPX_IMMEDIATE:
                {
                    Word result = (Word)X - (Word)get_byte(memory);
                    if (result >= 0)
                    {
                        C = true;
                        if (result == 0)
                        {
                            Z = true;
                        }
                        if (result & 0b10000000)
                        {
                            N = true;
                        }
                    }
                    sem.wait();
                    IP++;
                }
                break;

            case INSTR_6502_CPY_IMMEDIATE:
                {
                    Word result = (Word)Y - (Word)get_byte(memory);
                    if (result >= 0)
                    {
                        C = true;
                        if (result == 0)
                        {
                            Z = true;
                        }
                        if (result & 0b10000000)
                        {
                            N = true;
                        }
                    }
                    sem.wait();
                    IP++;
                }
                break;

            case INSTR_6502_BNE_RELATIVE:
                {
                    // Remember the starting page so we know if we've moved to a new page.
                    Byte current_page = IP >> 8;
                    
                    bool jump = !Z;
                    sem.wait();

                    if (jump)
                    {
                        Byte dist = get_byte(memory);
                        if (dist & 0x80)
                        {
                            // If jump is negative get the two's complement and subtract the result.
                            dist = ~dist;
                            dist += 1;
                            IP = IP - dist;
                        }
                        else
                        {
                            // If the jump is positive, do the jump.
                            IP = IP + dist;
                        }
                        sem.wait();
                    }
                    IP++;

                    // This should take two additional clock cycles if the branch leads to a new page.
                    Byte new_page = IP >> 8;
                    if (current_page != new_page)
                    {
                        sem.wait();
                        sem.wait();
                    }

                }
                break;

            case INSTR_6502_SED:
                D = true;
                sem.wait();
                break;

            case INSTR_6502_ORA_INDIRECT_X:
                {
                    // Read next byte to get indirect address and jump over it
                    Byte indirect_address = get_byte(memory) + X;
                    sem.wait();
                    IP++;

                    // Get the data from other address
                    Word target_address = get_word(memory, indirect_address);
                    sem.wait();

                    // Get the data to be ORed with A.
                    Byte data = get_byte(memory, target_address);
                    sem.wait();

                    // Do the OR, putting result in A.
                    A |= data;
                    sem.wait();

                    ORA_set_CPU_flags();
                    sem.wait();
                }
                break;

            case INSTR_6502_ADC_IMMEDIATE:
                {
                    // TODO: Explain this. What I've done is mostly based on
                    // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
                    Byte data = get_byte(memory);
                    IP++;

                    Word result = (Word)data + (Word)A + (Word)C;       // Result = accumulator + data_from_memory + carry_flag
                    Z = result == 0;                                    // Set Z if result is zero
                    C = result > 255;                                   // Set C if result overflowed, i.e. didn't fit in eight bits
                    N = result & 0b10000000;                            // Set N if result is negative (i.e. sign bit is on)
                    V = ((A ^ result) & (data ^ result) & 0x80) != 0;   // TODO: This part is not fully understood.
                    A = (Byte)(result & 0xFF);                          // Accumulator is set to the part of the result that fits in eight bits.
                    sem.wait();
                }
                break;

            case INSTR_6502_CLD:
                D = false;
                sem.wait();
                break;

            case INSTR_6502_CLI:
                I = false;
                sem.wait();
                break;

            case INSTR_6502_CLC:
                C = false;
                sem.wait();
                break;

            case INSTR_6502_CLV:
                V = false;
                sem.wait();
                break;

            case INSTR_6502_PHA:
                memory[SP] = A;
                sem.wait();
                SP--;
                sem.wait();
                break;

            case INSTR_6502_PHP:
                memory[SP] = flags_as_byte();
                sem.wait();
                SP--;
                sem.wait();
                break;

            case INSTR_6502_PLA:
                A = memory[SP];
                sem.wait();
                SP++;
                sem.wait();
                sem.wait();
                LDA_set_CPU_flags();
                break;

            case INSTR_6502_NOP:
                sem.wait();
                break;

            case INSTR_6502_BRK:
                // TODO: Why does this take seven cycles?
                sem.wait();
                sem.wait();
                sem.wait();
                sem.wait();
                sem.wait();
                sem.wait();
                B = true;
                set_byte(memory, SP, IP);
                SP--;
                set_byte(memory, SP, flags_as_byte());
                std::cout << "BRK reached" << std::endl;
                return;

            case INSTR_6502_JSR_ABSOLUTE:
                {
                    // Pushes (address minus one) of the return point onto the stack then sets program counter to target address
                    Word target_address = get_word(memory);
                    IP++;
                    memory[SP] = (IP >> 8);
                    SP--;
                    memory[SP] = (IP & 0xFF);
                    SP--;
                    IP = target_address;
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_RTS:
                {
                    SP++;
                    Word pointer = get_word(memory, SP);
                    SP++;

                    IP = pointer + 1;

                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;
            
            case INSTR_6502_JMP_ABSOLUTE:
                {
                    IP = get_word(memory);
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_INC_ABSOLUTE:
                {
                    Word target_address = get_word(memory);
                    memory[target_address]++;
                    IP++;
                    IP++;
                    N = (memory[target_address] & 0x80);  // Set N on if sign bit of result is set.
                    Z = (memory[target_address] == 0);    // Set Z on if result is zero.
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_INC_ABSOLUTE_X:
                {
                    Word target_address = get_word(memory);
                    target_address += X;
                    memory[target_address]++;
                    IP++;
                    IP++;
                    N = (memory[target_address] & 0x80);  // Set N on if sign bit of result is set.
                    Z = (memory[target_address] == 0);    // Set Z on if result is zero.
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_DEC_ABSOLUTE:
                {
                    Word target_address = get_word(memory);
                    memory[target_address]--;
                    IP++;
                    IP++;
                    N = (memory[target_address] & 0x80);  // Set N on if sign bit of result is set.
                    Z = (memory[target_address] == 0);    // Set Z on if result is zero.
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_DEC_ABSOLUTE_X:
                {
                    Word target_address = get_word(memory);
                    target_address += X;
                    memory[target_address]--;
                    IP++;
                    IP++;
                    N = (memory[target_address] & 0x80);  // Set N on if sign bit of result is set.
                    Z = (memory[target_address] == 0);    // Set Z on if result is zero.
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_JMP_INDIRECT:
                {
                    Word lookup_address = get_word(memory);
                    IP = get_word(memory, lookup_address);

                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;
            
            case INSTR_6502_AND_IMMEDIATE:
                {
                    Byte operand = get_byte(memory);
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    IP++;
                }
                break;

            case INSTR_6502_AND_ZEROPAGE_X:
                {
                    //get address, add X, wrap if necessary
                    Byte lookup_address = (get_byte(memory) + X) & 0xff;
                    Byte operand = get_byte(memory, lookup_address);
                    IP++;
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_AND_ZEROPAGE:
                {
                    Byte lookup_address = get_byte(memory);
                    Byte operand = get_byte(memory, lookup_address);
                    IP++;
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                }
                break;

            case INSTR_6502_AND_ABSOLUTE:
                {
                    Word lookup_address = get_word(memory);
                    Byte operand = get_byte(memory, lookup_address);
                    IP++;
                    IP++;
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;
            
            case INSTR_6502_AND_ABSOLUTE_X:
                {
                    Word lookup_address = get_word(memory);
                    Word old_page = lookup_address & 0xFF00;
                    lookup_address += X;
                    Word new_page = lookup_address & 0xFF00;
                    Byte operand = get_byte(memory, lookup_address);
                    IP++;
                    IP++;
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (old_page != new_page) 
                    {
                        sem.wait();
                    }
                }
                break;
            
            case INSTR_6502_AND_ABSOLUTE_Y:
                {
                    Word lookup_address = get_word(memory);
                    Word old_page = lookup_address & 0xFF00;
                    lookup_address += Y;
                    Word new_page = lookup_address & 0xFF00;
                    Byte operand = get_byte(memory, lookup_address);
                    IP++;
                    IP++;
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (old_page != new_page) 
                    {
                        sem.wait();
                    }
                }
                break;
            
            case INSTR_6502_AND_INDIRECT_X:
                {
                    // read next byte and add X without carry
                    Byte indirect_address = get_byte(memory) + X;
                    IP++;

                    //get target address from indirect_address data and next on zero page
                    Word target_address = get_word_zpg_wrap(memory, indirect_address);

                    // get data from target address
                    Byte operand = get_byte(memory, target_address);

                    //complete operation
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                }
                break;
            
            case INSTR_6502_AND_INDIRECT_Y:
                {
                    Byte indirect_address = get_byte(memory);
                    IP++;
                    Word target_address = get_word_zpg_wrap(memory, indirect_address);
                    Byte page1 = target_address >> 8;
                    target_address += Y;
                    Byte page2 = target_address >> 8;

                    Byte operand = get_byte(memory, target_address);

                    //complete operation
                    A = A & operand;
                    if (A == 0)
                    {
                        Z = true;
                    }
                    if (A & 0b10000000)
                    {
                        N = true;
                    }
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    sem.wait();
                    if (page1 != page2)
                    {
                        sem.wait();
                    }
                }
                break;

            default:
                std::cout << "Unknown instruction: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)instruction << "\n";
                return;
        }
    }
}

void CPU::set_byte(Memory& memory, Word address, Byte value)
{
    memory[address] = value;
}

Byte CPU::get_byte(Memory& memory)
{
    return memory[IP];
}

Byte CPU::get_byte(Memory& memory, const Byte address)
{
    return memory[address];
}

Byte CPU::get_byte(Memory& memory, const Word address)
{
    return memory[address];
}

Word CPU::get_word(Memory& memory)
{
    Word val1 = (Word)memory[IP];
    Word val2 = (Word)memory[IP+1];
    return (val2 << 8) | val1;
}

Word CPU::get_word(Memory& memory, const Byte address)
{
    Word val1 = (Word)memory[address];
    Word val2 = (Word)memory[(Word)address+1];
    return (val2 << 8) | val1;
}

Word CPU::get_word(Memory& memory, const Word address)
{
    Word val1 = (Word)memory[address];
    Word val2 = (Word)memory[address+1];
    return (val2 << 8) | val1;
}

Word CPU::get_word_zpg_wrap(Memory& memory, const Byte address)
{
    Word val1 = (Word)memory[address % 256];
    Word val2 = (Word)memory[(address+1) % 256];   // This wraps automatically since address is a Byte
    return (val2 << 8) | val1;
}

Byte CPU::get_data_indexed_indirect(Memory& memory, const Byte index)
{
    // read next byte and add index without carry
    Byte indirect_address = get_byte(memory) + index;

    //get target address from indirect_address data and next on zero page
    Word target_address = get_word_zpg_wrap(memory, indirect_address);

    // get data from target address and return
    return get_byte(memory, target_address);
}

Byte CPU::flags_as_byte()
{
    return (N << 7) | (V << 6) | (true << 5) | (B << 4) | (D << 3) | (I << 2) | (Z << 1) | (C << 1);
}

std::ostream& operator<<(std::ostream& stream, const CPU& cpu)
{
    stream << "A: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.A;
    stream << "   X: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.X;
    stream << "   Y: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.Y;
    stream << "   IP: 0x" << std::hex << std::setw(4) << std::setfill('0') << (int)cpu.IP;
    stream << "   SP: 0x" << std::hex << std::setw(4) << std::setfill('0') << (int)cpu.SP;
    stream << "\nFlags: ";
    stream << (int)cpu.N;
    stream << (int)cpu.V;
    stream << "-";
    stream << (int)cpu.B;
    stream << (int)cpu.D;
    stream << (int)cpu.I;
    stream << (int)cpu.Z;
    stream << (int)cpu.C;
    return stream;
}
